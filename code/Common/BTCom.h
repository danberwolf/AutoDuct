// BLE protocol handler module
// (C) 2023-09-09 by Daniel Porzig

#ifndef USERCONSOLE_H
#define	USERCONSOLE_H

#include <ctype.h>
#include "HardwareProfile.h"
#include <GenericTypeDefs.h>



typedef union _FLOAT_VAL
{
    float Val;
	WORD w[2];
    BYTE v[4];
} FLOAT_VAL;




// maximum number of commands
#define BTCOM_MAX_NUMBER_COMMANDS		30


typedef void (*VoidFnctCallback)( BYTE*, BYTE*, BYTE *);



typedef struct
{
	VoidFnctCallback callback;
	BYTE CmdID;
}CommandStruct;	


void BTCom_Init();
BYTE BTCom_Task();
void BTCom_addCallback(BYTE CmdID, VoidFnctCallback callback);

WORD BTCom_HandleCommand(BYTE *buf_in, BYTE *buf_out, WORD len);
void BTCom_PutResponse();

BYTE BTCom_injectCommand(char *str);

#endif	/* USERCONSOLE_H */

