/*

UART2 Driver File for PIC24.

********************************************************************************
 FileName:        uart2.c
 Dependencies:    HardwareProfile.h
 Processor:       PIC24
 Compiler:        MPLAB C30
 Linker:          MPLAB LINK30
 Company:         Microchip Technology Incorporated

Author                Date      Comment
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
KO                 12-Feb-2008  Modified to use HardwareProfile.h
KO                 11-Oct-2006  v1.0
Anton Alkhimenok   18-Oct-2005
Anton Alkhimenok   17-Feb-2009  Added UART2Char2Hex(), UART2Hex2Char(), 
									  UART2ClearError(), UART2DataReceived()
PAT				   27-Jan-2010  Added UART2GetBaudError() for dynamic checking
								of baud rate percentage error.

********************************************************************************
Software License Agreement

Microchip Technology Inc. ("Microchip") licenses to you the right to use, copy,
modify and distribute the software - including source code - only for use with
Microchip microcontrollers or Microchip digital signal controllers; provided
that no open source or free software is incorporated into the Source Code
without Microchip�s prior written consent in each instance.

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

#include "Compiler.h"
#include "HardwareProfile.h"
#include "UART2.h"
#include "circbuffer.h"

//******************************************************************************
// Constants
//******************************************************************************

//U2BRG register value and baudrate mistake calculation

#if defined (__C30__)
    #define BAUDRATEREG2        (((GetSystemClock()/2)+(BRG_DIV2/2*BAUDRATE2))/BRG_DIV2/BAUDRATE2-1)
#elif defined (__PIC32MX__)
    #define BAUDRATEREG2        ((GetPeripheralClock()+(BRG_DIV_UART2/2*BAUDRATE_UART2))/BRG_DIV_UART2/BAUDRATE_UART2-1)
#else
    #error Cannot calculate BRG value
#endif    

#if defined (__C30__)
    #define BAUD_ACTUAL2         ((GetSystemClock()/2)/BRG_DIV2/(BAUDRATEREG2+1))
#elif defined (__PIC32MX__)
    #define BAUD_ACTUAL2         (GetPeripheralClock()/BRG_DIV_UART2/(BAUDRATEREG2+1))
#else
    #error Cannot calculate actual baud rate
#endif    


	#define BAUD_ERROR2              ((BAUD_ACTUAL2 > BAUDRATE_UART2) ? BAUD_ACTUAL2-BAUDRATE_UART2 : BAUDRATE_UART2-BAUD_ACTUAL2)
	#define BAUD_ERROR_PERCENT2      ((BAUD_ERROR2*100+BAUDRATE_UART2/2)/BAUDRATE_UART2)
	
#if defined (__C30__)

	#if (BAUD_ERROR_PERCENT2 > 3)
	    #error UART1 frequency error is worse than 3%
	#elif (BAUD_ERROR_PERCENT2 > 2)
	    #warning UART1 frequency error is worse than 2%
	#endif

#endif // #if defined (__C30__)




static volatile CircBuffer CB_UART2RX;
static volatile CircBuffer CB_UART2TX;



void __ISR(_UART_2_VECTOR, ipl5) UART2InterruptServiceRoutine(void)
{
    char Temp;
	
    // Receive interrupt handling
	if(INTGetFlag(INT_U2RX))
	{
	
		// get received char
	    Temp = U2RXREG;

	
		// put char into receive buffer
		CircBufferWrite(&CB_UART2RX,Temp);
				
		
		// Clear the interrupt source flag
		INTClearFlag(INT_U2RX);		
	}
		
    // Transmit interrupt handling
	if(INTGetFlag(INT_U2TX))
	{
		// disable TX-Interrupt
   		INTEnable(INT_U2TX, INT_DISABLED);
   		
		// Clear the interrupt source flag		
		INTClearFlag(INT_U2TX);		

        if(CircBufferIsEmpty(&CB_UART2TX))
        {
	        // just do nothing
	        
	    } 
	    else
	    {
		    // send byte
	        Temp = CircBufferRead(&CB_UART2TX);
    	    U2TXREG = Temp;
		    
		    // switch on interrupt again
   			INTEnable(INT_U2TX, INT_ENABLED);		    
		} 

		   		
   		/*
        // get 
        Temp = CircBufferRead(&CB_UART3ATX);
        U1ATXREG = Temp;
        
        // check if Transmit buffer is empty
        if(CircBufferIsEmpty(&CB_UART3ATX))
        {
            // set TX restart flag
            UART3A_txrestart = 1;
        }
        else
        {
			// reenable TX-Interrupt
   			INTEnable(INT_U1ATX, INT_ENABLED);
   		}
   		*/

	}	

}	



BYTE UART2_BufIsEmpty()
{
	return (CircBufferIsEmpty(&CB_UART2RX));
}	


void UART2_ClearRXBuf()
{
    CircBufferInit(&CB_UART2RX);
}

void UART2_ClearTXBuf()
{
    CircBufferInit(&CB_UART2TX);    
}    



