// AutoDuct bootloader main module
// (C) 2023-09-09 by Daniel Porzig

#include "HardwareProfile.h"
#include "uart2.h"
#include "uart1.h"
#include "circbuffer.h"
#include <plib.h>
#include <proc/p32mx150f128b.h>
#include "Delay.h"
#include "BTCom.h"
#include "BTComCallbacksBootloader.h"
#include "BootLoader.h"


// original config
/** CONFIGURATION **************************************************/

// oscillator setup for 40 MHz system clock
#pragma config FPLLIDIV = DIV_2         // PLL Input Divider
#pragma config FPLLMUL  = MUL_20        // PLL Multiplier       
#pragma config FPLLODIV = DIV_2         // PLL Output Divider

#pragma config FUSBIDIO = OFF                  // Controlled by Port Function  
#pragma config FVBUSONIO = OFF                 // Controlled by Port Function  

// PBClock 40 MHz
#pragma config FPBDIV   = DIV_1         // Peripheral Clock divisor

#pragma config FNOSC = FRCPLL           // Fast RC Osc (FRC) with PLL  
#pragma config FSOSCEN = ON             // Secondary osc enabled (RTCC)
#pragma config IESO = OFF               // Internal/External Switch Over disabled
#pragma config POSCMOD = OFF            // Primary osc disabled  
#pragma config OSCIOFNC = OFF           // CLKO Output Signal  Disabled  
#pragma config FCKSM = CSDCMD           // Clock Switch Disable, FSCM Disabled  
#pragma config WDTPS = PS4096           // Watchdog Timer Postscale (4096 should be ~4 seconds)
#pragma config FWDTEN = OFF             // WDT Disabled (SWDTEN Bit Controls)  
#pragma config DEBUG = OFF              // Debugger is disabled  
#pragma config ICESEL = ICS_PGx1        // ICE EMUC1/EMUD1 pins shared with PGC1/PGD1  
#pragma config PWP = OFF                // Flash write protect Disabled
#pragma config BWP = OFF                // Boot Flash Write Protection Disabled  
#pragma config CP = OFF                 // Protection Disabled  
        
#pragma config JTAGEN = OFF             // JTAG Disabled  

#pragma config PMDL1WAY = OFF           // Allow multiple reconfigurations  
#pragma config IOL1WAY = OFF            // Allow multiple reconfigurations  

// firmware version string:
//  2 digits - Major FW version
//  2 chars  - TC: test version, PR: production version
//  6 digits - date: YYMMDD
//  1 digit  - minor FW version

const char __attribute__((space(prog),address(BOOTCODE_FWSTRING_BASE_ADDRESS)))  FW_VERS_STRING[] = "01bc220922.0";        // 13 chars

#define PROGRAM_BLOCK_SIZE          64

#define BOOTCODE_INIT         0x00
#define BOOTCODE_ACTIVE       0x01
#define BOOTCODE_START_APP    0x02

#define BOOTCODE_RES_OK             0x00
#define BOOTCODE_RES_PKT_LOSS       0x01
#define BOOTCODE_RES_CHKSUM_ERR     0x02
#define BOOTCODE_RES_PRGM_DONE      0x03
#define BOOTCODE_RES_SYNTAX_ERR     0x04
#define BOOTCODE_RES_IMAGE_INVALID  0x05



#define BOOTCODE_HOOKING_WINDOW        6    // number of seconds the bootloader will wait before jumping to app
                                            // (allows FW update after reset even if main app is corrupt)


#pragma pack(push,2)
typedef struct imageHeader_TD
{
        WORD signature;			// signature
        DWORD size;				// size of image in bytes (excluding header)
        //time_t	builddate;		// timecode of build date
        INT64       builddate;		// timecode of build date
		WORD crc16;				// CRC16 checksum (image only)
		char FWrevstr[13];		// Firmware revision string         
}imageHeader;
#pragma pack(pop) // disables the effect of #pragma pack from now on


BYTE Bootcode_state = BOOTCODE_INIT;
WORD Bootcode_expected_Block = 0x0000;
WORD Bootcode_cnt = 0;
cfg_bootcode_struct cfg_bootcode;

