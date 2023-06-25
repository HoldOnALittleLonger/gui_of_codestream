
/* Name: gui.cc
 * Type: C++ program code file
 * Description:
 * Header:
 * Function prototype:
 * Last modified date:
 * Fix:
 */

 /* Feature */
 /* Header */
 /* Macro */
 /* Data */
 /* Function */

#include<QtWidgets/QWidget>
#include<QtWidgets/QVBoxLayout>
#include<QtWidgets/QComboBox>
#include<QtWidgets/QSpinBox>
#include<QtWidgets/QLineEdit>
#include<QtWidgets/QPushButton>
#include<QtWidgets/QLabel>
#include<QtCore/QString>

#include<fcntl.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/select.h>
#include<sys/socket.h>

#include"gmprotocol.h"
#include"gui.h"

namespace csgui {

  static const char * const MEMORY_ERROR("ERROR : Memory allocate failed.");
  static const char * const IO_ERROR("ERROR : IO exception.");

  /*  Csgui - constructor.
   *  @unix_socket_fd : unix socket for communicate with middle.
   *  return - no return.
   */
  Csgui::Csgui(int unix_socket_fd) : _encode_text("encode"), _decode_text("decode"), _current_action(0)
  {
    _mainWidget = new QWidget;
    _mainVBox = new QVBoxLayout;
    _selectionComboBox = new QComboBox;
    _keySpinBox = new QSpinBox;
    _textLineEdit = new QLineEdit;
    _executePushButton = new QPushButton("start");
    _textLabel = new QLabel("nil");
    _unix_socket_fd = unix_socket_fd;
  }

  /*  first_settings - before work start,have to makeup GUI  */
  void Csgui::first_settings(void)
  {
    _selectionComboBox->addItem(QString{_encode_text});
    _selectionComboBox->addItem(QString{_decode_text});
    _keySpinBox->setRange(1, 16);
    _textLineEdit->setAlignment(Qt::AlignLeft);
    _textLineEdit->setPlaceholderText("enter text");
    _mainVBox->setDirection(QVBoxLayout::LeftToRight);
    _mainVBox->addWidget(_selectionComboBox);
    _mainVBox->addWidget(_keySpinBox);
    _mainVBox->addWidget(_textLineEdit);
    _mainVBox->addWidget(_executePushButton);
    _mainVBox->addWidget(_textLabel);
    _mainWidget->setLayout(_mainVBox);

    QObject::connect(_executePushButton, SIGNAL(clicked()), this, SLOT(startSlot()));  //  makeup entrance.
    QObject::connect(this, SIGNAL(inputSubcommitted()), this, SLOT(inputSubcommittedSlot()));
    QObject::connect(this, SIGNAL(backgroundWork()), this, SLOT(backgroundWorkSlot()));
    QObject::connect(this, SIGNAL(resultReturned()), this, SLOT(resultReturnedSlot()));
    QObject::connect(this, SIGNAL(guiUpdate()), this, SLOT(guiUpdateSlot()));

    resize(1024, 768);
    setWindowTitle("xwcode_stream");
    setCentralWidget(_mainWidget);
    
    this->show();
  }


  /*  ~Csgui - destructor  */
  Csgui::~Csgui()
  {
    delete _selectionComboBox;
    _selectionComboBox = nullptr;
    delete _keySpinBox;
    _keySpinBox = nullptr;
    delete _textLineEdit;
    _textLineEdit = nullptr;
    delete _textLabel;
    _textLabel = nullptr;

    //  destructor of QWidget will automatically destroy layout.
    //  use reverse order to prevent SIGSEGV.
    delete _mainVBox;
    _mainVBox = nullptr;
    delete _mainWidget;
    _mainWidget = nullptr;
  }

  /*  startSlot - wrapper for inputSubcommittedEvent.  */
  void Csgui::startSlot(void)
  {
    inputSubcommittedEvent();
  }


