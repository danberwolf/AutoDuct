// configuration data handling module
// (C) 2023-09-09 by Daniel Porzig
#include "Config.h"
#include "BTCom.h"
#include "TimeKeeper.h"
#include "Bootloader.h"


cfg_base                 CFGbase;
cfg_vent_schedule   CFGventSched;

BYTE CFG_fragment_changed[CFG_NUM_FRAGMENTS] = {0};

const WORD CFG_fragment_size[CFG_NUM_FRAGMENTS] = {
41,
125,
256,
};

// addresses aligned at 128 Byte page boundaries
// (EEPROM chip only allows writing of entire pages)
const WORD CFG_fragment_address[CFG_NUM_FRAGMENTS] = {
0x0,
0x80,
0x100,
};


const BYTE* CFG_fragment_ptr[CFG_NUM_FRAGMENTS] = {
(BYTE*)&CFGbase,
(BYTE*)&CFGventSched,
NULL,
};

const char CFG_fragment_desc[CFG_NUM_FRAGMENTS][19] = {
"CFG_BASE          ",
"CFG_VENT_SCHED    ",
"CFG_RESERVED      ",
};



// set factory defaults for all configuration parameters 
void ConfigFactoryReset()
{
    BYTE i;
    
    memset(&CFGbase,0,sizeof(CFGbase));
    memset(&CFGventSched,0,sizeof(CFGventSched));    
    
    // Config Info
    CFGbase.signature = CONFIG_SIGNATURE;
    CFGbase.version = CONFIG_VERSION;
    CFGbase.daylightsavingAuto = 1;      // automatic daylight saving handling activated by default
    CFGbase.HumctlCfg = 0;
    CFGbase.HumctlFan = 3;
    CFGbase.HumctlHyst = 1; 
    CFGbase.HumctlLimit = 60;
    CFGbase.HumctlTimeout = 120;
    CFGbase.LEDcfg = 1;
    CFGbase.LEDmaxBright[0] = 63;   // day
    CFGbase.LEDmaxBright[1] = 63;   // night
    CFGbase.flowInvert = 0;
    
    CFGbase.HumctlRestrictHour[0] = 22;
    CFGbase.HumctlRestrictHour[1] = 7;
    CFGbase.HumctlRestrictMin[0] = 30;    
    CFGbase.HumctlRestrictMin[1] = 30;    
    
    strcpy(CFGbase.deviceID, "DuctUG");

    for(i=0; i<7; i++)
    {
        // Sunday = 0
        CFGventSched.hour[i][0] = 7;
        CFGventSched.min[i][0] = 0;
        CFGventSched.hour[i][1] = 23;
        CFGventSched.min[i][1] = 0;
        CFGventSched.flagsActive[i] = 0b00000001;
        CFGventSched.fanMode[i][0]  = 4;
        CFGventSched.duration[i][0] = 60;     // 60 minutes
    }
    
    SaveConfig();
    
}


// save the contents of the specified config fragment to EEPROM chip
void SaveConfig_Fragment(BYTE ID)
{
    if(ID >= CFG_NUM_FRAGMENTS)
        return;
    
    if(ID == CFG_ID_RESERVED)
        return;
    
    EEPROM_write(CFG_fragment_address[ID], CFG_fragment_ptr[ID], CFG_fragment_size[ID]);
    
}

// load the contents of the specified config fragment from EEPROM chip
void LoadConfig_Fragment(BYTE ID)
{
    if(ID >= CFG_NUM_FRAGMENTS)
        return;
    
    if(ID == CFG_ID_RESERVED)
        return;
    
    EEPROM_read(CFG_fragment_address[ID], CFG_fragment_ptr[ID], CFG_fragment_size[ID]);
    
}

// save entire config set
void SaveConfig()
{
    BYTE    i;
    for(i=0; i<CFG_NUM_FRAGMENTS; i++)
    {
        SaveConfig_Fragment(i);
    }
    
    DEBUG_puts("\n\rConfig Saved.");
}

// only save updated config set
void SaveConfig_UdatesOnly()
{
    BYTE    i, cnt = 0;
    for(i=0; i<CFG_NUM_FRAGMENTS; i++)
    {
        if(CFG_fragment_changed[i])
        {
            SaveConfig_Fragment(i);
            CFG_fragment_changed[i] = 0;
            cnt++;
        }
    }
    
    if(cnt)
        DEBUG_puts("\n\rConfig Updates Saved.");
}