BYTE LEDblink_state = 0;
WORD LEDblink_cnt = 0;

char txt[256];



static void InitializeBoard(void);
BOOL CheckTrigger(void);
void JumpToApp(void);
BOOL ValidAppPresent(void);






 static enum {
      EXCEP_IRQ = 0,            // interrupt
      EXCEP_AdEL = 4,            // address error exception (load or ifetch)
      EXCEP_AdES,                // address error exception (store)
      EXCEP_IBE,                // bus error (ifetch)
      EXCEP_DBE,                // bus error (load/store)
      EXCEP_Sys,                // syscall
      EXCEP_Bp,                // breakpoint
      EXCEP_RI,                // reserved instruction
      EXCEP_CpU,                // coprocessor unusable
      EXCEP_Overflow,            // arithmetic overflow
      EXCEP_Trap,                // trap (possible divide by zero)
      EXCEP_IS1 = 16,            // implementation specfic 1
      EXCEP_CEU,                // CorExtend Unuseable
      EXCEP_C2E                // coprocessor 2
  } _excep_code;
  
  static unsigned int _epc_code;
  static unsigned int _excep_addr;

  void _general_exception_handler(void)
  {
      asm volatile("mfc0 %0,$13" : "=r" (_excep_code));
      asm volatile("mfc0 %0,$14" : "=r" (_excep_addr));
  
      _excep_code = (_excep_code & 0x0000007C) >> 2;
      
      DEBUG_puts("\r\nGeneral Exception ");
      switch(_excep_code){
        case EXCEP_IRQ: DEBUG_puts ("interrupt");break;
        case EXCEP_AdEL: DEBUG_puts ("address error exception (load or ifetch)");break;
        case EXCEP_AdES: DEBUG_puts ("address error exception (store)");break;
        case EXCEP_IBE: DEBUG_puts ("bus error (ifetch)");break;
        case EXCEP_DBE: DEBUG_puts ("bus error (load/store)");break;
        case EXCEP_Sys: DEBUG_puts ("syscall");break;
        case EXCEP_Bp: DEBUG_puts ("breakpoint");break;
        case EXCEP_RI: DEBUG_puts ("reserved instruction");break;
        case EXCEP_CpU: DEBUG_puts ("coprocessor unusable");break;
        case EXCEP_Overflow: DEBUG_puts ("arithmetic overflow");break;
        case EXCEP_Trap: DEBUG_puts ("trap (possible divide by zero)");break;
        case EXCEP_IS1: DEBUG_puts ("implementation specfic 1");break;
        case EXCEP_CEU: DEBUG_puts ("CorExtend Unuseable");break;
        case EXCEP_C2E: DEBUG_puts ("coprocessor 2");break;
      }
//      Uart1TxStringPolled(" at 0x");
//      Uart1TxUint32HexPolled(_excep_addr);
//      Uart1TxStringPolled("\r\n");
      while (1) 
      {
          asm("nop");
          // Examine _excep_code to identify the type of exception
          // Examine _excep_addr to find the address that caused the exception
      }
      
  }





void UserInput_Task(void *pvParameters, BYTE *skiprate)
{
    BTCom_Task();
    
    *skiprate = 1;
}

void LEDblink_Task(void *pvParameters, BYTE *skiprate)
{
    switch(LEDblink_state)
    {
        case 0:
            // do noting
        break;
        case 1:
            // blink fast
            if(LEDblink_cnt == 0)
            {
                STATLED = !STATLED;
                LEDblink_cnt = 20;
            }
            else
                LEDblink_cnt--;               
        break;
        case 2:
            // blink slow
            if(LEDblink_cnt == 0)
            {
                STATLED = !STATLED;
                LEDblink_cnt = 125;
            }
            else
                LEDblink_cnt--;               
        break;
        
    }
    
    *skiprate = 1;
}


void LEDblink_setMode(BYTE mode)
{
    LEDblink_cnt = 0;
    LEDblink_state = mode;
    if(mode == 0)
        STATLED = 1;    // switch off LED
}

void LEDblink_toggle()
{
    STATLED = !STATLED;
}


void SaveBootcodeConfig()
{
    EEPROM_write(BOOTCODE_CONFIG_EEPROM_ADD, &cfg_bootcode, sizeof(cfg_bootcode_struct));
}

