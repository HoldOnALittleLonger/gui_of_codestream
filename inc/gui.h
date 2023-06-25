
/* Name: gui.h
 * Type: Header
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

#pragma once

#include<cstddef>
#include<string>
#include<exception>
#include"xwcode_stream.h"

#include<QtWidgets/QMainWindow>

class QWidget;
class QVBoxLayout;
class QComboBox;
class QSpinBox;
class QLineEdit;
class QPushButton;
class QLabel;

namespace csgui {

  class Csgui final : public QMainWindow {
    Q_OBJECT
  public slots:
    void startSlot(void);                //  event loop start
    void inputSubcommittedSlot(void);    //  user input received
    void backgroundWorkSlot(void);       //  bakcground process
    void resultReturnedSlot(void);       //  result deal with
    void guiUpdateSlot(void);            //  redraw GUI
  signals: void inputSubcommitted(void);
  signals: void backgroundWork(void);
  signals: void resultReturned(void);
  signals: void guiUpdate(void);
 
  public:
    explicit Csgui(int unix_socket_fd);
    ~Csgui();
    Csgui(Csgui &) =delete;
    Csgui &operator=(Csgui &) =delete;
    Csgui(Csgui &&) noexcept =delete;
    Csgui &operator=(Csgui &&) noexcept =delete;
    void first_settings(void);

  private:
    QWidget *_mainWidget;
    QVBoxLayout *_mainVBox;
    QComboBox *_selectionComboBox;
    QSpinBox *_keySpinBox;
    QLineEdit *_textLineEdit;
    QPushButton *_executePushButton;
    QLabel *_textLabel;
    
    const char * const _encode_text;
    const char * const _decode_text;

    enum {
      ENCODE_MAX = (XWCODE_STREAM_TEXT_MAX * 3) / 8,
      DECODE_MAX = XWCODE_STREAM_TEXT_MAX
    };

    int _unix_socket_fd;
    unsigned _current_action;

    /*  updateLabel - update QLabel message for new style
     *  @msg : const char pointer which points to the message string.
     *  return - no return.
     */
    void updateLabel(const char *msg);

    //  signal wrappers
    //  this class is not allowed to be inherited by others,
    //  so dont need to place events in protected zone.
    void inputSubcommittedEvent(void)
    {
      emit inputSubcommitted();
    }
    void backgroundWorkEvent(void)
    {
      emit backgroundWork();
    }
    void resultReturnedEvent(void)
    {
      emit resultReturned();
    }
    void guiUpdateEvent(void)
    {
      emit guiUpdate();
    }
  };

}
