// task scheduler module
// (C) 2023-09-09 by Daniel Porzig

#ifndef _TASKSCHEDULER_H_
#define _TASKSCHEDULER_H_

#include <stdlib.h>
#include <stdio.h>
#include "HardwareProfile.h"
#include <GenericTypeDefs.h>

// number of simultaneous tasks
#define SCHEDULER_MAX_NUM_TASKS		8


typedef void (*VoidFnctpv)( void*, BYTE *);

typedef struct
{
	VoidFnctpv Task;
	void *pvParameters;
	BYTE mTaskCnt;
	BYTE mPriority;
}TaskStruct;	


void Scheduler_Init();
void Scheduler_AddTask(BYTE ID, VoidFnctpv Task, void *pvParameters, BYTE prio, BYTE startdelay);
void Scheduler_SetTaskSkiprate(BYTE ID, BYTE skiprate);
BYTE Scheduler_Run();

void Scheduler_BackupSettings(TaskStruct *bTasks);
void Scheduler_RestoreSettings(TaskStruct *bTasks);


void Scheduler_SetupTimer();

void Scheduler_DumpAnalysis();

#endif

