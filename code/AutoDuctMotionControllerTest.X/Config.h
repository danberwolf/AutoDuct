// configuration data handling module
// (C) 2023-09-09 by Daniel Porzig

#ifndef CONFIG_H_
#define CONFIG_H_

#include "HardwareProfile.h"
#include "GenericTypeDefs.h"
#include "Compiler.h"
#include "TimeKeeper.h"
#include "M24512.h"
#include <stdio.h>
#include <stdlib.h>


#define CONFIG_VERSION          0x07
#define CONFIG_SIGNATURE        0xF007

#define CFG_NUM_FRAGMENTS       3
// Configuration Fragment IDs

#define CFG_ID_BASE             0
#define CFG_ID_VENT_SCHED       1

#define CFG_ID_RESERVED         2


#pragma pack(push,1)
typedef struct cfg_base_TD
{
    WORD signature;         // signature for validity check
    BYTE version;           // config version
    
    BYTE deviceID[15];      // device name / ID
    BYTE LEDmaxBright[2];   // max. brightness of user feedback LEDs 
    BYTE LEDcfg;            // user feedback LED config
    BYTE flowInvert;        // flow inversion config
    BYTE HumctlCfg;         // humidity control config
    BYTE HumctlLimit;       // humidity control humidity limit
    BYTE HumctlHyst;        // humidity control hysteresis
    BYTE HumctlFan;         // humidity control max. fan speed
    BYTE HumctlTimeout;     // himidity control timeout (minutes)
    BYTE daylightsavingAuto;    // automatic daylight saving mode config
    BYTE HumctlRestrictHour[2]; // 
    BYTE HumctlRestrictMin[2];
    
    BYTE reserved[9];
}cfg_base;

typedef struct cfg_vent_schedule_TD
{
    BYTE flagsActive[7];
    BYTE fanMode[7][4];
    BYTE hour[7][4];
    BYTE min[7][4];
    BYTE duration[7][4];
    BYTE reserved[6];
}cfg_vent_schedule;

#pragma pack(pop)

extern cfg_base                 CFGbase;
extern cfg_vent_schedule        CFGventSched;

const WORD CFG_fragment_size[CFG_NUM_FRAGMENTS];
const WORD CFG_fragment_address[CFG_NUM_FRAGMENTS];




void ConfigFactoryReset();
void SaveConfig();
void SaveConfig_UdatesOnly();
void LoadConfig();
void ConfigBlueToothOutput(BYTE ID, BYTE *outbuf, BYTE *len);
void DumpConfig();

void Config_UpdateFragment(BYTE ID, BYTE *data);
void Config_NotifyChanged(BYTE ID);

void SaveBootcodeConfig();
void LoadBootcodeConfig();
void Bootcode_setMagicNumber(DWORD magic);
BYTE getBootcodeVersionString(char *bcstring);


#endif