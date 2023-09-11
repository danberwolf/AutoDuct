// AutoDuct app firmware BLE protocol commands and callbacks
// (C) 2023-09-09 by Daniel Porzig

#include "BTComCallbacksApp.h"
//#include "Config.h"
#include "DeviceControl.h"
#include "BootLoader.h"


// command table
#define CMD_RESERVED        0x00    
#define CMD_MANVENT         0x01        // manually activate/disable venting, set duration   
#define CMD_SETMODE         0x02        // set device mode (passive, manual, smart)
#define CMD_STATUS          0x03        // read various status information
#define CMD_READCONFIG      0x04        // new for Wordclock
#define CMD_UPDATECONFIG	0x05        // new for Wordclock
#define CMD_FWREV           0x06
#define CMD_POWERCFG        0x07        // set power mode
#define CMD_SCROLLTEXTTEST  0x08        // new for Wordclock
#define CMD_CLOCKSYNC       0x09        // new for Wordclock
#define CMD_GETCLOCK        0x0A        
#define CMD_DEV_PROGRAM             0xF0
#define CMD_DEV_RESET               0xF1
#define CMD_DEV_TESTMODE            0xFB
#define CMD_DEV_ECHO                0xFC
//#define CMD_DEV_RESET               0xFD
//#define CMD_DEV_BOOTL               0xFE

extern const char FW_VERS_STRING[];     // example: "00TC180215.3";  


void SaveBootcodeConfig();
void LoadBootcodeConfig();
void Bootcode_setMagicNumber(DWORD magic);



void BTCom_defaultCallback(BYTE *buf_in, BYTE *buf_out, BYTE* len)
{
    //responseBytes = *len-1;  // echo back entire data packet (last data byte is checksum)        
    *len = *len-1;
}


void SendFWStringBT(BYTE *inbuf, BYTE *outbuf, WORD *len)
{
    BYTE i;
    char fwstr[20];
    char *storedFWstr;    
    
// firmware version string:
//  2 digits - Major FW version
//  2 chars  - TC: test version, PR: production version
//  6 digits - date: YYMMDD
//  1 digit  - minor FW version
        
    BYTE *bptr = &outbuf[1];    
      
    // send currently running FW revision string
    for(i=0; i<12; i++)
    {
        *bptr++ = FW_VERS_STRING[i];
    }
    *bptr++ = '\0';
    
    
    // send bootloader FW revision string
    
    // get global address of FW string in app program memory space
    storedFWstr = PA_TO_KVA0(BOOTCODE_FWSTRING_BASE_ADDRESS); 

    // copy string
    for(i = 0; i<12; i++)
    {
        *bptr++ = storedFWstr[i];
    }
    *bptr++ = '\0';
    
    // send installed app FW revision string
    
    // get global address of FW string in app program memory space
    storedFWstr = PA_TO_KVA0(APP_FWSTRING_BASE_ADDRESS); 

    // copy string
    for(i = 0; i<12; i++)
    {
        *bptr++ = storedFWstr[i];
    }
    *bptr++ = '\0';
    
  
    // reserved FW revision string
    strcpy(fwstr, "  [invalid] ");
    
    for(i=0; i<12; i++)
    {
        *bptr++ = fwstr[i];
    }
    *bptr++ = '\0';    

    *len = (13 * 4) + 1 ;  // 4 FW revision strings, 13 chars each
}


void cmd_manvent_callback(BYTE *buf_in, BYTE *outbuf, BYTE *responseBytes)
{
    // CMD              buf_in[0]

    // fan mode         buf_in[1]
    // fan direction    buf_in[2]
    // fan speed        buf_in[3]
    // duration         buf_in[4]


    DeviceControl_ManualVent(buf_in[1], buf_in[2], buf_in[3], buf_in[4]);


    *responseBytes = 1;    // set response length to 1 byte  (just echo command)
}


void cmd_setmode_callback(BYTE *buf_in, BYTE *outbuf, BYTE *responseBytes)
{
    // CMD              buf_in[0]
    // mode             buf_in[1]

    DeviceControl_SetMode(buf_in[1]);

    *responseBytes = 1;    // set response length to 1 byte  (just echo command)
}

        
void cmd_readconfig_callback(BYTE *buf_in, BYTE *buf_out, BYTE *responseBytes)
{
            
    // buf_in[0] = CMD
    // buf_in[1] = ID

    ConfigBlueToothOutput(buf_in[1], buf_out, responseBytes);
}


void cmd_updateconfig_callback(BYTE *buf_in, BYTE *buf_out, BYTE *responseBytes)
{
    Config_UpdateFragment(buf_in[1], &buf_in[2]);

    *responseBytes = 1;    // set response length to 1 byte  (just echo command)
}    


void cmd_status_callback(BYTE *buf_in, BYTE *buf_out, BYTE *responseBytes)
{
    DeviceControl_StatusBTCom(buf_out, responseBytes);   
}


void cmd_clocksync_callback(BYTE *buf_in, BYTE *buf_out, BYTE *responseBytes)
{
//                cmddata[1] = (byte)rnow.get(Calendar.HOUR_OF_DAY);
//                cmddata[2] = (byte)rnow.get(Calendar.MINUTE);
//                cmddata[3] = (byte)rnow.get(Calendar.SECOND);
//
//                cmddata[4] = (byte)rnow.get(Calendar.DAY_OF_WEEK);    // Android: 1..7, Timekeeper: 0..6
//                cmddata[5] = (byte)rnow.get(Calendar.DAY_OF_MONTH);
//                cmddata[6] = (byte)rnow.get(Calendar.MONTH);      // january = 0
//                cmddata[7] = (byte)rnow.get(Calendar.YEAR);            

    // Android time starts with Sunday = 1 (needs to be converted for Timekeeper), January = 0
    TimeKeeper_setTimeDate(buf_in[4], buf_in[5], buf_in[6] + 1, buf_in[7], buf_in[1], buf_in[2], buf_in[3], 1, (buf_in[8] == 0xF1)?1:0);
    
    *responseBytes = 1;    // set response length to 1 byte  (just echo command)    
}