void LoadBootcodeConfig()
{
    EEPROM_read(BOOTCODE_CONFIG_EEPROM_ADD, &cfg_bootcode, sizeof(cfg_bootcode_struct));
}





// erase complete user app flash region
void Bootloader_eraseFlash()
{
    DWORD i;
    void* pFlash;

    
    DEBUG_puts("\n\rErasing Flash...");   
    
    // erase App Flash region   
    pFlash = (void*)APP_FLASH_BASE_ADDRESS;									
    for( i = 0; i < ((APP_FLASH_END_ADDRESS - APP_FLASH_BASE_ADDRESS + 1)/FLASH_PAGE_SIZE_PIC32MX1); i++ )
    {
        NVMemErasePage( pFlash + (i*FLASH_PAGE_SIZE_PIC32MX1) );
    }	    
    
    DEBUG_puts("Done.\n\r");       
}




BYTE Bootloader_programBlock(DWORD offset, BYTE *data, BYTE size)
{
    DWORD len;
    DWORD WrData;
    void* pFlash;
    BYTE *bptr;

    len = size;
    bptr = data;
    
    //sprintf(txt,"Writing Block 0x%04X...\n\r",offset);
    //DEBUG_puts(txt);       
    
    DEBUG_puts("Writing Block 0x");  
    UART2PutHexWord( offset );
    DEBUG_puts("...");  
    
    
    // set destination base address
    pFlash = (void*)APP_FLASH_START_ADDRESS + offset;
    
    while(len > 0)
    {
        
        if(len < 4)
        {

            // Sometimes record data length will not be in multiples of 4. Appending 0xFF will make sure that..
            // we don't write junk data in such cases.
            WrData = 0xFFFFFFFF;
            memcpy(&WrData, bptr, len);	
            len = 0;
        }
        else
        {	
            memcpy(&WrData, bptr, 4);
            len -= 4;
        }	        

        // write data word to Flash
        NVMemWriteWord(pFlash, WrData);

        // move data pointers
        pFlash += 4;
        bptr += 4;     
	} 
    
    DEBUG_puts("Done.\n\r");   
}


void Bootloader_BTcomCallback_Status(BYTE *buf_in, BYTE *buf_out, BYTE *responseBytes)
{
//    FLOAT_VAL   fVal;
//    WORD_VAL    wVal;
    
    WORD_VAL wVal1, wVal2;    
    
    BYTE *bptr = &buf_out[1];
    BYTE *ptr;
    BYTE j, HumctlFan;

	// outbuf[0] = CMD              // 0

    buf_out[1] = 0xF1;  // bootloader mode
            
    buf_out[2] = Bootcode_state;     // bootloader status
    
    buf_out[3] = 0;
    
    // current Temperature
    buf_out[4] = 0;
    buf_out[5] = 0;
    
    // current Humidity
    buf_out[6] = 0;
    buf_out[7] = 0;
    
    // venting indicator
    buf_out[8] = 0;
    
    // reserved
    buf_out[9]  = 0xFF;
    buf_out[10] = 0xFF;
    buf_out[11] = 0xFF;
    buf_out[12] = 0xFF;
    buf_out[13] = 0xFF;
    buf_out[14] = 0xFF;
    buf_out[15] = 0xFF;
    
    *responseBytes = 16;    
}






//
//
//
//#define FIRMWARE_IMG_SIGNATURE      0xA2F6
//
//
//#pragma pack(push,2)
//typedef struct imageHeader_TD
//{
//        WORD signature;			// signature
//        DWORD size;				// size of image in bytes (excluding header)
//        //time_t	builddate;		// timecode of build date
//        INT64       builddate;		// timecode of build date
//		WORD crc16;				// CRC16 checksum (image only)
//		char FWrevstr[13];		// Firmware revision string         
//}imageHeader;
//#pragma pack(pop) // disables the effect of #pragma pack from now on




