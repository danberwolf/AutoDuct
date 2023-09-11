// BLE protocol handling module
// (C) 2023-09-09 by Daniel Porzig

#include "BTCom.h"
#include "UART1.h"


#define MAX_PACKET_SIZE         256

#define COMREC_STATE_IDLE       0
#define COMREC_STATE_STARTSEQ   1
#define COMREC_STATE_DATADEC    2
#define COMREC_STATE_NEXTISDATA 3

#define COMREC_DECODE_DONE              1       // decoding done
#define COMREC_ERROR_CHECKSUM           2       // checksum error
#define COMREC_ERROR_MAX_PACKETSIZE     3       // max. packet size exceeded
#define COMREC_ERROR_TIMEOUT            4       // timeout occured before complete package was received
#define COMREC_ERROR_SYNTAX             5       // syntax error, unexpected data received

//Communications Control bytes
#define STX             0x55
#define ETX             0x04
#define DLE             0x05


#define TIMEOUT_CNT     500     // approx 2 second counter    


#define _VERBOSE_



WORD responseBytes;                                                                 //Number of bytes in command response
BYTE buffer[MAX_PACKET_SIZE+1];                                                     //Transmit/Recieve Buffer
BYTE checksum;
WORD dataCount;

BYTE comRecState = 0;       // status of command receiver
WORD comRecTimeout = TIMEOUT_CNT;   

CommandStruct BTcoms[BTCOM_MAX_NUMBER_COMMANDS];
BYTE numComsUsed;


static char txt[60];


static void SendFWStringBT(BYTE *outbuf, WORD *len);
void BTCom_defaultCallback(BYTE *buf_in, BYTE *buf_out, BYTE* len);

// initialize protocol handler
void BTCom_Init()
{
    BYTE i;
    checksum = 0;
    dataCount = 0;
    comRecState = 0;
    
    for(i=0; i<BTCOM_MAX_NUMBER_COMMANDS; i++)
    {
        BTcoms[i].CmdID = 0xFF;
        BTcoms[i].callback = NULL;
    }
    numComsUsed = 0;
}

// add a command including callback to protocol handler
void BTCom_addCallback(BYTE CmdID, VoidFnctCallback callback)
{
    if(numComsUsed == BTCOM_MAX_NUMBER_COMMANDS)
        return;
    
    BTcoms[numComsUsed].CmdID = CmdID;
    BTcoms[numComsUsed].callback = callback;
    numComsUsed++;
}

