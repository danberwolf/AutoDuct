/*

UART1 Driver Header File for PIC24.

********************************************************************************
 FileName:        uart1.c
 Dependencies:    HardwareProfile.h
 Processor:       PIC24
 Compiler:        MPLAB C30
 Linker:          MPLAB LINK30
 Company:         Microchip Technology Incorporated

Author                Date      Comment
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Anton Alkhimenok   18-Oct-2005
KO                 11-Oct-2006  v1.0
Anton Alkhimenok   17-Feb-2009  Added UART1Char2Hex(), UART1Hex2Char(), 
									  UART1ClearError(), UART1DataReceived()
PAT				   27-Jan-2010  Added UART1GetBaudError() for dynamic checking
								of baud rate percentage error.
********************************************************************************
Software License Agreement

Microchip Technology Inc. ("Microchip") licenses to you the right to use, copy,
modify and distribute the software - including source code - only for use with
Microchip microcontrollers or Microchip digital signal controllers; provided
that no open source or free software is incorporated into the Source Code
without Microchip’s prior written consent in each instance.

The software is owned by Microchip and its licensors, and is protected under
applicable copyright laws.  All rights reserved.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, ANY CLAIMS BY THIRD PARTIES (INCLUDING
BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

********************************************************************************
*/

//******************************************************************************
// Function Prototypes
//******************************************************************************




BYTE UART1_BufIsEmpty();
char UART1_BufRead();
void UART1_BufPutChar( char ch );
void UART1_BufPrintString( char *str );
void UART1_ClearRXBuf();
void UART1_ClearTXBuf();







/*******************************************************************************
Function: UART1GetBaudError()

Precondition:
    None.

Overview:
    This routine checks the UART baud rate error percentage and returns it.

Input: None.

Output: Returns the baud rate error in percent.

*******************************************************************************/
char UART1GetBaudError();

/*********************************************************************
Function: char UART1GetChar()

PreCondition: none

Input: none

Output: last character received

Side Effects: none

Overview: returns last character received

Note: none

********************************************************************/
char UART1GetChar();

/*********************************************************************
Function: void UART1PutChar(char ch)

PreCondition: none

Input: none

Output: none

Side Effects: none

Overview: puts character

Note: none
********************************************************************/
void UART1PutChar( char ch );

/*********************************************************************
Function: void UART1Init(void)

PreCondition: none

Input: none

Output: none

Side Effects: none

Overview: initializes UART 

Note: none
********************************************************************/
void UART1Init();

/*******************************************************************************
Function: UART1IsPressed()

Precondition:
    UART1Init must be called prior to calling this routine.

Overview:
    This routine checks to see if there is a new byte in UART reception buffer.

Input: None.

Output:
    0 : No new data received.
    1 : Data is in the receive buffer

*******************************************************************************/
char UART1IsPressed();

/*******************************************************************************
Function: UART1PrintString( char *str )

Precondition:
    UART1Init must be called prior to calling this routine.

Overview:
    This function prints a string of characters to the UART.

Input: Pointer to a null terminated character string.

Output: None.

*******************************************************************************/
void UART1PrintString( char *str );

/*******************************************************************************
Function: UART1PutDec(unsigned char dec)

Precondition:
    UART1Init must be called prior to calling this routine.

Input: Binary data

Output: none

Side Effects: none

Overview: This function converts decimal data into a string 
          and outputs it to UART.

Note: none
*******************************************************************************/
void UART1PutDec( unsigned char dec );

/*******************************************************************************
Function: UART1PutHex

Precondition:
    UART1Init must be called prior to calling this routine.

Input: Binary data

Output: none

Side Effects: none

Overview: This function converts hex data into a string 
          and outputs it to UART.

Note: none
*******************************************************************************/
void UART1PutHex( int toPrint );

/*******************************************************************************
Function: UART1PutHexWord(unsigned int toPrint)

Precondition:
    UART1Init must be called prior to calling this routine.

Input: Binary data

Output: none

Side Effects: none

Overview: This function converts hex data into a string 
          and outputs it to UART.

Note: none
*******************************************************************************/
#if defined( __C30__ ) || defined( __PIC32MX__ )
void UART1PutHexWord( unsigned int toPrint );
void UART1PutHexDWord( unsigned long int toPrint );
#endif

/*********************************************************************
Function: char UART1Char2Hex(char ch)

PreCondition: none

Input: ASCII to be converted

Output: number

Side Effects: none

Overview: converts ASCII coded digit into number

Note: none

********************************************************************/
char UART1Char2Hex(char ch);

/*********************************************************************
Function: char UART1Hex2Char(char hex)

PreCondition: none

Input: number

Output: ASCII code

Side Effects: none

Overview: converts low nibble into ASCII coded digit

Note: none

********************************************************************/
char UART1Hex2Char(char hex);

/*********************************************************************
Function: void UART1ClrError(void)

PreCondition: none

Input: none

Output: character received

Side Effects: none

Overview: wait for character

Note: none

********************************************************************/
void UART1ClrError(void);

/*********************************************************************
Macros: UART1DataReceived()

PreCondition: none

Input: none

Output: zero if character is not received

Side Effects: none

Overview: checks if data is available

Note: none

********************************************************************/
#define UART1DataReceived() (U3STAbits.URXDA)

