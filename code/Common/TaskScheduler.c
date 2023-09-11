// task scheduler module
// (C) 2023-09-09 by Daniel Porzig

#include "TaskScheduler.h"
#include "HardwareProfile.h"
#include <stdio.h>
#include <stdlib.h>

TaskStruct Tasks[SCHEDULER_MAX_NUM_TASKS];	

// #define ENABLE_LOAD_ANALYSIS


#ifdef ENABLE_LOAD_ANALYSIS

	#define TRACE_DEPTH		1024		// 1024 Samples = ~4 Sec. Recording time

	BYTE LA_numTasks[TRACE_DEPTH];
	WORD LA_TimeLeft[TRACE_DEPTH];
    BYTE LA_Tasks[TRACE_DEPTH];
	WORD LA_idx = 0;
	
#endif

// variables for simple load analysis
    BYTE window = 0;
    BYTE underrun = 0;


void Scheduler_Init()
{
	BYTE i;
	
	// setup system timer as time reference for Task Scheduler
	Scheduler_SetupTimer();
	
	for(i=0; i< SCHEDULER_MAX_NUM_TASKS; i++)
	{
		Tasks[i].mPriority = 0;		// prio = 0 -> task will never be called
		Tasks[i].Task = NULL;
	}	
}	

void Scheduler_AddTask(BYTE ID, VoidFnctpv Task, void *pvParameters, BYTE prio, BYTE startdelay)
{
	Tasks[ID].Task = Task;
	Tasks[ID].pvParameters = pvParameters;
	Tasks[ID].mPriority = prio;
	Tasks[ID].mTaskCnt = startdelay+1;
}	

void Scheduler_SetTaskSkiprate(BYTE ID, BYTE skiprate)
{
	Tasks[ID].mTaskCnt = skiprate;
}	

BYTE Scheduler_Run()
{
	BYTE ct,nt,tdel = 1;
    BYTE txt[100];
	
	nt = 0;
	
	// START SYSTEM TIMER
	TMR4=0;					// Reset Counter
	IFS0bits.T4IF = 0;		// Reset Interrupt Flag	
	
	#ifdef ENABLE_LOAD_ANALYSIS	
    LA_Tasks[LA_idx] = 0;
    #endif

	
	for(ct=0; ct< SCHEDULER_MAX_NUM_TASKS; ct++)
	{
        
        
        
        
		if(Tasks[ct].mPriority == 2)
		{
            // high priority task. needs to be executed right now
			if(--Tasks[ct].mTaskCnt == 0)
			{

                #ifdef ENABLE_LOAD_ANALYSIS	
                LA_Tasks[LA_idx] |= (1<<ct);
                #endif


				// call task, pass Parameters and receive new task skiprate
				Tasks[ct].Task(Tasks[ct].pvParameters,&Tasks[ct].mTaskCnt);
				nt++;
			}
//			else
//			{
//				Tasks[ct].mTaskCnt--;
//			}	
		}
        else if(Tasks[ct].mPriority == 1)
		{
            // lower priority task that can be slightly delayed if no remaining cycle time available

			if(--Tasks[ct].mTaskCnt == 0)
			{
                if(IFS0bits.T4IF)
                {
                    // delay task by at least one cycle
                    Tasks[ct].mTaskCnt = tdel;
                    tdel++;
                }
                else
                {
                    
                    #ifdef ENABLE_LOAD_ANALYSIS	
                    LA_Tasks[LA_idx] |= (1<<ct);
                    #endif
                    
                    
                    // call task, pass Parameters and receive new task skiprate
                    Tasks[ct].Task(Tasks[ct].pvParameters,&Tasks[ct].mTaskCnt);
                    nt++;
                }
			}
            
        }
                
	}
	


//    // DEBUG output
//    if(IFS0bits.T3IF)
//    {
//        UART1_putc('=');
//    }		


////////    // 
////////    if(IFS0bits.T3IF)
////////    {
////////        underrun++;
////////    }		
////////    
////////    window++;
////////    if(window == 250)
////////    {
////////        // 1 sec passed, output underrun count
////////        sprintf(txt,"\n\ru=%5u",underrun);
////////        UART1_puts(txt);        
////////        underrun = 0;
////////        window = 0;
////////    }
    
    
	
	#ifdef ENABLE_LOAD_ANALYSIS	
	
		if(IFS0bits.T3IF)
		{
			LA_TimeLeft[LA_idx] = 0;		
		}		
		else
		{
			LA_TimeLeft[LA_idx] = TMR3;
		}	
			
		LA_numTasks[LA_idx] = nt;
		LA_idx++;
		
		if(LA_idx == TRACE_DEPTH)
		{
			// wrap around
			LA_idx = 0;
		}
	
	#endif		
			
	// Wait for Timer4 overflow	
	while(!IFS0bits.T4IF)
	{
		;
	}	
	
	return 0;
}				
				

void Scheduler_SetupTimer()
{
    
    // setup timer 4 for 4ms cycle time
	T4CON = 0x0000;

	T4CONbits.TCKPS = 0b011;        // 1:8 prescale
    PR4 = 19999;                     // 4 ms @ 40 MHz PBCLK
     
	// Clear counter
	TMR4 = 0;

    IPC4bits.T4IP = 0;	// disable calling of ISR
    IFS0bits.T4IF = 0;
    IEC0bits.T4IE = 1;

	// Start timer
	T4CONbits.TON = 1;        
    
}

void Scheduler_DumpAnalysis()
{
	#ifdef ENABLE_LOAD_ANALYSIS	
	
		WORD i;
		BYTE txt[100];
		
		for(i=0; i<TRACE_DEPTH; i++)
		{
			sprintf(txt,"\n\r%5u, %5u, %5u, %3u",i,LA_TimeLeft[i],LA_numTasks[i],LA_Tasks[i]);
			UART1_puts(txt);
		}	
    #else
        //UART1_puts("\n\r error: Load analysis disabled.");
	#endif		
}	







void Scheduler_BackupSettings(TaskStruct *bTasks)
{
	memcpy(bTasks,Tasks,SCHEDULER_MAX_NUM_TASKS*sizeof(TaskStruct));
}	

void Scheduler_RestoreSettings(TaskStruct *bTasks)
{
	memcpy(Tasks,bTasks,SCHEDULER_MAX_NUM_TASKS*sizeof(TaskStruct));
}	



// example Task:
/*
void TestTask(void *pvParameters, BYTE *skiprate)
{
	switch(BatteryADC_state)
	{
	case 0:	// Idle -> start sampling
		AD1CON1bits.SAMP = 1;
		BatteryADC_state = 1;
		*skiprate = 2;			// default skiprate
	break;
	case 1:	// check for finished conversion -> Wait State
		if(AD1CON1bits.DONE)
		{
			BatteryADC_value = ADC1BUF0 - BATTERY_ADC_OFFSET_UNITS;
			BatteryADC_state = 2;	// continue to wait state
		}
		*skiprate = 2;			// default skiprate		
	break;
	case 2:	// display Battery voltage
		// Signal Header Bar Update Request
		HeaderBar_setUpdateRequest(1);
		BatteryADC_state = 3;	
		*skiprate = 2;			// default skiprate
	break;
		
	case 3: // Wait State -> Idle
		BatteryADC_state = 0;	// continue to Idle state
		*skiprate = 250;		// delay skiprate
	break;
	}	
}
*/


