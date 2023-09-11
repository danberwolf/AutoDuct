/* 
 * File:   TimeKeeper.h
 * Author: purzel
 *
 * Created on 10. Juli 2012, 22:58
 */

#ifndef TIMEKEEPER_H
#define	TIMEKEEPER_H



#include <stdlib.h>
#include <stdio.h>
#include "HardwareProfile.h"
#include "GenericTypeDefs.h"
#include "TaskScheduler.h"

#define bcd2dec(bcd)	(((((bcd)>>4) & 0x0F) * 10) + ((bcd) & 0x0F)) 
#define dec2bcd(dec)	((((dec)/10)<<4)|((dec)%10)) 


typedef struct
{
    BYTE      hour;
    BYTE      min;
    BYTE      sec;
}TimeStruct;

typedef struct
{
    BYTE    weekday;
    BYTE    day;
    BYTE    month;
    WORD    year;
}DateStruct;


typedef void (*VoidFnct)( void );

typedef struct TimeoutEvent_Struct
{
	BYTE Active;
	BYTE TimedOut;
	//TimeStruct TimeoutTime;
	BYTE MinutesLeft;
    VoidFnct Task;
}TimeoutEvent;	




#define TIMEOUT_ID_MANUALVENT   0
#define TIMEOUT_ID_AUTOVENT     1
#define TIMEOUT_ID_FANALT       2


// timekeeper and event scheduling module
// (C) 2023-09-09 by Daniel Porzig

void TimeKeeper_Init();
void TimeKeeper_Task(void *pvParameters, BYTE *skiprate);
BYTE TimeKeeper_needsTimeUpdate();
BYTE TimeKeeper_UpdateTime();
void TimeKeeper_ForceScheduleUpdates();
void TimeKeeper_setTimeDate(BYTE weekday, BYTE day, BYTE month, BYTE year,BYTE hour, BYTE min, BYTE sec, BYTE hourmode, BYTE calibrateRTCC);
void TimeKeeper_getTimeDate(BYTE* weekday, BYTE* day, BYTE* month, BYTE* year,BYTE* hour, BYTE* min, BYTE* sec);

void TimeKeeper_SetupTimeout(BYTE tnum, BYTE minutes, VoidFnct Task);
BYTE TimeKeeper_TimeoutOver(BYTE tnum);
void TimeKeeper_GetTimeoutStats(BYTE tnum, BYTE *state, BYTE *minutes);

void TimeKeeper_ResetTimeouts();
void TimeKeeper_ResetTimeout(BYTE tnum);


BYTE TimeKeeper_getMinuteOffset();
void TimeKeeper_getTime(BYTE *h, BYTE *m, BYTE *s);
BYTE GetPassedMinutesCurrent();

#define CDSM_NOCHANGE				0
#define CDSM_DISABLED_NOW			3
#define CDSM_ENABLED_NOW			4
#define CDSM_ERROR_DISABLED_NOW		0xF3
#define CDSM_ERROR_ENABLED_NOW		0xF4

BYTE AdjustDaylightSavingTime();
BYTE CheckDaylightSavingMode(BYTE BBoverride);
BYTE GetTimeTillDaylightSwitch(WORD *timerem);



#endif	/* TIMEKEEPER_H */

