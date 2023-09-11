// shutter motion control module
// (C) 2023-09-09 by Daniel Porzig

#ifndef _MOTORCONTROL_H_
#define _MOTORCONTROL_H_

#include <stdlib.h>
#include <stdio.h>
#include "HardwareProfile.h"
#include <GenericTypeDefs.h>


void MotorControl_Init();
void MotorControl_Ramp(BYTE dir, WORD delay, BYTE tspeed, WORD fspeed);
void MotorControl_SetSpeed(BYTE dir, BYTE speed);

BYTE MotorControl_getSpeed();
BYTE MotorControl_getTaskState();
BYTE MotorControl_getDir();


// low level motor control data
typedef struct MotorCTLSequence_Struct
{
	BYTE state;
	WORD counter;
    BYTE index;
    BYTE seq[1000];
    BYTE repcnt;
}MotorCTLSequenceData;	


typedef struct MotorCTL_Struct
{
	BYTE state;
    BYTE dir;
	BYTE rpm, targetrpm;
	WORD rampspeed;
	WORD delaycnt;
    MotorCTLSequenceData Sequence;
}MotorCTLData;	


// high level valve motion control data
typedef struct VMCTL_Struct
{
	BYTE state;				// state of valve motion control state machine
    BYTE faultcode;			// last fault code
	
	WORD maxcurr;			// maximum current measured during last run
	WORD timeout_cnt;		// timeout counter
	
    BYTE ignoreEndStopH;    // special test mode flag -> eject valve
	
	WORD s_timeout;			// timeout setting
	WORD s_rampmode;		// ramp mode setting (none - slow - fast)
	
	
	
	MotorCTLData MotorCTL;

}VMCTLData;	



// valve motion controller state machine states
#define VALVE_STATUS_UNDEFINED      0   // status after powerup when no endstop is detected
#define VALVE_STATUS_AT_ENDSTOP_L   1   // valve is fully closed and endstop signal detected
#define VALVE_STATUS_AT_ENDSTOP_H   2   // valve is fully open and endstop signal detected
#define VALVE_STATUS_OPENING        3   // valve is currently opening
#define VALVE_STATUS_CLOSING        4   // valve is currently closing

#define VALVE_STATUS_STOPPED        5   // valve was stopped at arbitrary position (no fault)

#define VALVE_STATUS_ERROR          10  // fault mode

#define VALVE_STATUS_SLEEP          20  // DRV8801 sleep mode



#define VALVECTL_RAMPMODE_NONE		0 // no motor speed ramping
#define VALVECTL_RAMPMODE_SLOW		1 // 
#define VALVECTL_RAMPMODE_MED		2 // 
#define VALVECTL_RAMPMODE_FAST		3 //

// ramp speeds
#define VCTL_RAMPSPEED_SLOW		6	
#define VCTL_RAMPSPEED_MED		3	
#define VCTL_RAMPSPEED_FAST		1	


// default timeout for valve actuation (20 seconds)
#define VCTL_TIMEOUT_DEFAULT	(20 * 250 * 1)	// 4ms scheduler period, skiprate = 1

// valve motion controller fault codes
#define VCTL_FAULT_NONE				0x00	

#define VCTL_FAULT_TIMEOUT_CLOSE	0x01	
#define VCTL_FAULT_TIMEOUT_OPEN 	0x02	
#define VCTL_FAULT_OVERTEMP			0x03
#define VCTL_FAULT_OVERCURR			0x04	





extern const BYTE seq_alternate2s[];


void MotorControl_StartSeq(BYTE* seq);
void MotorControl_StopSeq();
void MotorControl_Sequence_Task();
void MotorControl_SoftShutdown();



void ValveMotionControl_Init();
void ValveMotionControl_Task(void *pvParameters, BYTE *skiprate);
void Valve_FaultStop(BYTE faultcode);
void Valve_Stop();
void Valve_Close(BYTE speed, BYTE mode);
void Valve_Open(BYTE speed, BYTE mode);
void Valve_Eject();
void ValveMotionControl_setTimeout(WORD timeout);
void ValveMotionControl_setEndstopIgnore(BYTE mode);
void ValveMotionControl_setRampMode(BYTE rampmode);
BYTE ValveMotionControl_getState();
BYTE Valve_isClosed();
BYTE Valve_isOpened();

#endif 

