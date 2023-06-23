
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
#include<QtWidgets/QApplication>

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
  Csgui::Csgui(int unix_socket_fd) noexcept(false) : _encode_text("encode"), _decode_text("decode"), _current_action(0)
  {
    _mainWidget = new QWidget;
    _mainVBox = new QVBoxLayout;
    _selectionComboBox = new QComboBox;
    _keySpinBox = new QSpinBox;
    _textLineEdit = new QLineEdit;
    _executePushButton = new QPushButton("start");
    _textLabel = new QLabel("nil");

    if (!_mainWidget || !_mainVBox || !_selectionComboBox || !_keySpinBox || !_textLineEdit ||
	!_executePushButton || !_textLabel)
      throw std::string{MEMORY_ERROR};

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

    QObject::connect(_executePushButton, SIGNAL(clicked()), this, SLOT(start()));
    QObject::connect(this, SIGNAL(shouldUpdate()), this, SLOT(update()));
    _unix_socket_fd = unix_socket_fd;

    _mainWidget->resize(1024, 768);
    _mainWidget->setWindowTitle("xwcode_stream");
    _mainWidget->show();
    QApplication::exec();
  }

  /*  ~Csgui - destructor  */
  Csgui::~Csgui()
  {
    delete _mainWidget;
    _mainWidget = nullptr;
    delete _mainVBox;
    _mainVBox = nullptr;
    delete _selectionComboBox;
    _selectionComboBox = nullptr;
    delete _keySpinBox;
    _keySpinBox = nullptr;
    delete _textLineEdit;
    _textLineEdit = nullptr;
    delete _textLabel;
    _textLabel = nullptr;
  }


  /*  start - Qt slot,it is used to start process  */
  void Csgui::start(void)
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
      updateEvent();
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
      updateEvent();
      sleep(1);
      exit(XWCODE_STREAM_MEMORY_ERROR);
    }

    delete[] text_buffer;
  }

  /*  update - Qt slot,it is used to update display  */
  void Csgui::update(void)
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
    _mainWidget->show();
    QApplication::exec();
  }

  void Csgui::updateLabel(const char *msg) noexcept(false)
  {
    if (msg) {
      if (_textLabel)
	delete _textLabel,
	  _textLabel = nullptr;

      _textLabel = new QLabel(msg);
      if (!_textLabel)
	throw std::string{MEMORY_ERROR};
    }
  }

#include<QtWidgets/QApplication>
  /*  startEventLoop - start event loop driver  */
  void Csgui::startEventLoop(void) noexcept(false)
  {
    for ( ; ; ) {
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
	  updateEvent();
	  sleep(1);
	  exit(XWCODE_STREAM_MEMORY_ERROR);
	}
	memset(text_buffer, 0, tbs);  //  to zero

	ssize_t ioReturn = recv(_unix_socket_fd, &gmp, sizeof(struct gmprotocol), 0);

      internal_exit:

	if (ioReturn <= 0) {
	  delete[] text_buffer;
	  updateLabel(IO_ERROR);
	  updateEvent();
	  sleep(1);
	  exit(XWCODE_STREAM_IOERROR);
	}

	//  read next data fragment
	ioReturn = recv(_unix_socket_fd, text_buffer, gmp.text_length, 0);
	if (ioReturn != gmp.text_length)
	  goto internal_exit;
	
	updateLabel(text_buffer);
	updateEvent();

      }
    }
  }

}
