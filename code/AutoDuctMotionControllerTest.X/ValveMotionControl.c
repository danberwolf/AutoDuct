// shutter motion control module
// (C) 2023-09-09 by Daniel Porzig

#include <proc/p32mx150f128b.h>
#include "ValveMotionControl.h"
#include "LedFade.h"


const BYTE seq_alternate2s[] = {CMD_SET, 40, 0, CMD_PAUSE, 17,CMD_SET, 40, 1, CMD_PAUSE, 17,CMD_REPEAT, 0};
WORD pwmtable_10_motor[64];

#define PWM_RES     	1276
#define PWMTABLE_MTR 	pwmtable_10_motor

VMCTLData VMCTL;


// interrupt for endstop HIGH sensor
void __ISR(_EXTERNAL_2_VECTOR , ipl5) _ExtInt2Interrupt(void)
{
    DEBUG_puts("Upper Endstop reached.\n\r");
    
    if(!VMCTL.ignoreEndStopH)
    {
        // Stop Motor, if currently opening valve
        if(MotorControl_getDir() == 0)
            MotorControl_SetSpeed(0, 0);

        // stop sequence playback
        MotorControl_StopSeq();


        // change valve controller state machine status
        VMCTL.state = VALVE_STATUS_AT_ENDSTOP_H;
    }
    
    IFS0bits.INT2IF = 0; // Reset respective interrupt flag  
}


// interrupt for endstop LOW sensor
void __ISR(_EXTERNAL_1_VECTOR , ipl4) _ExtInt1Interrupt(void)
{

    DEBUG_puts("Lower Endstop reached.\n\r");

    // Stop Motor, if currently closing valve
    if(MotorControl_getDir() == 1)
        MotorControl_SetSpeed(0, 0);

	// stop sequence playback
	MotorControl_StopSeq();


	// change valve controller state machine status
	VMCTL.state = VALVE_STATUS_AT_ENDSTOP_L;

    
    IFS0bits.INT1IF = 0; // Reset respective interrupt flag  
}







// Low level motor controller initialization
void MotorControl_Init()
{
    BYTE i;
    
    for(i=0; i<64; i++)
    pwmtable_10_motor[i] = PWM_RES/63.0 * i;
    
    // init motor driver data
	
	VMCTL.MotorCTL.state = 0;	
 

    // Timer is shared with fan PWM control signal generator
    
    // configure Timer 3 with 1:8 prescaler, gives 3.915 kHz PWM Frequency at 10 bit
    T3CONSET = 0x8000;          // Enable Timer3
    T3CONbits.TCKPS = 0b011;    // timer3 prescaler 1:8
    PR3 = PWM_RES;    
    
    
 	// PWM init (motor driver speed control)
	OC1CON = 0;

	OC1CONbits.OCM = 6; 			// PWM mode, fault pin disabled
	OC1CONbits.OCTSEL = 0b001;		// select Timer3 as clock source

	OC1R = 0x0000; 					// output disabled at startup
	OC1RS = 0x0000;                // PWM period

    OC1CONbits.ON = 1;              // enable output
    
    
	MOTOR_PWM_TRIS = 0;
	MOTOR_DIR_TRIS = 0;
    MOTOR_DIR = 0;    
    
    
    // setup interrupts for endstop detection
    
    INTCONbits.INT1EP = 0;  // setup interrupt for falling edge
    INTCONbits.INT2EP = 0;  // setup interrupt for falling edge
    
    
	IPC1bits.INT1IP = 1; // Setup interrupt for desired priority
	IPC2bits.INT2IP = 2; // Setup interrupt for desired priority
						
	IFS0bits.INT1IF = 0; // Clear the INT1 interrupt status flag
	IFS0bits.INT2IF = 0; // Clear the INT2 interrupt status flag

	IEC0bits.INT1IE = 1; // Enable INT1 interrupts	
	IEC0bits.INT2IE = 1; // Enable INT2 interrupts	
	    
}