void cmd_getclock_callback(BYTE *buf_in, BYTE *buf_out, BYTE *responseBytes)
{
//                cmddata[1] = (byte)rnow.get(Calendar.HOUR_OF_DAY);
//                cmddata[2] = (byte)rnow.get(Calendar.MINUTE);
//                cmddata[3] = (byte)rnow.get(Calendar.SECOND);
//
//                cmddata[4] = (byte)rnow.get(Calendar.DAY_OF_WEEK);
//                cmddata[5] = (byte)rnow.get(Calendar.DAY_OF_MONTH);
//                cmddata[6] = (byte)rnow.get(Calendar.MONTH);      // january = 0
//                cmddata[7] = (byte)rnow.get(Calendar.YEAR);            

    // Android time starts with Sunday = 1, January = 0
    
    TimeKeeper_getTimeDate(&buf_out[4], &buf_out[5], &buf_out[6], &buf_out[7],&buf_out[1], &buf_out[2], &buf_out[3]);

    buf_out[6] -= 1;    // Android Month starts with 0
    //buf_out[4] += 1;    // Android day of week starts with 1 (Sunday)
    
    *responseBytes = 8;    // set response length to 8 byte
}


            
void cmd_dev_program_callback(BYTE *buf_in, BYTE *buf_out, BYTE *responseBytes)
{           

    // TODO:
    // - setup magic number in EEPROM
    // - send response code
    // - reset device to enter bootloader mode
    
    
    // buf_in[0] = CMD
    // buf_in[1] = ID   
    
    // check special case: firmware update initialization sequence
    if(buf_in[1] == 0xFF && buf_in[2] == 0xFF && buf_in[3] == 2 && buf_in[4] == 0xBE && buf_in[5] == 0xEF)
    {
        // initialization sequence detected
        DEBUG_puts("Firmware update request detected. Starting Bootloader mode...\n\r");
        
        // return OK code
        buf_out[1] = 0x00;        
        *responseBytes = 2;    // set response length to 2 byte        
        
        
        // program firmware update request to external EEPROM 
        LoadBootcodeConfig();
        Bootcode_setMagicNumber(MAGIC_NUMBER_UPDATE_REQ);
        SaveBootcodeConfig();        
        
        // put response to BLE stack before resetting
        BTCom_PutResponse();
        
        // reset system
        Delayms(150);
        Reset();        
    }    
    else
    {
        // all other packets get rejected with syntax error result code
        
        *responseBytes = 2;    // set response length to 2 byte
        buf_out[1] = 0x04;     // unexpected syntax error code
    }
    
}            


void cmd_dev_testmode_callback(BYTE *buf_in, BYTE *buf_out, BYTE *responseBytes)
{           

    
    // buf_in[0] = CMD
    // buf_in[1] = sub-command   
    switch(buf_in[1])
    {
        case 0: // activate alternating test mode
            DEBUG_puts("DEV_TESTMODE: alternate\n\r");
            DeviceControl_Testmode(DEVCTL_STATE_TESTMODE1);
        break;
        case 1: // eject / jettison valve piston
            DEBUG_puts("DEV_TESTMODE: piston eject\n\r");
            Valve_Eject();
        break;
        case 2: // catch valve piston / close valve
            DEBUG_puts("DEV_TESTMODE: piston catch\n\r");
            
            // re-initializing the VMC will close the valve
            // (only works if valve is currently NOT positioned at upper end stop)
            ValveMotionControl_Init();
        break;
    }

    
    *responseBytes = 1;    // set response length to 1 byte  (just echo command)    
}  






        
void cmd_dev_reset_callback(BYTE *buf_in, BYTE *buf_out, BYTE *responseBytes)
{           
    
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
        
        Delayms(150);
        
        Reset();
    }
    else
    {
        // safety codes do not match, return error code
        
        buf_out[1] = 0x01;  // safety code mismatch
        
    }
    
}            
            

void BTCom_SetupCallbacks()
{
    BTCom_addCallback(CMD_RESERVED,     BTCom_defaultCallback);
    BTCom_addCallback(CMD_MANVENT,      cmd_manvent_callback);   
    BTCom_addCallback(CMD_SETMODE,      cmd_setmode_callback);   
    BTCom_addCallback(CMD_STATUS,       cmd_status_callback);   
    BTCom_addCallback(CMD_READCONFIG,   cmd_readconfig_callback); 
    BTCom_addCallback(CMD_UPDATECONFIG, cmd_updateconfig_callback); 
    BTCom_addCallback(CMD_FWREV,        SendFWStringBT); 
    BTCom_addCallback(CMD_CLOCKSYNC,    cmd_clocksync_callback); 
    BTCom_addCallback(CMD_GETCLOCK,     cmd_getclock_callback); 
    BTCom_addCallback(CMD_DEV_PROGRAM,  cmd_dev_program_callback); 
    BTCom_addCallback(CMD_DEV_ECHO,     BTCom_defaultCallback); 
    BTCom_addCallback(CMD_DEV_RESET,    cmd_dev_reset_callback);  
    BTCom_addCallback(CMD_DEV_TESTMODE, cmd_dev_testmode_callback);  
}