void Bootloader_BTcomCallback(BYTE *buf_in, BYTE *buf_out, BYTE *responseBytes)
{
    DWORD offset, blockID;
    WORD_VAL wval;
    imageHeader *Header = NULL;
    
    
    // buf_in[0] = CMD
    // buf_in[1] = BlockID High Byte
    // buf_in[2] = BlockID Low Byte
    // buf_in[3] = Data Length
    // buf_in[4..n] = Data Bytes
    

    // get block ID
    wval.v[1] = buf_in[1];
    wval.v[0] = buf_in[2];  
    blockID = wval.Val;    
    
    
    // check special case: firmware update initialization sequence
    if(buf_in[1] == 0xFF && buf_in[2] == 0xFF && buf_in[3] == 2 && buf_in[4] == 0xBE && buf_in[5] == 0xEF)
    {
        // initialization sequence detected
        
        // - if a bootloading operation is ongoing, it will be reset to beginning
        // - if bootcode is in hooking window, it will be catched
        
        Bootcode_expected_Block = 0;
        Bootcode_state = BOOTCODE_ACTIVE;
        LEDblink_setMode(2);    // blink slow

        DEBUG_puts("Bootcode hook catched. Starting Bootloader mode.\n\r");                
        
        // return OK code
        buf_out[1] = BOOTCODE_RES_OK;        
        
        
    }
    else if(buf_in[1] == 0x00 && buf_in[2] == 0x00)
    {
        // first block:
        // - check if BlockID was expected
        // - start app area flash erase procedure
        // - update magic number in EEPROM
        // - skip header bytes and program block to flash
        
        // check if blockID matches expectations
        if(blockID != Bootcode_expected_Block)
        {
            // return packet loss error code
            buf_out[1] = BOOTCODE_RES_PKT_LOSS;
            Bootcode_state = BOOTCODE_INIT;
        }
        else if(buf_in[3] > PROGRAM_BLOCK_SIZE)
        {
            // unexpected data size
            
            // return syntax error code
            buf_out[1] = BOOTCODE_RES_SYNTAX_ERR;
            Bootcode_state = BOOTCODE_INIT;            
        }
        else
        {
            // extract header from first Firmware block
            Header = (imageHeader*)&buf_in[4];
            
            sprintf(txt,"Image Signature:   %04X\n\r",Header->signature);
            DEBUG_puts(txt);     
            sprintf(txt,"Image CRC16:       %04X\n\r",Header->crc16);
            DEBUG_puts(txt);     
            sprintf(txt,"Image Size:        %u\n\r",Header->size);
            DEBUG_puts(txt);         

            if(Header->signature == 0xA2F6)
            {
                // reset magic number in EEPROM config 
                cfg_bootcode.magicnumber = MAGIC_NUMBER_INVALID_APP;
                SaveBootcodeConfig();

                // erase user app flash region
                Bootloader_eraseFlash();     
                
            // do not program first block, as it only contains the header
            // padded to 64 Bytes                
//                offset = 0x00000000;
//                Bootloader_programBlock(offset, &buf_in[4], buf_in[3]);
                
                LEDblink_setMode(0);    // switch to toggle mode

                Bootcode_expected_Block++;

                // return OK code
                buf_out[1] = BOOTCODE_RES_OK;                
                
            }
            else
            {
                DEBUG_puts("Signature of Image invalid!\n\r");   
                // return error code
                buf_out[1] = BOOTCODE_RES_IMAGE_INVALID;
                Bootcode_state = BOOTCODE_INIT; 
            }
                
        }
    }
    else
    {
        // regular block -> program to flash
        
        // check if blockID matches expectations
        if(blockID != Bootcode_expected_Block)
        {
            // return packet loss error code
            buf_out[1] = BOOTCODE_RES_PKT_LOSS;
            Bootcode_state = BOOTCODE_INIT;
        }
        else
        {        
        
            // check if a dummy block (data length = 0) was sent
            if(buf_in[3] == 0)
            {
                // dummy block. Image transfer complete.
                
                if(ValidAppPresent())
                {
                    // update magic number in EEPROM config 
                    cfg_bootcode.magicnumber = MAGIC_NUMBER_VALID_APP;
                    SaveBootcodeConfig();

                    // return Programming done code
                    buf_out[1] = BOOTCODE_RES_PRGM_DONE;     


                    Bootcode_cnt = 50;
                    Bootcode_state = BOOTCODE_START_APP;
                    
                    LEDblink_setMode(1);    // blink fast
                }
                else
                {
                    // return Programming failed error code
                    buf_out[1] = BOOTCODE_RES_IMAGE_INVALID;
                    Bootcode_state = BOOTCODE_INIT;
                }

            }
            else if(buf_in[3] > PROGRAM_BLOCK_SIZE)
            {
                // unexpected data size

                // return syntax error code
                buf_out[1] = BOOTCODE_RES_SYNTAX_ERR;
                Bootcode_state = BOOTCODE_INIT;            
            }
            else
            {

                // calculate offset (first block is skipped = header)
                offset = (blockID - 1) * PROGRAM_BLOCK_SIZE;
                
                // program block
                Bootloader_programBlock(offset, &buf_in[4], buf_in[3]);

                Bootcode_expected_Block++;
                
                LEDblink_toggle();

                // return OK code
                buf_out[1] = BOOTCODE_RES_OK;            
            }
        
        }
    }
    
    
    // return codes:   
    //0x00 = OK
    //0x01 = Error, packet lost
    //0x02 = Error, checksum mismatch
    //0x03 = Image programming done, firmware update started
    //0x04 = Error, unexpected syntax
    //0x05 = Error, firmware image invalid
    
//#define BOOTCODE_RES_OK             0x00
//#define BOOTCODE_RES_PKT_LOSS       0x01
//#define BOOTCODE_RES_CHKSUM_ERR     0x02
//#define BOOTCODE_RES_PRGM_DONE      0x03
//#define BOOTCODE_RES_SYNTAX_ERR     0x04
//#define BOOTCODE_RES_IMAGE_INVALID  0x05
    
    
    
    
    *responseBytes = 2;
}