// set motor low level speed and rotation direction
void MotorControl_SetSpeed(BYTE dir, BYTE speed)
{
   
    VMCTL.MotorCTL.state = 0;

    if(dir>0)
        VMCTL.MotorCTL.dir = 1;
    else
        VMCTL.MotorCTL.dir = 0;
        

    if(speed>63)
        VMCTL.MotorCTL.rpm = 63;
    else
        VMCTL.MotorCTL.rpm = speed;

    // write new rpmness to PWM register	
    OC1RS = PWMTABLE_MTR[VMCTL.MotorCTL.rpm];


    MOTOR_DIR = dir;
		

}	


// set a motor speed ramp control
void MotorControl_Ramp(BYTE dir, WORD fdelay, BYTE trpm, WORD rampspeed)
{
    
	if(fdelay>0)
	{
		if(trpm > VMCTL.MotorCTL.rpm) // ramping up
			VMCTL.MotorCTL.state = 1;
		else
			VMCTL.MotorCTL.state = 3;		
	}
	else
	{
		if(trpm > VMCTL.MotorCTL.rpm) // ramping up
			VMCTL.MotorCTL.state = 2;
		else
			VMCTL.MotorCTL.state = 4;		
	}
	VMCTL.MotorCTL.delaycnt = fdelay;
	VMCTL.MotorCTL.targetrpm = trpm;
	VMCTL.MotorCTL.rampspeed = rampspeed;
    
    if(dir>0)
        VMCTL.MotorCTL.dir = 1;
    else
        VMCTL.MotorCTL.dir = 0;    
    
}	

BYTE MotorControl_getSpeed()
{
	return VMCTL.MotorCTL.rpm;
}

BYTE MotorControl_getDir()
{
	return VMCTL.MotorCTL.dir;
}


BYTE MotorControl_getTaskState()
{
	return VMCTL.MotorCTL.state;
}
			


// +---------------------------------------------------------------------------+
// | PWM sequence playback engine                                              |
// +---------------------------------------------------------------------------+
void MotorControl_StartSeq(BYTE* seq)
{
    
    VMCTL.MotorCTL.Sequence.state = 1;
    VMCTL.MotorCTL.Sequence.index = 0;
    
    memcpy(VMCTL.MotorCTL.Sequence.seq,seq,100);

}

void MotorControl_StopSeq()
{
    
    VMCTL.MotorCTL.Sequence.state = 0;
    VMCTL.MotorCTL.Sequence.index = 0;
}

void MotorControl_SoftShutdown()
{
    BYTE dir;
    
    VMCTL.MotorCTL.Sequence.state = 0;
    
    dir = MotorControl_getDir();
    
    MotorControl_Ramp(dir,0,0,0);
}



