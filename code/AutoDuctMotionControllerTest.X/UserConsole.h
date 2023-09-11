// user debug console module
// (C) 2023-09-09 by Daniel Porzig

#ifndef USERCONSOLE_H
#define	USERCONSOLE_H

#include "circbuffer.h"

BYTE UserConsole_Parse();
void UserConsole_Init();
void UserConsole_Task();

#endif	/* USERCONSOLE_H */