// protocol handler task
BYTE BTCom_Task()
{
    BYTE RXByte;
    BYTE status = 0;
    

    // check user inputs
    while(!UART1_BufIsEmpty())
    {
        RXByte=UART1_BufRead();
        comRecTimeout = TIMEOUT_CNT; // reset timeout counter
  
        
//        sprintf(txt,"%02X,",RXByte);
//        UART3PrintString(txt);        
        
        switch(comRecState)
        {
            case COMREC_STATE_IDLE:     // Idle
                
                if(RXByte == STX)
                {
                    // first STX received, wait for second STX
                    comRecState = COMREC_STATE_STARTSEQ;

                }
                else
                {
                    //status = COMREC_ERROR_SYNTAX;   // error, unexpected data received
                    ;       // unexpected data received, ignore.
                }

            break;
            case COMREC_STATE_STARTSEQ:     // active sequence, first STX received, wait for second STX

                if(RXByte == STX)
                {
                    // second STX received, following bytes are data
                    comRecState = COMREC_STATE_DATADEC;

                    dataCount = 0;
                    checksum = 0;
                    
                    // DEBUG: start sequence
#ifdef _VERBOSE_
                    DEBUG_puts("\r\n<[");
#endif
                }
                else
                {
                    status = COMREC_ERROR_SYNTAX;   // error, unexpected data received
                }
                
            break;
            case COMREC_STATE_DATADEC:     // active sequence, store incoming data into buffer
                
				switch(RXByte)
                {
					case STX: //Start over if STX
						checksum = 0;
						dataCount = 0;
                        
                        // DEBUG: start sequence
#ifdef _VERBOSE_                        
                        DEBUG_puts("\r\n<[");                        
#endif
                    break;

					case ETX: //End of packet if ETX
                        
                        //Test checksum
						checksum = ~checksum + 1;                                   
						                        
						if(checksum == 0) 
                        {
                            // checksum okay, continue with Command Decoding
                            status = COMREC_DECODE_DONE;
#ifdef _VERBOSE_                            
                            DEBUG_puts("] OK\n\r");
#endif                            
                        }
                        else
                        {
                            // checksum error.
                            status = COMREC_ERROR_CHECKSUM;
                            dataCount = 0xFFFF;                                         //Otherwise restart                            
                            
                            
                        }
					break;

					case DLE: //If DLE, treat next as data
                        
                        // signal receiver state machine that next byte is
                        // to be treated as data
                        comRecState = COMREC_STATE_NEXTISDATA;
                        
                    break;
					default: // regular data byte
						checksum += RXByte;
						buffer[dataCount++] = RXByte;

#ifdef _VERBOSE_                        
                        sprintf(txt,"%02X",RXByte);
                        DEBUG_puts(txt);
#endif
                        
                        if(dataCount <= MAX_PACKET_SIZE+1)
                        {
                            ; // everything ok
                        }
                        else
                        {
                            // max. packet size exceeded
                            status = COMREC_ERROR_MAX_PACKETSIZE;
                        }
					break;        
                }
                
            break;
            case COMREC_STATE_NEXTISDATA:     // active sequence, treat current byte as data without inspection
                
                // treat as regular data byte
                checksum += RXByte;
                buffer[dataCount++] = RXByte;

#ifdef _VERBOSE_                
                sprintf(txt,"%02X",RXByte);
                DEBUG_puts(txt);
#endif
                
                
                // set state machine back to default decoding mode
                comRecState = COMREC_STATE_DATADEC;                
                
                if(dataCount <= MAX_PACKET_SIZE+1)
                {
                    ; // everything ok
                }
                else
                {
                    // max. packet size exceeded
                    status = COMREC_ERROR_MAX_PACKETSIZE;
                }
                
            break;            
        }
    
        
        if(status > 0)
        {
            // leave receiving loop upon error or completed packet
            break;
        }
    }

    

    if(status == 0)
    {
        // check if operation timed out before a complete packet was received
        if(comRecState > 0)
        {
            if(--comRecTimeout == 0)
            {
                status = COMREC_ERROR_TIMEOUT;   // set timeout error
            }
        }
        
    }
   
    // handle status code
    switch(status)
    {
        case COMREC_DECODE_DONE:
            // reset receiver state machine
            comRecState = COMREC_STATE_IDLE;

            // decode received package
            BTCom_HandleCommand(buffer,buffer,dataCount);
            
            // send response
            BTCom_PutResponse();            

#ifdef _VERBOSE_            
            // send response to debug console
            BTCom_PutResponseDebug();
#endif
        break;
        
        case COMREC_ERROR_MAX_PACKETSIZE:
             DEBUG_puts("COMREC_ERROR_MAX_PACKETSIZE\n\r");
            
            // do not send a response so host will resend package
            comRecState = COMREC_STATE_IDLE;           
            
        break;
        
        case COMREC_ERROR_CHECKSUM:
            DEBUG_puts("COMREC_ERROR_CHECKSUM\n\r");
            comRecState = COMREC_STATE_IDLE;
            // do not send a response so host will resend package
            
        break;
        
        case COMREC_ERROR_TIMEOUT:
            DEBUG_puts("COMREC_ERROR_TIMEOUT\n\r");
            comRecState = COMREC_STATE_IDLE;
            // do not send a response so host will resend package
            
        break;
        
        default:
            // nothing to do
        break;
    }
    

    return status;
}