// low level motor controller task (not to be called individually)
void MotorControl_Task()
{
    BYTE updatePWM,loop;
  

	// --------------------------------------------------------------------        
	// LED Sequence Player Engine
	// --------------------------------------------------------------------        
	
	loop = 1;
	switch(VMCTL.MotorCTL.Sequence.state)
	{
		case 0:
			// idle - do nothing
		break;
		case 1:
			// command execution

			// continue command execution only if ramp engine is in idle state
			if(VMCTL.MotorCTL.state == 0)
			{

				// execute commands until next pause command is issued
				while(loop)
				{
					switch(VMCTL.MotorCTL.Sequence.seq[VMCTL.MotorCTL.Sequence.index])
					{
						case CMD_SET:
							// arg1 = rpmness (0...64)
							// arg2 = dir
							MotorControl_SetSpeed(VMCTL.MotorCTL.Sequence.seq[VMCTL.MotorCTL.Sequence.index+2],VMCTL.MotorCTL.Sequence.seq[VMCTL.MotorCTL.Sequence.index+1]);
							VMCTL.MotorCTL.Sequence.index+=3;
						break;
						case CMD_RAMP:
							// arg1 = rpmness (0...31)
							// arg2 = speed (0...255)
							// arg3 = dir
							MotorControl_Ramp(VMCTL.MotorCTL.Sequence.seq[VMCTL.MotorCTL.Sequence.index+3],0, VMCTL.MotorCTL.Sequence.seq[VMCTL.MotorCTL.Sequence.index+1], VMCTL.MotorCTL.Sequence.seq[VMCTL.MotorCTL.Sequence.index+2]);
							VMCTL.MotorCTL.Sequence.index+=4;     
							loop = 0;   // do not continue with next command until fade operation complete
						break;
						case CMD_PAUSE:
							// arg1 = duration (0...255), duration = arg1 * 60ms (~15sec max.)
							VMCTL.MotorCTL.Sequence.state = 2;
							VMCTL.MotorCTL.Sequence.counter = VMCTL.MotorCTL.Sequence.seq[VMCTL.MotorCTL.Sequence.index+1]*5;
							VMCTL.MotorCTL.Sequence.index+=2;
							loop = 0;   // do not continue with next command 
						break;
						case CMD_STOP:
							// no arguments
							VMCTL.MotorCTL.Sequence.state = 0; // move to idle state
							VMCTL.MotorCTL.Sequence.index+=1;
							loop = 0;   // do not continue with next command until fade operation complete
						break;
						case CMD_REPEAT:
							// arg1 = number of repetitions (0...255, 0 = infinite)                       

							if(VMCTL.MotorCTL.Sequence.repcnt == 0)
							{
								// it is the first time the REPEAT-command is encountered. copy over repeat-value
								VMCTL.MotorCTL.Sequence.repcnt = VMCTL.MotorCTL.Sequence.seq[VMCTL.MotorCTL.Sequence.index+1];

								// set sequence index counter to sequence start
								VMCTL.MotorCTL.Sequence.index=0;
							}
							else
							{
								if(--VMCTL.MotorCTL.Sequence.repcnt == 0)
								{
									// sequence completed. go to idle state
									VMCTL.MotorCTL.Sequence.state = 0; // move to idle state
									loop = 0;   // do not continue with next command until fade operation complete
								}
								else
								{
									// set sequence index counter to sequence start
									VMCTL.MotorCTL.Sequence.index=0;
								}
							}
						break;

					}

				}
			}            

		break;
		case 2:
			// wait state (task cycle time 3*4ms)
			if(--VMCTL.MotorCTL.Sequence.counter == 0)
			{
				// continue with command execution
				VMCTL.MotorCTL.Sequence.state = 1; 
			}

		break;
	}



	// --------------------------------------------------------------------
	// RPM ramp control engine
	// --------------------------------------------------------------------        
	updatePWM = 0;
	
	switch(VMCTL.MotorCTL.state)
	{
		case 0: // continuos brigthness
			// do nothing
		break;
		case 1: // Waiting to fade up
			if(VMCTL.MotorCTL.delaycnt==0)
			{
				VMCTL.MotorCTL.state = 2;
			}
			else
				VMCTL.MotorCTL.delaycnt--;
		break;			
		case 2:	// Fade Up
			if(++VMCTL.MotorCTL.delaycnt>=VMCTL.MotorCTL.rampspeed)
			{
				VMCTL.MotorCTL.delaycnt = 0;	
				if(VMCTL.MotorCTL.rpm>=VMCTL.MotorCTL.targetrpm)
					VMCTL.MotorCTL.state = 0;	
				else						
					VMCTL.MotorCTL.rpm++;

				updatePWM = 1;                
			}		
		break;
		case 3: // Waiting to fade down
			if(VMCTL.MotorCTL.delaycnt==0)
			{
				VMCTL.MotorCTL.state = 4;
			}
			else
				VMCTL.MotorCTL.delaycnt--;	
		break;		
		case 4:	// Fade Down
			if(++VMCTL.MotorCTL.delaycnt>=VMCTL.MotorCTL.rampspeed)
			{
				VMCTL.MotorCTL.delaycnt = 0;
				if(VMCTL.MotorCTL.rpm<=VMCTL.MotorCTL.targetrpm)
					VMCTL.MotorCTL.state = 0;
				else				
					VMCTL.MotorCTL.rpm--;

				updatePWM = 1;
			}		
		break;
	}
	
	if(updatePWM == 1)
	{
		// write new duty cycle to PWM register	
		OC1RS = PWMTABLE_MTR[VMCTL.MotorCTL.rpm];
		
		if(VMCTL.MotorCTL.dir > 0)
			MOTOR_DIR = 1;
		else
			MOTOR_DIR = 0;                    
	}
}



