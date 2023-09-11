// AutoDuct bootloader BLE protocol callbacks
// (C) 2023-09-09 by Daniel Porzig

#include "BTComCallbacksBootloader.h"
#include "BootLoader.h"

// command table
#define CMD_RESERVED        0x00    
#define CMD_MANVENT         0x01        // manually activate/disable venting, set duration   
#define CMD_SETRGB          0x02        // --
#define CMD_STATUS          0x03        // read power mode, display brightness, ALS reading, color
#define CMD_READCONFIG      0x04        // new for Wordclock
#define CMD_UPDATECONFIG	0x05        // new for Wordclock
#define CMD_FWREV           0x06
#define CMD_POWERCFG        0x07        // set power mode

#define CMD_DEV_PROGRAM             0xF0
#define CMD_DEV_RESET               0xF1
#define CMD_DEV_ECHO                0xFC



// callbacks implemented in AutoDuctBootloader.c
void Bootloader_BTcomCallback_Status(BYTE *buf_in, BYTE *buf_out, BYTE *responseBytes);
void Bootloader_BTcomCallback(BYTE *buf_in, BYTE *buf_out, BYTE *responseBytes);
void cmd_dev_reset_callback(BYTE *buf_in, BYTE *buf_out, BYTE *responseBytes);


void BTCom_defaultCallback(BYTE *buf_in, BYTE *buf_out, BYTE* len)
{
    //responseBytes = *len-1;  // echo back entire data packet (last data byte is checksum)        
    *len = *len-1;
}



extern const char __attribute__((space(prog),address(BOOTCODE_FWSTRING_BASE_ADDRESS)))  FW_VERS_STRING[];



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


    //DeviceControl_ManualVent(buf_in[1], buf_in[2], buf_in[3], buf_in[4]);


    *responseBytes = 1;    // set response length to 1 byte  (just echo command)
}

        
void cmd_readconfig_callback(BYTE *buf_in, BYTE *buf_out, BYTE *responseBytes)
{
            
    // buf_in[0] = CMD
    // buf_in[1] = ID

    //ConfigBlueToothOutput(buf_in[1], buf_out, responseBytes);
    *responseBytes = 1;    // set response length to 1 byte  (just echo command)    
}


void cmd_updateconfig_callback(BYTE *buf_in, BYTE *buf_out, BYTE *responseBytes)
{
    //Config_UpdateFragment(buf_in[1], &buf_in[2]);

    *responseBytes = 1;    // set response length to 1 byte  (just echo command)
}    


void cmd_status_callback(BYTE *buf_in, BYTE *buf_out, BYTE *responseBytes)
{
    //DeviceControl_StatusBTCom(buf_out, responseBytes);   
    Bootloader_BTcomCallback_Status(buf_in, buf_out, responseBytes);    
 
}


void cmd_clocksync_callback(BYTE *buf_in, BYTE *buf_out, BYTE *responseBytes)
{
//                cmddata[1] = (byte)rnow.get(Calendar.HOUR_OF_DAY);
//                cmddata[2] = (byte)rnow.get(Calendar.MINUTE);
//                cmddata[3] = (byte)rnow.get(Calendar.SECOND);
//
//                cmddata[4] = (byte)rnow.get(Calendar.DAY_OF_WEEK);
//                cmddata[5] = (byte)rnow.get(Calendar.DAY_OF_MONTH);
//                cmddata[6] = (byte)rnow.get(Calendar.MONTH);      // january = 0
//                cmddata[7] = (byte)rnow.get(Calendar.YEAR);            

    // Android time starts with Sunday = 1 (same as Timekeeper), January = 0
 //   TimeKeeper_setTimeDate(buf_in[4], buf_in[5], buf_in[6] + 1, buf_in[7], buf_in[1], buf_in[2], buf_in[3], 1);
    
    *responseBytes = 1;    // set response length to 1 byte  (just echo command)    
}

            
void cmd_dev_program_callback(BYTE *buf_in, BYTE *buf_out, BYTE *responseBytes)
{           
    Bootloader_BTcomCallback(buf_in, buf_out, responseBytes);
}            


            

void BTCom_SetupCallbacks()
{
    BTCom_addCallback(CMD_RESERVED,     BTCom_defaultCallback);
    BTCom_addCallback(CMD_MANVENT,      cmd_manvent_callback);   
    BTCom_addCallback(CMD_STATUS,       cmd_status_callback);   
    BTCom_addCallback(CMD_READCONFIG,   cmd_readconfig_callback); 
    BTCom_addCallback(CMD_UPDATECONFIG, cmd_updateconfig_callback); 
    BTCom_addCallback(CMD_FWREV,        SendFWStringBT); 
    BTCom_addCallback(CMD_CLOCKSYNC,    cmd_clocksync_callback); 
    BTCom_addCallback(CMD_DEV_PROGRAM,  cmd_dev_program_callback); 
    BTCom_addCallback(CMD_DEV_ECHO,     BTCom_defaultCallback); 
    BTCom_addCallback(CMD_DEV_RESET,    cmd_dev_reset_callback);     
}

