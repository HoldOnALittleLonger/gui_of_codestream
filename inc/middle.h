
/* Name: middle.h
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

#include"xwcode_stream.h"
#include"gmprotocol.h"

#ifndef CRYPTOR_PATH
#define CRYPTOR_PATH "/bin/cryptor"
#endif


/*  middle_procedure - the middle program for communicate with GUI and Cryptor. 
 *  @unix_socket_fd : the unix domain socket for communication with GUI.
 *  return - no return
 */

#ifdef __cplusplus
extern "C"
{
#endif

void middle_procedure(int unix_socket_fd);

#ifdef __cplusplus
}
#endif
