// Fan control state machine
// (C) 2023-09-09 by Daniel Porzig

#ifndef FANCONTROL_H
#define	FANCONTROL_H

#include <stdlib.h>
#include <stdio.h>
#include "HardwareProfile.h"
#include <GenericTypeDefs.h>



#define FAN_LEVEL_OUT4      0
#define FAN_LEVEL_OUT3      1
#define FAN_LEVEL_OUT2      2
#define FAN_LEVEL_OUT1      3
#define FAN_LEVEL_OFF       4
#define FAN_LEVEL_IN1       5
#define FAN_LEVEL_IN2       6
#define FAN_LEVEL_IN3       7
#define FAN_LEVEL_IN4       8

#define FAN_ALTERNATING     0x80



#define FAN_DIR_INWARDS		0
#define FAN_DIR_OUTWARDS	1


#define NUM_PWM_SAMPLES     16
#define PWMSENSE_MAX_DEV    10
#define FAN_PWM_OFFSET      10      // 30 does not seem to work



// data structure for fan control state machine
typedef struct FanCTL_Struct
{
	BYTE state;
    BYTE dir;
	BYTE speed, targetspeed;
    BYTE fanlevel;                  // just for display purposes in App
	WORD fadespeed;
	WORD delaycnt;
    // FanCTLSequenceData Sequence;
}FanCTLData;	

// data structure for PWM measurement state machine
typedef struct PWMsense_Struct
{
    volatile BYTE state;             // state machine state
    WORD samp_period[NUM_PWM_SAMPLES+1];   // sample buffer period measurement
    WORD samp_pulse[NUM_PWM_SAMPLES+1];    // sample buffer pulse width measurement
    BYTE sampcnt;           // sample index counter
    
    WORD PWM_pulse;
    WORD PWM_period;

    WORD PWM_pulse_last;
    WORD PWM_period_last;
    
}PWMsenseData;


void FanControl_Init();
void FanControl_Task(void *pvParameters, BYTE *skiprate);
void FanControl_getFanLevel(BYTE *level, BYTE *dir);
void FanSpeedControl_Ramp(BYTE dir, WORD fdelay, BYTE level, WORD rampspeed);
BYTE FanSpeedControl_getSpeed();
BYTE FanSpeedControl_getDir();
BYTE FanSpeedControl_getTaskState();
BYTE FanControl_Level2pwm(BYTE level);
BYTE FanSpeedControl_getFanLevel();
void FanControl_getPWMSignalRaw(WORD *pulse, WORD *period);

#endif	/* FANCONTROL_H */

