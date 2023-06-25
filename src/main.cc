
/* Name: main.cc
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

#include<sys/socket.h>
#include<sys/types.h>
#include<sys/un.h>
#include<sys/wait.h>
#include<unistd.h>
#include<signal.h>

#include<iostream>
#include<cstring>

#include<QtWidgets/QApplication>

#include"gui.h"
#include"xwcode_stream.h"
#include"middle.h"

static const char * const UNIX_SOCK_PATH("/tmp/xwcode_stream_unix.sock");

static const char * const MAIN_CONNECTION_ERROR("ERROR : connection error.");
static const char * const MAIN_FORK_ERROR("ERROR : fork process error.");
static const char * const MAIN_SIGACTION_ERROR("ERROR : install sigaction failed.");

/*  signalaction_SIGCHLD - sigaction for SIGCHLD
 *  #  parent will stop if signal SIGCHLD had been received.
 */
//  in correct situation,must parent stop first than child.
void signalaction_SIGCHLD([[maybe_unused]]int arg)
{
  std::cerr<<"Child Process stopped"<<std::endl;
  _exit(XWCODE_STREAM_CHILD_EXCEPTION);
}

int main(int argc, char *argv[])
{
  pid_t forkPid = fork();

  if (forkPid > 0) {  //  parent

    struct sigaction sigchld;
    sigchld.sa_handler = signalaction_SIGCHLD;
    sigchld.sa_flags = 0;
    sigemptyset(&sigchld.sa_mask);
    if (sigaction(SIGCHLD, &sigchld, NULL) < 0) {
      std::cerr<<MAIN_SIGACTION_ERROR<<std::endl;
      exit(XWCODE_STREAM_SIGACTION_ERROR);
    }

    //  bind() will automatically create socket file for UNIX socket,
    //  delete it as first.
    if (!access(UNIX_SOCK_PATH, F_OK))
      unlink(UNIX_SOCK_PATH);

    int unix_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un unix_sockaddr;
    unix_sockaddr.sun_family = AF_UNIX;
    strncpy(unix_sockaddr.sun_path, UNIX_SOCK_PATH, strlen(UNIX_SOCK_PATH));
    unix_sockaddr.sun_path[strlen(UNIX_SOCK_PATH) + 1] = '\0';

    bind(unix_socket_fd, (struct sockaddr *)&unix_sockaddr, (socklen_t)sizeof(unix_sockaddr));
    listen(unix_socket_fd, 1);

    //  wait client.
    int communicateSocket = accept(unix_socket_fd, NULL, NULL);

    QApplication app(argc, argv);
    csgui::Csgui csguiObj(communicateSocket);
    csguiObj.first_settings();
    //  dont care about what child has returned,OS will recyle status
    return QApplication::exec();
  } else if (forkPid == 0) {  //  child
    sleep(1);  //  let parent executes first.
    errno = 0;

    int client_unix_sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un server_addr = {
      .sun_family = AF_UNIX
    };
    strncpy(server_addr.sun_path, UNIX_SOCK_PATH, strlen(UNIX_SOCK_PATH));
    server_addr.sun_path[strlen(UNIX_SOCK_PATH) + 1] = '\0';

    if (connect(client_unix_sock_fd, (struct sockaddr *)&server_addr, (socklen_t)sizeof(server_addr)) < 0) {
      std::cerr<<MAIN_CONNECTION_ERROR<<std::endl;
      std::cerr<<strerror(errno)<<std::endl;
      exit(XWCODE_STREAM_SOCK_ERROR);
    }

    middle_procedure(client_unix_sock_fd);
  } else {
    std::cerr<<MAIN_FORK_ERROR<<std::endl;
  }
  
  return 0;
}

