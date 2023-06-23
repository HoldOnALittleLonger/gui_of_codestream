
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
  int unix_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  struct sockaddr_un unix_sockaddr = {
    .sun_family = AF_UNIX,
  };

  strncpy(unix_sockaddr.sun_path, UNIX_SOCK_PATH, strlen(UNIX_SOCK_PATH));
  unix_sockaddr.sun_path[strlen(UNIX_SOCK_PATH) + 1] = '\0';

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

    bind(unix_socket_fd, (struct sockaddr *)&unix_sockaddr, sizeof(struct sockaddr_un));
    listen(unix_socket_fd, 1);

    //  wait client.
    socklen_t addr_len(sizeof(struct sockaddr_un));
    int communicateSocket = accept(unix_socket_fd, (struct sockaddr *)&unix_sockaddr, &addr_len);
    csgui::Csgui *pObjCsgui(nullptr);

    try {
      pObjCsgui = new csgui::Csgui(communicateSocket);
    } catch (std::string s) {
      std::cerr<<s<<std::endl;
      delete pObjCsgui;
      exit(XWCODE_STREAM_PROGRAM_ERROR);
    }

    QApplication app(argc, argv);
    QApplication::exec();

    try {
      pObjCsgui->startEventLoop();
    } catch (std::string s) {
      std::cerr<<s<<std::endl;
      exit(XWCODE_STREAM_PROGRAM_ERROR);
    }

    //  dont care about what child has returned
    (void)wait(NULL);

  } else if (forkPid == 0) {  //  child
    sleep(1);  //  let parent executes first.
    int communicateSocket = -1;
    if (connect(communicateSocket, (struct sockaddr *)&unix_sockaddr, sizeof(struct sockaddr_un)) < 0) {
      std::cerr<<MAIN_CONNECTION_ERROR<<std::endl;
      exit(XWCODE_STREAM_SOCK_ERROR);
    }

    middle_procedure(communicateSocket);
  } else {
    std::cerr<<MAIN_FORK_ERROR<<std::endl;
  }
  
  return 0;
}

