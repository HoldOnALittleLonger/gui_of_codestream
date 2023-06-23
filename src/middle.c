
/* Name: middle.c
 * Type: C program code file
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

#include<sys/types.h>
#include<sys/wait.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include<stdio.h>

#include"middle.h"

/*  middle_procedure - middle procedure between GUI and Cryptor.
 *  @unix_socket_fd : unix socket fd,it have to been connected.
 *  return - no return.
 *  #  middle dont close any file descriptor,because OS will does this for us.
 */
void middle_procedure(int unix_socket_fd)
{
	int fdPipe[2] = {-1, -1};
	if (pipe(fdPipe) < 0)
		_exit(XWCODE_STREAM_OPENPIPE_FAILED);
	else {
		/*  redirection for 0 and 1  */
		/*  child will inherits this  */
		dup2(fdPipe[0], STDIN_FILENO);
		dup2(fdPipe[1], STDOUT_FILENO);
	}

	fcntl(unix_socket_fd, F_SETFD, FD_CLOEXEC);

	while (1) {
		struct gmprotocol gmp;
		memset(&gmp, 0, sizeof(struct gmprotocol));

		ssize_t ioReturn = EOF;
		ioReturn = recv(unix_socket_fd, &gmp, sizeof(struct gmprotocol), 0);    /*  read message from GUI  */
		if (ioReturn == EOF)                              /*  if peer shutdown write,receiver will get EOF  */
			_exit(XWCODE_STREAM_EXIT);
		else if (ioReturn != sizeof(struct gmprotocol))
			_exit(XWCODE_STREAM_IOERROR);

		char cryptor_buffer[XWCODE_STREAM_TEXT_MAX];
		memset(cryptor_buffer, 0, XWCODE_STREAM_TEXT_MAX);

		/*  read data from GUI  */
		ioReturn = recv(unix_socket_fd, cryptor_buffer, gmp.text_length, 0);
		if (ioReturn <= 0)
			_exit(XWCODE_STREAM_IOERROR);

		/*  fork  */
		pid_t execPid = fork();

		if (execPid < 0)
			_exit(XWCODE_STREAM_FORK_FAILED_IN_MIDDLE);
		else if (execPid > 0) {    /*  parent  */
			memset(cryptor_buffer, 0, XWCODE_STREAM_TEXT_MAX);
			ioReturn = read(STDIN_FILENO, cryptor_buffer, XWCODE_STREAM_TEXT_MAX);
			if (ioReturn <= 0)
				_exit(XWCODE_STREAM_IOERROR);

			int statloc = 0;
			(void)wait(&statloc);
			if (WIFEXITED(statloc)) {
				if (WEXITSTATUS(statloc) != 0)
					ioReturn = snprintf(cryptor_buffer, XWCODE_STREAM_TEXT_MAX, "ERROR : cryptor return value is %d",
							    WEXITSTATUS(statloc));
			} else {
				ioReturn = snprintf(cryptor_buffer, XWCODE_STREAM_TEXT_MAX, "ERROR : cryptor exception occurred.");
			}

			gmp.text_length = ioReturn;
			send(unix_socket_fd, &gmp, sizeof(struct gmprotocol), 0);
			if (send(unix_socket_fd, cryptor_buffer, ioReturn, 0) != ioReturn)
				_exit(XWCODE_STREAM_IOERROR);

		} else {    /*  child  */
			char key_string[4];
			memset(key_string, 0, 4);
			snprintf(key_string, 4, "%u", gmp.key);
			cryptor_buffer[ioReturn + 1] = '\0';
			/*  cryptor -k @key_string -e | -d @cryptor_buffer  */
			if (execl(CRYPTOR_PATH, CRYPTOR_PATH, &gmp.key_option, key_string, &gmp.ed_option, cryptor_buffer, NULL) < 0)
				_exit(XWCODE_STREAM_EXECERROR);
		}
	}
}