// interprete command and execute callback
WORD BTCom_HandleCommand(BYTE *buf_in, BYTE *buf_out, WORD len)
{
	BYTE Command, j, match;
    // len = packet length (including checksum byte!!!)

	Command = buf_in[0];  //Get command from buffer
//	length = buf_in[1];   //Get data length from buffer

//    responseBytes = 1;    // set response length to 1 byte default
    responseBytes = len;

    match = 0;
    for(j=0; j<numComsUsed; j++)
    {
        if(BTcoms[j].CmdID == Command)
        {
            // execute callback
            BTcoms[j].callback(buf_in, buf_out, &responseBytes);
            match = 1;
        }
    }
    
    // execute default callback, if command not recognized
    if(match == 0)
    {
        BTCom_defaultCallback(buf_in, buf_out, &responseBytes);
    }
    
    return responseBytes;
}


// transmit response
void BTCom_PutResponse()
{
	WORD i;
	BYTE data;
	BYTE checksum;

	UART1PutChar(STX);                                                                   //Put 2 STX characters
	UART1PutChar(STX);
    

	checksum = 0;
	for(i = 0; i < responseBytes; i++)
    {
		data = buffer[i];                                                           //Get data from response buffer
		checksum += data;                                                           //Accumulate checksum
		if(data == STX || data == ETX || data == DLE)
        {                         		//If control character, stuff DLE
            UART1PutChar(DLE);
		}
        UART1PutChar(data);                                                              //Send data
	}

	checksum = ~checksum + 1;                                                       //Keep track of checksum
	if(checksum == STX || checksum == ETX || checksum == DLE)
    {                      //If control character, stuff DLE
        UART1PutChar(DLE);
	}

	UART1PutChar(checksum);                                                              //Put checksum
	UART1PutChar(ETX);                                                                   //Put End of text

}



void BTCom_PutResponseDebug()
{
	WORD i,numbytes;
	BYTE data;
	BYTE checksum;
//    char txt[10];

    
    DEBUG_puts("\n\r>[");
//    sprintf(txt,"%02X",STX);
//    UART3PrintString(txt);
//    sprintf(txt,"%02X",STX);
//    UART3PrintString(txt);
    
	checksum = 0;
    numbytes = 0;
	for(i = 0; i < responseBytes; i++)
    {
		// asm("clrwdt");                                                              //Looping code so clear WDT
		data = buffer[i];                                                           //Get data from response buffer
		checksum += data;                                                           //Accumulate checksum
		if(data == STX || data == ETX || data == DLE)
        {                         		//If control character, stuff DLE
//			UART1_putc(DLE);
 //           sprintf(txt,"%02X",DLE);
 //           UART3PrintString(txt);

		}
//		UART1_putc(data);                                                              //Send data
        sprintf(txt,"%02X",data);
        DEBUG_puts(txt);
        
        numbytes++;
	}

	checksum = ~checksum + 1;                                                       //Keep track of checksum
	if(checksum == STX || checksum == ETX || checksum == DLE)
    {                      //If control character, stuff DLE
//			UART1_putc(DLE);
//            sprintf(txt,"%02X",DLE);
//            UART3PrintString(txt);
	}

//	UART1_putc(checksum);                                                              //Put checksum
//	UART1_putc(ETX);                                                                   //Put End of text
    
        sprintf(txt,"%02X] ",checksum);
        DEBUG_puts(txt);

//        sprintf(txt,"%02X] ",ETX);
//        UART3PrintString(txt);
        
        sprintf(txt,"%u Bytes \n\r",responseBytes);
        DEBUG_puts(txt);
        
    
//	UART1_TXflush();                                                         //Wait for transmit to finish
}


BYTE hexStrToByte(char* string)
{
    BYTE value,v, index = 0;
    char c;
    
    v=0;
    
    for(index=0;index<2;index++)
    {
        c = string[index];

        value = 0;
        if(c >= '0' && c <= '9')
        {
            value = (c - '0');
        }
        else if (c >= 'A' && c <= 'F')
        {
            value = (10 + (c - 'A'));
        }
        else if (c >= 'a' && c <= 'f')
        {
             value = (10 + (c - 'a'));
        }
        else
        {
            return 0;
        }

        v += value << (((index + 1) % 2) * 4);
    }

    return v;
}