// load entire config set
void LoadConfig()
{
    char txt[60];
    BYTE    i, ok = 1;
    
    
    do
    {
        ok = 1;
        DEBUG_puts("\n\rLoading Config...\n\r");

        DEBUG_puts("\n\rDescription        | Address | Size");
        DEBUG_puts("\n\r-------------------+---------+-----");        
        for(i=0; i<CFG_NUM_FRAGMENTS; i++)
        {
            LoadConfig_Fragment(i);

            sprintf(txt,"\n\r%s |  0x%04X | %4u",CFG_fragment_desc[i], CFG_fragment_address[i], CFG_fragment_size[i]);
            DEBUG_puts(txt);   

            // reset fragment change notification
            CFG_fragment_changed[i] = 0;
        }

        DEBUG_puts("\n\r-------------------+---------+-----");    

        sprintf(txt,"\n\r\n\rConfig Version: %02X",CFGbase.version);
        DEBUG_puts(txt);   

        // load factory defaults if unexpected config signature found
        if(CFGbase.signature != CONFIG_SIGNATURE)
        {
            DEBUG_puts("\n\rConfig signature mismatch!");
            ConfigFactoryReset();
            ok = 0;
        }

        // load factory defaults if version of config in EEPROM chip does not match
        if(CFGbase.version != CONFIG_VERSION)
        {
            DEBUG_puts("\n\rConfig version mismatch!");
            ConfigFactoryReset();
            ok = 0;
        }
        
    }while(!ok);

}



// load entire config set from EEPROM and dump contents to console
void DumpConfig()
{
    char txt[60];
    BYTE    i,j;
    BYTE *ptr;

    DEBUG_puts("\n\rDumping Config...");
    
    for(i=0; i<CFG_NUM_FRAGMENTS; i++)
    {
        
        // do not reload config from EEPROM here. RAM version will be outputted
        // (changes to config in RAM are only saved to EEPROM every 10 minutes)
        
        sprintf(txt,"\n\r\n\r[0x%02X] %s  (Address: 0x%04X, Size: %4u)\n\r\n\r",i,CFG_fragment_desc[i], CFG_fragment_address[i], CFG_fragment_size[i]);
        DEBUG_puts(txt);   

        if(i == CFG_ID_RESERVED)
            continue;
        
        ptr = CFG_fragment_ptr[i];
        
        for(j=0;j<CFG_fragment_size[i]; j++)
        {
            sprintf(txt,"%02X ",*ptr++);
            DEBUG_puts(txt);   
            
            if((((j+1)%16) == 0))
            {
                DEBUG_puts("\n\r");
            }
            
        }
        
    }

}


// copy data from buffer into selected Config Fragment
void Config_UpdateFragment(BYTE ID, BYTE *data)
{
    BYTE j;
    BYTE *ptr;
    
    if(ID >= CFG_NUM_FRAGMENTS)
        return;
    
    ptr = CFG_fragment_ptr[ID];
    for(j=0; j<CFG_fragment_size[ID]; j++)
    {
        *ptr++ = *data++;
    }
    
    CFG_fragment_changed[ID] = 1;    
    
}

// set modified flag for specified config fragment
void Config_NotifyChanged(BYTE ID)
{
    if(ID >= CFG_NUM_FRAGMENTS)
        return;
    
    CFG_fragment_changed[ID] = 1;
}

// copy specified config fragment into BLE protocol response buffer
void ConfigBlueToothOutput(BYTE ID, BYTE *outbuf, BYTE *len)
{
    BYTE *bptr = &outbuf[1];
    BYTE *ptr;
    BYTE j;

	// buffer[0] = CMD              // 0
    *bptr++ = ID;                   // 1
    
    if(ID == CFG_ID_RESERVED || ID >= CFG_NUM_FRAGMENTS)
    {
        *len = 2;
        return;
    }
    
    ptr = CFG_fragment_ptr[ID];
    for(j=0; j<CFG_fragment_size[ID]; j++)
    {
        *bptr++ = *ptr++;
    }    
    
    *len = CFG_fragment_size[ID] + 2;
    
}


// configuration structure handling for bootloader activation
cfg_bootcode_struct cfg_bootcode;
void SaveBootcodeConfig()
{
    EEPROM_write(BOOTCODE_CONFIG_EEPROM_ADD, &cfg_bootcode, sizeof(cfg_bootcode_struct));
}

void LoadBootcodeConfig()
{
    EEPROM_read(BOOTCODE_CONFIG_EEPROM_ADD, &cfg_bootcode, sizeof(cfg_bootcode_struct));
}

void Bootcode_setMagicNumber(DWORD magic)
{
    cfg_bootcode.magicnumber = magic;
}


// read bootloader firmware version string
BYTE getBootcodeVersionString(char *bcstring)
{
   BYTE i;
    char fwstr[20];
    char *storedFWstr;    
    
// firmware version string:
//  2 digits - Major FW version
//  2 chars  - TC: test version, PR: production version, bc: boot code
//  6 digits - date: YYMMDD
//  1 digit  - minor FW version
        
    BYTE *bptr = bcstring;    
    
    // send bootloader FW revision string
    
    // get global address of FW string in app program memory space
    storedFWstr = PA_TO_KVA0(BOOTCODE_FWSTRING_BASE_ADDRESS); 

    if(storedFWstr[2]=='b' && storedFWstr[3]=='c')
    {
        // valid boot code present
    
        // copy string
        for(i = 0; i<12; i++)
        {
            *bptr++ = storedFWstr[i];
        }
        *bptr++ = '\0';
        
        return 1;
    }
    else
    {
        // no valid bootcode found
        strcpy(bptr,"[invalid]");
        return 0;
    }
    
}