void cmd_dev_reset_callback(BYTE *buf_in, BYTE *buf_out, BYTE *responseBytes)
{           
    if(cfg_bootcode.magicnumber == MAGIC_NUMBER_UPDATE_REQ)
    {
        // check if a valid app is present
        if(ValidAppPresent())
        {               
            // reset magic number in EEPROM config to remove bootloading request
            cfg_bootcode.magicnumber = MAGIC_NUMBER_VALID_APP;
            SaveBootcodeConfig();
        }
    }
            
    
    // buf_in[0] = CMD
    // buf_in[1] = safety code 1
    // buf_in[2] = safety code 2
    
    *responseBytes = 2;    // set response length to 2 byte

    // check safety codes
    if(buf_in[1] == 0xE0 && buf_in[2] == 0x9B)
    {
        // safety codes match. initiate reset
        
        buf_out[1] = 0x00;  // OK
        
        // put response to BLE stack before resetting
        BTCom_PutResponse();
        
        Delayms(50);
        
        Reset();
    }
    else
    {
        // safety codes do not match, return error code
        
        buf_out[1] = 0x01;  // safety code mismatch
        
    }
    
}            
        





void Bootloader_Task(void *pvParameters, BYTE *skiprate)
{
    
    // TODO:
    // - handle timeouts?
    
    
    
    switch(Bootcode_state)
    {
        case BOOTCODE_INIT:
            
            // check Trigger Info in external EEPROM
            
            // if no valid App found, reset magic number in EEPROM


            // read magic number from external EEPROM
            LoadBootcodeConfig();
            
            
            if(cfg_bootcode.magicnumber == MAGIC_NUMBER_VALID_APP)
            {
                // check if a valid app is present
                if(ValidAppPresent())
                {
                    // execute user app
                    Bootcode_cnt = BOOTCODE_HOOKING_WINDOW * 250;
                    Bootcode_state = BOOTCODE_START_APP;
                    LEDblink_setMode(1);    // blink fast
                    
                    DEBUG_puts("Preparing to boot user app...\n\r"); 
                }
                else
                {
                    // installed app seems broken
                    // reset magic number in EEPROM config and start bootloader code
                    cfg_bootcode.magicnumber = 0x00000000;
                    SaveBootcodeConfig();
                    Bootcode_expected_Block = 0;
                    Bootcode_state = BOOTCODE_ACTIVE;
                    LEDblink_setMode(2);    // blink slow
                    
                    DEBUG_puts("Invalid App. Starting Bootloader mode.\n\r");   
                }
            }
            else if(cfg_bootcode.magicnumber == MAGIC_NUMBER_UPDATE_REQ)
            {
                // a valid app seems to be present but an update request was issued

                // start bootloader code
                Bootcode_state = BOOTCODE_ACTIVE;
                LEDblink_setMode(2);    // blink slow
                DEBUG_puts("Update requested. Starting Bootloader mode.\n\r");   
            }
            else
            {
                // no valid app present, start bootloader
                Bootcode_state = BOOTCODE_ACTIVE;
                LEDblink_setMode(2);    // blink slow
                DEBUG_puts("Invalid App. Starting Bootloader mode.\n\r");   
            }
            
            
        break;
            
        case BOOTCODE_ACTIVE:
            
            // housekeeping for Bootloader process
            
            // TODO: handle timeouts:
            // - timeout, in case cfg_bootcode.magicnumber == MAGIC_NUMBER_UPDATE_REQ
            //   (update was requested but no blocks received so far)
            //
            // - timeout after 10 seconds with no block transfers during programming
            
        
        break;
            
        
        case BOOTCODE_START_APP:
            
            // jump to user app after a defined delay
            if(Bootcode_cnt == 0)
            {
                // execute user app
                JumpToApp();                
            }
            else
                Bootcode_cnt--;
            
            
        break;
        
        default:
        
        break;
    }
    
    *skiprate = 1;
}








