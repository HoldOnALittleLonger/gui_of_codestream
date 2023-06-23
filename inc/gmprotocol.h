
/* Name: gmprotocol.h
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

#include<stddef.h>
#include"xwcode_stream.h"

struct gmprotocol {
	char key_option[3];
	char ed_option[3];
	unsigned short key;
	size_t text_length;
};

