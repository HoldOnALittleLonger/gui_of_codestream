
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

#include<QtCore/QObject>

class QWidget;
class QVBoxLayout;
class QComboBox;
class QSpinBox;
class QLineEdit;
class QPushButton;
class QLabel;

namespace csgui {

  static const char *MEMORY_ERROR("ERROR : Memory allocate failed.");
  static const char *IO_ERROR("ERROR : IO exception.");

  class Csgui final : public QObject {
    Q_OBJECT
  public slots:
    void start(void);
    void update(void);

  signals: void shouldUpdate(void);
 
  public:
    explicit Csgui(int unix_socket_fd) noexcept(false);
    ~Csgui();
    Csgui(Csgui &) =delete;
    Csgui &operator=(Csgui &) =delete;
    Csgui(Csgui &&) noexcept =delete;
    Csgui &operator=(Csgui &&) noexcept =delete;

    void startEventLoop(void) noexcept(false);

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

    /*  shouldUpdate - event for info object should update display  */
    void updateEvent(void)
    {
      emit shouldUpdate();
    }

    /*  updateLabel - update QLabel message for new style
     *  @msg : const char pointer which points to the message string.
     *  return - no return.
     */
    void updateLabel(const char *msg) noexcept(false);

  };

}