int main(void)
{ 
   
	// Initialize application specific hardware
	InitializeBoard();
//    Delay10us(20000);   
    Delayms(20);   
    
//    
//    while(1)
//    {
//        
//        if(SYSLED == 1)
//        {
//            SYSLED      = 0;
//        }
//        else
//        {
//            SYSLED      = 1;
//        }        
//        
//        Delay10us(20000);
//        Delay10us(20000);
//        Delay10us(20000);
//        Delay10us(20000);
//        Delay10us(20000);
//        Delay10us(20000);
//        Delay10us(20000);
//        
//    }
//    
//    
    
    
	UART2Init();   
    UART1Init();
 
    DEBUG_puts("\n\r");   
    DEBUG_puts("\n\r");   
    DEBUG_puts("\n\r");   
    DEBUG_puts("\n\r");   
    
    DEBUG_puts("\n\r+--------------------------------------------------+");   
    DEBUG_puts("\n\r|              AutoDuct Bootloader                 |");   
    DEBUG_puts("\n\r+--------------------------------------------------+");   
    DEBUG_puts("\n\r");   
    DEBUG_puts("\n\rFirmware version: ");       
    DEBUG_puts(FW_VERS_STRING);           
    
    Scheduler_Init();    
 
    BTCom_Init();
    BTCom_SetupCallbacks();

    EEPROM_init(0x50);
    
    LEDblink_setMode(0);
 
    DEBUG_puts("\n\r\n\rBoard Init complete.\n\r\n\r");        
      
    Scheduler_AddTask(0, Bootloader_Task, NULL, 1, 0);
    Scheduler_AddTask(1, LEDblink_Task, NULL, 1, 0);
    Scheduler_AddTask(2, UserInput_Task, NULL, 1, 0);        
     
    while(1)
    {    
        Scheduler_Run();        
    }

}







#define BRG_VAL 146   // actual val: 46.4 for 400 kHz @ 40 MHz PBclock


/****************************************************************************
  Function:
    static void InitializeBoard(void)

  Description:
    This routine initializes the hardware.  It is a generic initialization
    routine for many of the Microchip development boards, using definitions
    in HardwareProfile.h to determine specific initialization.

  Precondition:
    None

  Parameters:
    None - None

  Returns:
    None

  Remarks:
    None
  ***************************************************************************/
