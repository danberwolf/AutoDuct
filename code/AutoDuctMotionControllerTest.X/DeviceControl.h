#ifndef _DEVICECONTROL_H_
#define _DEVICECONTROL_H_

#include <stdlib.h>
#include <stdio.h>
#include "HardwareProfile.h"
#include "FanControl.h"
#include "ValveMotionControl.h"
#include <GenericTypeDefs.h>

void DeviceControl_Init();
void DeviceControl_Task(void *pvParameters, BYTE *skiprate);
void DeviceControl_Testmode(BYTE mode);
void DeviceControl_ManualVent(BYTE fanmode, BYTE fandir, BYTE fanspeed, BYTE minutes);
void DeviceControl_StatusBTCom(BYTE *outbuf, BYTE *len);
void DeviceControl_SetMode(BYTE mode);
BYTE DeviceControl_GetMode();
void DeviceControl_AcknowledgeLED();
void DeviceControl_SmartVent(BYTE fanmode, BYTE fandir, BYTE fanspeed, BYTE minutes);

#define DEVICEMODE_SLAVE        0
#define DEVICEMODE_MANUAL       1
#define DEVICEMODE_SMART        2

#define DEVCTL_STATE_UNINITIALIZED              0
#define DEVCTL_STATE_INITIALIZED                1
#define DEVCTL_STATE_ZEROCROSS                  2
#define DEVCTL_STATE_TESTMODE1                  10


#define DEVICE_CONTROL_PWM_TIMEOUT_INIT         250     // 1 second
#define DEVICE_CONTROL_PWM_TIMEOUT_ZEROCROSS    2000     // 8 seconds for zero detection

#define FAN_STARTUP_DELAY                       600

#define SHT31_UNINITIALIZED                     0x00
#define SHT31_MEASURING                         0x01
#define SHT31_WAITING                           0x02

#define SHT31_MEASURE_DELAY                     5 // 20ms       1250    // 5 seconds
#define SHT31_MEASURE_INTERVAL                  30000   // 2 minutes
#define SHT31_TIMEOUT_CNT                       5


#endif