  /*  inputSubcommittedSlot - the work after QPushButton was pressed  */
  void Csgui::inputSubcommittedSlot(void)
  {
    if (_selectionComboBox->currentText() == QString{_encode_text})
      _current_action = 1;
    else
      _current_action = 0;

    unsigned keyValue = _keySpinBox->value();

    size_t tbs = (_current_action) ? ENCODE_MAX : DECODE_MAX;
    char *text_buffer(new char[tbs]);
    if (!text_buffer) {
      updateLabel(MEMORY_ERROR);
      guiUpdateEvent();
      sleep(1);
      exit(XWCODE_STREAM_MEMORY_ERROR);
    }

    std::string textString(_textLineEdit->text().toStdString());
    strncpy(text_buffer, textString.c_str(), textString.length());

    struct gmprotocol gmp = {
      .key_option = "-k",
      .ed_option = "-e",
      .key = keyValue,
      .text_length = textString.length()
    };

    if (!_current_action)
      strncpy(gmp.ed_option, "-d", 3);

    ssize_t ioReturn = send(_unix_socket_fd, &gmp, sizeof(struct gmprotocol), 0);
    ioReturn += send(_unix_socket_fd, text_buffer, gmp.text_length, 0);
    if (ioReturn != sizeof(struct gmprotocol) + gmp.text_length) {
      delete[] text_buffer;
      updateLabel(IO_ERROR);
      guiUpdateEvent();
      sleep(1);
      exit(XWCODE_STREAM_MEMORY_ERROR);
    }

    delete[] text_buffer;
    backgroundWorkEvent();  //  produce new event
  }

  /*  resultReturnedSlot - the work after results had been returned  */
  void Csgui::resultReturnedSlot(void)
  {
    guiUpdateEvent();
  }

  /*  guiUpdateSlot - redraw GUI  */
  void Csgui::guiUpdateSlot(void)
  {
    if (_mainVBox)
      delete _mainVBox,
	_mainVBox = nullptr;

    _mainVBox = new QVBoxLayout;
    if (!_mainVBox)
      exit(XWCODE_STREAM_MEMORY_ERROR);

    _mainVBox->setDirection(QVBoxLayout::LeftToRight);
    _mainVBox->addWidget(_selectionComboBox);
    _mainVBox->addWidget(_keySpinBox);
    _mainVBox->addWidget(_textLineEdit);
    _mainVBox->addWidget(_executePushButton);
    _mainVBox->addWidget(_textLabel);

    _mainWidget->setLayout(_mainVBox);
    _mainWidget->update();
    this->update();
  }

  /*  updateLabel - update QLabel widget with new message.  */
  void Csgui::updateLabel(const char *msg)
  {
    if (msg) {
      if (_textLabel)
	delete _textLabel,
	  _textLabel = nullptr;

      _textLabel = new QLabel(msg);
      if (!_textLabel)
	exit(XWCODE_STREAM_MEMORY_ERROR);
    }
  }

  /*  backgroundWorkSlot - the work after request had been sent  */
  void Csgui::backgroundWorkSlot(void)
  {
      fd_set readFdset;
      FD_ZERO(&readFdset);
      FD_SET(_unix_socket_fd, &readFdset);

      if (select(_unix_socket_fd + 1, &readFdset, NULL, NULL, NULL) == 1 &&
	  FD_ISSET(_unix_socket_fd, &readFdset)) {
	struct gmprotocol gmp;
	memset(&gmp, 0, sizeof(struct gmprotocol));
	size_t tbs = (_current_action) ? DECODE_MAX : ENCODE_MAX;
	++tbs;
	char *text_buffer(new char[tbs]);
	if (!text_buffer) {
	  updateLabel(MEMORY_ERROR);
	  guiUpdateEvent();
	  sleep(1);
	  exit(XWCODE_STREAM_MEMORY_ERROR);
	}
	memset(text_buffer, 0, tbs);  //  to zero

	ssize_t ioReturn = recv(_unix_socket_fd, &gmp, sizeof(struct gmprotocol), 0);

      internal_error_exit:

	if (ioReturn <= 0) {
	  delete[] text_buffer;
	  updateLabel(IO_ERROR);
	  guiUpdateEvent();
	  sleep(1);
	  exit(XWCODE_STREAM_IOERROR);
	}
	//  read next data fragment
	ioReturn = recv(_unix_socket_fd, text_buffer, gmp.text_length, 0);
	if (ioReturn != gmp.text_length) {
	  ioReturn = -1;
	  goto internal_error_exit;
	}

	
	updateLabel(text_buffer);
	delete[] text_buffer;
	resultReturnedEvent();
      }
  }

}