char UART2_BufRead()
{
    char temp;
    
    // disable TX-Interrupt
   	INTEnable(INT_U2RX, INT_DISABLED);
    
    temp = (CircBufferRead(&CB_UART2RX));
    
    // re-enable TX-Interrupt
   	INTEnable(INT_U2RX, INT_ENABLED);    
        
	return temp;
}	

void UART2_BufPutChar( char ch )
{
    // disable TX-Interrupt
   	INTEnable(INT_U2TX, INT_DISABLED);

    // put the char into the transmit buffer
    CircBufferWrite(&CB_UART2TX,ch);

/*
    if(UART3A_txrestart)
    {
        // if TX-Interrupt is not active activate TX-Interrupt and set TX-Interrupt flag
      	//	INTEnable(INT_U1ATX, INT_ENABLED);
      	//  UART3A_txrestart = 0;
      	
     	UART3A_txrestart = 0;     
     	
      	// manually trigger interrupt to send first byte
      	INTSetFlag(INT_U1ATX);
    }
*/
    
    // reenable TX-Interrupt
   	INTEnable(INT_U2TX, INT_ENABLED);
    
}


void UART2_BufPrintString( char *str )
{
    unsigned char c;

    while( (c = *str++) )
    {
  		// send char
		UART2_BufPutChar(c);
    }
    
    // DEBUG
    /*
    while( !CircBufferIsEmpty(&CB_UART3ATX))
    {
    	UART3APutChar( CircBufferRead(&CB_UART3ATX)); 
    }
    */	
}













/*******************************************************************************
Function: UART2GetBaudError()

Precondition:
    None.

Overview:
    This routine checks the UART baud rate error percentage and returns it.

Input: None.

Output: Returns the baud rate error in percent.

*******************************************************************************/
char UART2GetBaudError()
{
    unsigned int errorPercent = 0;

	errorPercent = ((BAUD_ERROR2*100+BAUDRATE_UART2/2)/BAUDRATE_UART2);
    return (char)errorPercent;
}


/*******************************************************************************
Function: UART2GetChar()

Precondition:
    UART2Init must be called prior to calling this routine.

Overview:
    This routine waits for a byte to be received.  It then returns that byte.

Input: None.

Output: Byte received.

*******************************************************************************/
char UART2GetChar()
{
	
    char Temp;

    while(IFS1bits.U2RXIF == 0);

    Temp = U2RXREG;
    IFS1bits.U2RXIF = 0;
    return Temp;
    
    
}

/*******************************************************************************
Function: UART2Init()

Precondition: None.

Overview:
    This routine sets up the UART2 module.

Input: None.

Output: None.

Notes:
    Allow the peripheral to set the I/O pin directions.  If we set the TRIS
    bits manually, then when we disable the UART, the shape of the stop bit
    changes, and some terminal programs have problems.
*******************************************************************************/
void UART2Init()
{
	/*
	#define UMODE					U3AMODE
	#define UMODEbits				U3AMODEbits
	#define USTA					U3ASTA
	#define USTAbits				U3ASTAbits
	#define UTXREG					U3ATXREG
	*/
	
    U2BRG = BAUDRATEREG2;
    U2MODE = 0;
    U2MODEbits.BRGH = BRGH_UART2;
    U2STA = 0;
    U2MODEbits.UARTEN = 1;
    U2STAbits.UTXEN = 1;
 
    IFS1bits.U2RXIF = 0;	// receive interrupt USART 3

    // Configure the interrupt priority, level 5
	INTSetVectorPriority( INT_UART_2_VECTOR,
	                      INT_PRIORITY_LEVEL_5);
	
	// Configure the interrupt sub-priority, level 2
	INTSetVectorSubPriority( INT_UART_2_VECTOR,
	                         INT_SUB_PRIORITY_LEVEL_2);
	
	// Clear the interrupt source flag
	INTClearFlag(INT_U2RX);
	INTClearFlag(INT_U2TX);
	
	// Enable the interrupt source
	INTEnable(INT_U2RX, INT_ENABLED);
	INTEnable(INT_U2TX, INT_DISABLED);


    CircBufferInit(&CB_UART2RX);
    CircBufferInit(&CB_UART2TX);    

    U2STAbits.URXEN = 1;
}

/*******************************************************************************
Function: UART2IsPressed()

Precondition:
    UART2Init must be called prior to calling this routine.

Overview:
    This routine checks to see if there is a new byte in UART reception buffer.

Input: None.

Output:
    0 : No new data received.
    1 : Data is in the receive buffer

*******************************************************************************/
char UART2IsPressed()
{
	
//	    IFS0bits.U1RXIF = 0;	// receive interrupt USART 1A

	
    if(IFS1bits.U2RXIF == 1)
        return 1;

    return 0;
}

/*******************************************************************************
Function: UART2PrintString( char *str )

Precondition:
    UART2Init must be called prior to calling this routine.

Overview:
    This function prints a string of characters to the UART.

Input: Pointer to a null terminated character string.

Output: None.

*******************************************************************************/
void UART2PrintString( char *str )
{
    unsigned char c;

    while( (c = *str++) )
        UART2PutChar(c);
}