// VALVE MOTION CONTROL FUNCTIONS


void ValveMotionControl_setTimeout(WORD timeout)
{
	VMCTL.s_timeout = timeout;
}

void ValveMotionControl_setRampMode(BYTE rampmode)
{
	VMCTL.s_rampmode = rampmode;
}

void ValveMotionControl_setEndstopIgnore(BYTE mode)
{
    VMCTL.ignoreEndStopH = mode;
}



	
void Valve_Actuate(BYTE dir, BYTE speed, BYTE mode)
{
	// check if motion controller is in error state
	if(VMCTL.state == VALVE_STATUS_ERROR)
		return;

	// stop any active sequence playback
	MotorControl_StopSeq();
	
	// start valve actuation
	
	switch(VMCTL.s_rampmode)
	{

		case VALVECTL_RAMPMODE_SLOW:
			// program motor speed ramp up
			MotorControl_Ramp(dir, 0, speed, VCTL_RAMPSPEED_SLOW);
		break;
		case VALVECTL_RAMPMODE_MED:
			// program motor speed ramp up
			MotorControl_Ramp(dir, 0, speed, VCTL_RAMPSPEED_MED);			
		break;
		case VALVECTL_RAMPMODE_FAST:
			// program motor speed ramp up
			MotorControl_Ramp(dir, 0, speed, VCTL_RAMPSPEED_FAST);			
		break;
		case VALVECTL_RAMPMODE_NONE:
		default:		
			// actuate valve without speed ramping
			MotorControl_SetSpeed(dir, speed);
		break;		
	}
	
	// set new state machine state
	if(dir == 0)
    {
		VMCTL.state = VALVE_STATUS_OPENING;
        DEBUG_puts("Valve opening...\n\r");
    }
	else
    {
		VMCTL.state = VALVE_STATUS_CLOSING;
        DEBUG_puts("Valve closing...\n\r");
    }


	// start timeout counter
	VMCTL.timeout_cnt = VMCTL.s_timeout;

}

void Valve_Open(BYTE speed, BYTE mode)
{	
	// safety check: already at upper endstop?
	if(ENDSTOP_HIGH == 0)
	{
		// valve currently at upper endstop (fully open)
		VMCTL.state = VALVE_STATUS_AT_ENDSTOP_H;
	}
	else
	{
		// initiate valve actuation
		Valve_Actuate(0, speed, mode);
	}
}


// development mode: ignore upper end stop / eject valve disc
void Valve_Eject()
{	
    // set endstop ignore flag
    ValveMotionControl_setEndstopIgnore(1);
    
    // initiate valve actuation
    Valve_Actuate(0, 30, 0);
}


void Valve_Close(BYTE speed, BYTE mode)
{
	// safety check: already at upper endstop?
	if(ENDSTOP_LOW == 0)
	{
		// valve currently at upper endstop (fully open)
		VMCTL.state = VALVE_STATUS_AT_ENDSTOP_L;
	}
	else
	{
		// initiate valve actuation
		Valve_Actuate(1, speed, mode);
	}	
}

// emergency stop valve actuation based on error condition
void Valve_FaultStop(BYTE faultcode)
{
	// stop motor
	MotorControl_SetSpeed(0, 0);

	// stop sequence playback
	MotorControl_StopSeq();
	
	// set error condition
	VMCTL.state = VALVE_STATUS_ERROR;
	VMCTL.faultcode = faultcode;
}


// stop valve actuation at arbitrary position (no fault condition)
void Valve_Stop()
{
	// stop motor
	MotorControl_SetSpeed(0, 0);
	
	// stop sequence playback
	MotorControl_StopSeq();
	
	// update state machine
	VMCTL.state = VALVE_STATUS_STOPPED;
}