// a string with a hex encoded command sequence is passed to the command parser
// the response is sent to the debug interface
// NOTES: 
//    + pending data in the input buffer is discarded
//    + input string does not contain <DLE> encoding, but <STX>,<ETX> and checksum
BYTE BTCom_injectCommand(char *str)
{
    BYTE len,i,RXByte,status = 0;
    WORD respbytes;
    char *pstr;
//    char txt[20];
    
    pstr=str;
    
    // reset command decoding state machine
    comRecState = COMREC_STATE_IDLE;

    len = strlen(str);

    i = 0;
    
    DEBUG_puts(str);
    
    // check user inputs
    while(i<len)
    {
        
        // convert input hex string into raw data byte
        RXByte = hexStrToByte(str + i);
        i+=2;
                
        //UART2_putc(RXByte);
        
//        sprintf(txt," %02X,",RXByte);
//        UART2_puts(txt);        
    
        switch(comRecState)
        {
            case COMREC_STATE_IDLE:     // Idle
                
                if(RXByte == STX)
                {
                    // first STX received, wait for second STX
                    comRecState = COMREC_STATE_STARTSEQ;
                }
                else
                {
                    ;   // error, unexpected data received. ignore.
                }

            break;
            case COMREC_STATE_STARTSEQ:     // active sequence, first STX received, wait for second STX

                if(RXByte == STX)
                {
                    // second STX received, following bytes are data
                    comRecState = COMREC_STATE_DATADEC;
                    dataCount = 0;
                    checksum = 0;
                    
                    // DEBUG: start sequence
                    DEBUG_puts("[");
                }
                else
                {
                    ;   // error, unexpected data received
                }
                
            break;
            case COMREC_STATE_DATADEC:     // active sequence, store incoming data into buffer
                
				switch(RXByte)
                {
					case STX: //Start over if STX
						checksum = 0;
						dataCount = 0;
                        
                        // DEBUG: start sequence
                        DEBUG_puts("[");                       
                    break;

					case ETX: //End of packet if ETX
                        
                        //Test checksum
						checksum = ~checksum + 1;                                   
						                        
						if(checksum == 0) 
                        {
                            // checksum okay, continue with Command Decoding
                            status = COMREC_DECODE_DONE;
                            
                            DEBUG_puts("] OK\n\r");
                        }
                        else
                        {
                            // checksum error.
                            status = COMREC_ERROR_CHECKSUM;
                            dataCount = 0xFFFF;                                         //Otherwise restart                            
                            
                            DEBUG_puts("] Checksum error\n\r");
                        }
					break;

					case DLE: //If DLE, treat next as data
                        
                        // signal receiver state machine that next byte is
                        // to be treated as data
                        comRecState = COMREC_STATE_NEXTISDATA;
                        
                    break;
					default: // regular data byte
						checksum += RXByte;
						buffer[dataCount++] = RXByte;
                        
                        sprintf(txt,"%02X",RXByte);
                        DEBUG_puts(txt);
                        
                        if(dataCount <= MAX_PACKET_SIZE+1)
                        {
                            ; // everything ok
                        }
                        else
                        {
                            // max. packet size exceeded
                            status = COMREC_ERROR_MAX_PACKETSIZE;
                        }
					break;        
                }
                
            break;
            case COMREC_STATE_NEXTISDATA:     // active sequence, treat current byte as data without inspection
                
                // treat as regular data byte
                checksum += RXByte;
                buffer[dataCount++] = RXByte;

                sprintf(txt,"%02X",RXByte);
                DEBUG_puts(txt);
                
                
                // set state machine back to default decoding mode
                comRecState = COMREC_STATE_DATADEC;                
                
                if(dataCount <= MAX_PACKET_SIZE+1)
                {
                    ; // everything ok
                }
                else
                {
                    // max. packet size exceeded
                    status = COMREC_ERROR_MAX_PACKETSIZE;
                }
                
            break;            
        }
    
        
        if(status > 0)
        {
            // leave receiving loop upon error or completed package
            break;
        }
    }

 
    // handle status code
    switch(status)
    {
        case COMREC_DECODE_DONE:
            // reset receiver state machine
            comRecState = COMREC_STATE_IDLE;

            // decode received package
            BTCom_HandleCommand(buffer,buffer,dataCount);
            
            // send response
            BTCom_PutResponseDebug();
        break;
        default:
            // handle error
        break;
    }
    

    
}