/*******************************************************************************
Function: UART2PutChar( char ch )

Precondition:
    UART2Init must be called prior to calling this routine.

Overview:
    This routine writes a character to the transmit FIFO, and then waits for the
    transmit FIFO to be empty.

Input: Byte to be sent.

Output: None.

*******************************************************************************/
void UART2PutChar( char ch )
{
    U2TXREG = ch;
    #if !defined(__PIC32MX__)
        Nop();
    #endif
    while(U2STAbits.TRMT == 0);
}

/*******************************************************************************
Function: UART2PutDec(unsigned char dec)

Precondition:
    UART2Init must be called prior to calling this routine.

Overview:
    This function converts decimal data into a string and outputs it to UART.

Input: Binary data.

Output: None.

*******************************************************************************/
void  UART2PutDec(unsigned char dec)
{
    unsigned char res;
    unsigned char printed_already = 0;

    res = dec;

    if (res/100)
    {
        UART2PutChar( res/100 + '0' );
        printed_already = 1;
    }
    res = res - (res/100)*100;

    if ((res/10) || (printed_already == 1))
    {
        UART2PutChar( res/10 + '0' );
    }
    res = res - (res/10)*10;

    UART2PutChar( res + '0' );
}

/*******************************************************************************
Function: UART2PutHex

Precondition:
    UART2Init must be called prior to calling this routine.

Overview:
    This function converts hex data into a string and outputs it to UART.

Input: Binary data.

Output: None.

*******************************************************************************/


void UART2PutHex( int toPrint )
{
	const unsigned char CharacterArray[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	
    int printVar;

    printVar = toPrint;
    toPrint = (toPrint>>4) & 0x0F;
    UART2PutChar( CharacterArray[toPrint] );

    toPrint = printVar & 0x0F;
    UART2PutChar( CharacterArray[toPrint] );

    return;
}

/*******************************************************************************
Function: UART2PutHexWord(unsigned int toPrint)

Precondition:
    UART2Init must be called prior to calling this routine.

Overview:
    This function converts hex data into a string and outputs it to UART.

Input: Binary data.

Output: None.

*******************************************************************************/
#if defined( __C30__ ) || defined( __PIC32MX__ )
void UART2PutHexWord( unsigned int toPrint )
{
	const unsigned char CharacterArray[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	
    unsigned int printVar;

    printVar = (toPrint>>12) & 0x0F;
    UART2PutChar( CharacterArray[printVar] );

    printVar = (toPrint>>8) & 0x0F;
    UART2PutChar( CharacterArray[printVar] );

    printVar = (toPrint>>4) & 0x0F;
    UART2PutChar( CharacterArray[printVar] );

    printVar = toPrint & 0x0F;
    UART2PutChar( CharacterArray[printVar] );

    return;
}

void UART2PutHexDWord( unsigned long toPrint )
{
	const unsigned char CharacterArray[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	
    unsigned long printVar;

    printVar = (toPrint>>28) & 0x0F;
    UART2PutChar( CharacterArray[printVar] );

    printVar = (toPrint>>24) & 0x0F;
    UART2PutChar( CharacterArray[printVar] );

    printVar = (toPrint>>20) & 0x0F;
    UART2PutChar( CharacterArray[printVar] );

    printVar = (toPrint>>16) & 0x0F;
    UART2PutChar( CharacterArray[printVar] );

    printVar = (toPrint>>12) & 0x0F;
    UART2PutChar( CharacterArray[printVar] );

    printVar = (toPrint>>8) & 0x0F;
    UART2PutChar( CharacterArray[printVar] );

    printVar = (toPrint>>4) & 0x0F;
    UART2PutChar( CharacterArray[printVar] );

    printVar = toPrint & 0x0F;
    UART2PutChar( CharacterArray[printVar] );

    return;
}

#endif

/*********************************************************************
Function: char UART2Char2Hex(char ch)

PreCondition: none

Input: ASCII to be converted

Output: number

Side Effects: none

Overview: converts ASCII coded digit into number

Note: none

********************************************************************/
char UART2Char2Hex(char ch){
  // Wrong char
  if(ch > 102)
    return 0;  
  
  // From a to f     
  if(ch > 96)
    return (ch-87);
    
  // Wrong char    
  if(ch > 70)
    return 0;
  
  // From A to F    
  if(ch > 64)
    return (ch-55);

  // Wrong char  
  if(ch > 57)
    return 0;
  
  // From 0 - 9
  if(ch > 47)
    return(ch-48);
  else
  // Wrong char
    return 0;
}

/*********************************************************************
Function: char UART2Hex2Char(char hex)

PreCondition: none

Input: number

Output: ASCII code

Side Effects: none

Overview: converts low nibble into ASCII coded digit

Note: none

********************************************************************/
char UART2Hex2Char(char hex){
char h;
  h = hex&0x0f;
  // From 0xa to 0xf
  if(h>9)
    return (h+55);
  else
    return (h+48);
}

/*********************************************************************
Function: void UART2ClrError(void)

PreCondition: none

Input: none

Output: character received

Side Effects: none

Overview: wait for character

Note: none

********************************************************************/
void UART2ClrError(void){
    // Clear error flag
    if(U2STAbits.OERR)
		U2STAbits.OERR = 0;
}