BYTE Valve_isClosed()
{
    return (VMCTL.state == VALVE_STATUS_AT_ENDSTOP_L);
}

BYTE Valve_isOpened()
{
    return (VMCTL.state == VALVE_STATUS_AT_ENDSTOP_H);
}


void ValveMotionControl_Task(void *pvParameters, BYTE *skiprate)
{
    BYTE i;

	*skiprate = 1;		// default skiprate for all states	
	
    // --------------------------------------------------------------------        
    // Valve Control State Machine
    // --------------------------------------------------------------------        
    
    switch(VMCTL.state)
	{
		case VALVE_STATUS_UNDEFINED:
		
			// this is the default state after powerup. Check if the valve is currently at one of the endstops
			
			if(ENDSTOP_LOW == 0)
			{
				// valve currently at lower endstop (fully closed)
				VMCTL.state = VALVE_STATUS_AT_ENDSTOP_L;
			}
			else if(ENDSTOP_HIGH == 0)
			{
				// valve currently at upper endstop (fully open)
				VMCTL.state = VALVE_STATUS_AT_ENDSTOP_H;
			}
			else
			{
				// neither of the endstops detected. Assume that valve is currently positioned between the endstops
				// bring the valve into a defined state
				DEBUG_puts("VMCTL: valve position undefined. Initializing...\n\r"); 
                
				Valve_Close(30, 0);
			}
				

		
		break;
		
		case VALVE_STATUS_AT_ENDSTOP_H:
			// nothing to do
			
			// Optional:
			// check if valve is still in end stop position, otherwise try to move valve back to endstop
			if(ENDSTOP_HIGH == 1)
			{
				Valve_Open(30, 0);
			}

		break;
		
		case VALVE_STATUS_AT_ENDSTOP_L:
			// nothing to do
			
			// Optional:
			// check if valve is still in end stop position, otherwise try to move valve back to endstop
			if(ENDSTOP_LOW == 1)
			{
				Valve_Close(30, 0);
			}
			
		break;
		
		case VALVE_STATUS_CLOSING:
		
			if(VMCTL.timeout_cnt > 0)
			{
				VMCTL.timeout_cnt--;
				if(VMCTL.timeout_cnt == 0)
				{
                    DEBUG_puts("VMCTL: Timeout\n\r"); 

					// react on timeout
					Valve_FaultStop(VCTL_FAULT_TIMEOUT_CLOSE);
				}
			}
					
					
		break;
		
		case VALVE_STATUS_OPENING:

			if(VMCTL.timeout_cnt > 0)
			{
				VMCTL.timeout_cnt--;
				if(VMCTL.timeout_cnt == 0)
				{
                    DEBUG_puts("VMCTL: Timeout\n\r"); 

					// react on timeout
					Valve_FaultStop(VCTL_FAULT_TIMEOUT_OPEN);
                    
                    // reset ignore endstop flag in case it was set
                    VMCTL.ignoreEndStopH = 0;
				}
			}
		
		
		break;
		
		case VALVE_STATUS_STOPPED:
			// valve actuation stopped at arbitrary position by higher-level process
			// no failure state - do nothing
		break;
		
		case VALVE_STATUS_ERROR:
			// do nothing until failure is cleared by device controller
		break;
		
		
	}
	
	// execute low level motor control task
	MotorControl_Task();
	
}


BYTE ValveMotionControl_getState()
{
	return VMCTL.state;
}
	
	
	
	
void ValveMotionControl_Init()
{
	// low level motor controller initialization
	MotorControl_Init();

	// setup default values for valve motion control
	VMCTL.state = VALVE_STATUS_UNDEFINED;
	VMCTL.s_timeout = VCTL_TIMEOUT_DEFAULT;
	VMCTL.s_rampmode = VALVECTL_RAMPMODE_FAST;
	VMCTL.faultcode = VCTL_FAULT_NONE;
    VMCTL.ignoreEndStopH = 0;

}