static void InitializeBoard(void)
{	
    
    ANSELA = 0x0000;       // all pins digital
    ANSELB = 0x0000;        // all pins digital
    
//    ANSELBbits.ANSB12 = 1;  // pin B12 used for motor current sense
    
    
	// +------------------------------+
	// | PIN SETUP                    |
	// +------------------------------+

	//UART 2   - FTDI
	RX_CON_TRIS = 1;
	TX_CON_TRIS = 0;

    //UART 1   - HM-17
 	RX_BLE_TRIS	= 1; // input (RX)
 	TX_BLE_TRIS = 0; // output (TX)    
    
    // % LED INDICATORS										
    // SYSLED	PWM	OC5	52	RD4				
    SYSLED_TRIS = 0;
    SYSLED = 1;
    
    STATLED_TRIS = 0;
    STATLED = 0;
    
    MOTOR_DIR_TRIS = 0;
    MOTOR_PWM_TRIS = 0;
    MOTOR_SLEEP_TRIS = 0;
    MOTOR_SLEEP = 1;
    MOTOR_PWM = 0;
    
    MOTOR_FAULT_TRIS = 1;
    MOTOR_ADC_TRIS = 1;

    // PWM in/out
    FAN_PWM_IN_TRIS = 1;
    FAN_PWM_OUT_TRIS = 0;
    FAN_PWM_OUT_ODC = 0;        // enable open drain config

    // end stop sensors
    ENDSTOP_LOW_TRIS = 1;
    ENDSTOP_HIGH_TRIS = 1;

        
 	// +------------------------------+
	// | Setup PPS                    |
	// +------------------------------+
   
    // defined in <pps.h>

    // inputs
    PPSInput(2, INT3, RPB11); //INT3 -> B11       
    PPSInput(3, INT2, RPB13); //INT2 -> B13               
    //PPSInput(3, IC1, RPA2); //IC1 -> A2    // not used in Bootloader mode
    PPSInput(2, U2RX, RPB5); //U2RX -> B5        
    PPSInput(4, INT1, RPB14); //INT1 -> B14  
    PPSInput(3, U1RX, RPB2); //U1RX -> B2    
    
    
    // outputs
    //PPSOutput(1, RPA0, OC1); //OC1  -> A0    // not used in Bootloader mode
    //PPSOutput(2, RPA1, OC2); //OC2  -> A1    // not used in Bootloader mode
    PPSOutput(4, RPA3, U2TX); //U2TX -> A3
    //PPSOutput(4, RPB10, OC3); //OC3  -> B10    // not used in Bootloader mode
    //PPSOutput(3, RPB6, OC5); //OC5 -> B6    // not used in Bootloader mode    
    PPSOutput(1, RPB3, U1TX); //U1TX -> B3
    
    
	// configure for multi-vectored mode
	INTConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);
	
	// enable interrupts
	INTEnableInterrupts();

	mJTAGPortEnable(DEBUG_JTAGPORT_OFF);					// JTAG port must be disabled to use PORTA
		
    // Enable optimal performance
    SYSTEMConfigPerformance(GetSystemClock());
    mOSCSetPBDIV(OSC_PB_DIV_1);				// Use 1:1 CPU Core:Peripheral clocks
		
    
    // Disable JTAG port so we get our I/O pins back, but first
    DDPCONbits.JTAGEN = 0;
       

    // setup I2C module for EEPROM access
    OpenI2C1(I2C_ON , BRG_VAL);
    I2C1CONbits.DISSLW = 1;     // workaround for silicon issue
    IdleI2C1();     //Wait to complete  
   
    
}



/********************************************************************
* Function: 	JumpToApp()
*
* Precondition: 
*
* Input: 		None.
*
* Output:		
*
* Side Effects:	No return from here.
*
* Overview: 	Jumps to application.
*
*			
* Note:		 	None.
********************************************************************/
void JumpToApp(void)
{	
	void (*fptr)(void);
	fptr = (void (*)(void))USER_APP_RESET_ADDRESS;
	fptr();
}	


/********************************************************************
* Function: 	ValidAppPresent()
*
* Precondition: 
*
* Input: 		None.
*
* Output:		TRUE: If application is valid.
*
* Side Effects:	None.
*
* Overview: 	Logic: Check application vector has 
				some value other than "0xFFFFFFFF"
*
*			
* Note:		 	None.
********************************************************************/
BOOL ValidAppPresent(void)
{
	DWORD *AppPtr;
	AppPtr = (DWORD *)USER_APP_RESET_ADDRESS;
	if(*AppPtr == 0xFFFFFFFF)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}			




