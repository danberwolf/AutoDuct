// Fan control state machine
// (C) 2023-09-09 by Daniel Porzig


#include "Fancontrol.h"

/*
 This module performs the following tasks:
 -Sampling of fan control PWM signal from Pluggit control unit
 -output of fan control PWM signal to connected fan
 
 
 */


// pulse duration tables for measured PWM signal from Pluggit control
// (inward flow direction)
const WORD fspeed_in_table[64] = {
638,646,655,663,672,680,688,697,705,714,722,731,739,747,756,764,773,781,789,
798,806,815,823,831,840,848,857,865,874,882,890,899,907,916,924,932,941,949,
958,966,975,983,991,1000,1008,1017,1025,1033,1042,1050,1059,1067,1075,1084,
1092,1101,1109,1118,1126,1134,1143,1151,1160,1168
};

// (outward flow direction)
const WORD fspeed_out_table[64] = {
638,630,623,615,607,599,592,584,576,569,561,553,545,538,530,522,515,507,499,
491,484,476,468,461,453,445,437,430,422,414,407,399,391,383,376,368,360,353,
345,337,329,322,314,306,299,291,283,275,268,260,252,245,237,229,221,214,206,
198,191,183,175,167,160,152
};



const WORD ctlsignal_pwm_table[9] = { 1215,   // out, level 4
                                      2058,   // out, level 3
                                      2858,   // out, level 2
                                      3420,   // out, level 1
                                      5100,   // level 0
                                      6943,   // in, level 1
                                      7503,   // in, level 2
                                      8343,   // in, level 3
                                      9345,   // in, level 4
};


const BYTE ctlsignal_pwm_table_output[5] = {
                                      0,   // level 0
                                      27,   // level 1
                                      36,   // level 2
                                      48,   // level 3
                                      63,   // level 4
};






FanCTLData FanCTL;
PWMsenseData PWMsense;

WORD samp[4];
char txt[250];



// +-------------------------------------------------------+
// | Low Level Sampling routines / ISR                     |
// +-------------------------------------------------------+

void SetupPWM_InputCapture()
{
	WORD j;

    PWMsense.sampcnt = 0;
    PWMsense.state = 0;     // idle
    
    
    // uses timer2 (shared with OC3 - setup in ledfade.c)
	
	// set up IC1 module for rpm capturing (uses TIMER2)
	IC1CON = 0;
	IC1CONbits.ICTMR = 0b001;	// selects timer 2
	IC1CONbits.ICI = 0b11;	// interrupt on every fourth capture event
//	IC1CONbits.ICI = 0b10;	// interrupt on every third capture event    
	IC1CONbits.ICM = 0b110;	// Simple Capture Event mode ? every edge, specified edge first and every edge thereafter
    IC1CONbits.FEDGE = 1; // start with rising edge


	// IC1 for rising edges
	IPC1bits.IC1IP = 1; // Setup Input Capture 1 interrupt for desired priority
						// level (this example assigns level 1 priority)
	IFS0bits.IC1IF = 0; // Clear the IC1 interrupt status flag
	IEC0bits.IC1IE = 1; // Enable IC1 interrupts	
	

	// clear capture buffer
	while(IC1CONbits.ICBNE)
	{
		j = IC1BUF;
	}	
		    


    IC1CONbits.ON = 1;
    PWMsense.state = 1;     // sampling
	
}

// interrupt service routine for input capture module
void __ISR(_INPUT_CAPTURE_1_VECTOR , ipl5) _IC1Interrupt(void)
{
    WORD i;
    
    // buffer should contain 4 samples:
    // -first rising edge
    // -first falling edge
    // -second rising edge
    // -second falling edge (discarded)
    
    for(i=0; i<4; i++)
    {
        samp[i] = IC1BUF; // Read and save off capture entry
    }    

    PWMsense.samp_period[PWMsense.sampcnt] = samp[2] - samp[0];
    PWMsense.samp_pulse[PWMsense.sampcnt] = samp[1] - samp[0];
    
    if(++PWMsense.sampcnt == (NUM_PWM_SAMPLES+1))
    {
        // sample buffer full. Stop sampling
        PWMsense.sampcnt = 0;
        PWMsense.state = 0;
        
        IC1CONbits.ON = 0;  // disable IC module
        
        IEC0bits.IC1IE = 0; // disable IC1 interrupts	
    }
    
    IFS0bits.IC1IF = 0; // Reset respective interrupt flag  
}




void FanControl_getPWMSignalRaw(WORD *pulse, WORD *period)
{
    *pulse = PWMsense.PWM_pulse;
    *period = PWMsense.PWM_period;
}


void FanControl_getFanLevel(BYTE *level, BYTE *dir)
{
    BYTE i, idx;
    signed int dif = 0, dif_last;
    
    dif_last = (signed int)ctlsignal_pwm_table[0] - (signed int)PWMsense.PWM_pulse;
    idx = 0;
    
    for(i=1; i<9; i++)
    {
        dif = (signed int)ctlsignal_pwm_table[i] - (signed int)PWMsense.PWM_pulse;
        if(abs(dif) < abs(dif_last))
        {
            idx = i;
            dif_last = dif;
        }
    }
    
    
    switch(idx)
    {
        case 0:
            *level = 4;
            *dir = 0;   // out
        break;
        case 1:
            *level = 3;
            *dir = 0;   // out
        break;
        case 2:
            *level = 2;
            *dir = 0;   // out
        break;
        case 3:
            *level = 1;
            *dir = 0;   // out
        break;
        case 4:
            *level = 0;
            *dir = 0;   // out
        break;
        case 5:
            *level = 1;
            *dir = 1;   // in
        break;
        case 6:
            *level = 2;
            *dir = 1;   // in
        break;
        case 7:
            *level = 3;
            *dir = 1;   // in
        break;
        case 8:
            *level = 4;
            *dir = 1;   // in
        break;
        default:
            *level = 0;
            *dir = 0;   // out
        break;        
    }
    
}

// get PWM step according to fan level
BYTE FanControl_Level2pwm(BYTE level)
{
    if(level > 4)
        return ctlsignal_pwm_table_output[4];
    else
        return ctlsignal_pwm_table_output[level];
}


// main fan control state machine task
void FanControl_Task(void *pvParameters, BYTE *skiprate)
{
    WORD i;
	BYTE updatePWM;
    // do PWM measurement tasks
    
    DWORD periodavg, pulseavg;
    signed int period_dev, pulse_dev, perd, puld;
    
    if(PWMsense.state == 0)
    {
        // sample buffer full, process samples
        // Note: first sample will be discarded
        
        periodavg = 0;
        pulseavg = 0;
        // calculate average values
        for(i=0; i<NUM_PWM_SAMPLES; i++)
        {
            periodavg += PWMsense.samp_period[i+1];
            pulseavg += PWMsense.samp_pulse[i+1];
        }
        
        periodavg /= NUM_PWM_SAMPLES;
        pulseavg /= NUM_PWM_SAMPLES;
        
        // calculate max. deviation
        period_dev = 0;
        pulse_dev = 0;
        for(i=0; i<NUM_PWM_SAMPLES; i++)
        {
            perd = PWMsense.samp_period[i+1] - periodavg;
            if( abs(perd) > abs(period_dev))
                period_dev = perd;

            puld = PWMsense.samp_pulse[i+1] - pulseavg;
            if( abs(puld) > abs(pulse_dev))
                pulse_dev = puld;
            
        }    
        
		// store new values in structure, discard values in case of high deviation
		
        if(abs(period_dev) > PWMSENSE_MAX_DEV || abs(pulse_dev) > PWMSENSE_MAX_DEV )
		{
//            sprintf(txt,"FANCTL: duty cycle measurement discarded (dev = %d).\n\r",( abs(period_dev) > abs(pulse_dev)) ? period_dev : pulse_dev);
//            DEBUG_puts(txt);            
		}
		else
		{
			// update PWM measurement values
			PWMsense.PWM_pulse = pulseavg;
			PWMsense.PWM_period = periodavg;
		}
		
		
        
//        sprintf(txt,"Period: %d, Pulse: %d, delta_period: %d, delta_pulse: %d \n\r",periodavg, pulseavg, period_dev,pulse_dev);
//        DEBUG_puts(txt);
//        
//        if(abs(period_dev) > 10 || abs(pulse_dev) > 10 )
//        {
//            for(i=0; i<NUM_PWM_SAMPLES; i++)
//            {
//                sprintf(txt,"   [%2d] Period: %d, Pulse: %d\n\r",i,PWMsense.samp_period[i+1],PWMsense.samp_pulse[i+1]);
//                DEBUG_puts(txt);
//                
//            }
//        }
        
        // restart measurement
        SetupPWM_InputCapture();
    }
    else
    {
//        üüüüüüüüüüüüüüüüüüüüüü
                
                
//                add timeout
    }
       
	   
	   
	// handle fan control PWM signal output   
	// --------------------------------------------------------------------
	// RPM ramp control engine
	// --------------------------------------------------------------------        
	updatePWM = 0;
	
	switch(FanCTL.state)
	{
		case 0: // continuos brigthness
			// do nothing
		break;
		case 1: // Waiting to fade up
			if(FanCTL.delaycnt==0)
			{
				FanCTL.state = 2;
			}
			else
				FanCTL.delaycnt--;
		break;			
		case 2:	// Fade Up
			if(++FanCTL.delaycnt>=FanCTL.fadespeed)
			{
				FanCTL.delaycnt = 0;	
				if(FanCTL.speed>=FanCTL.targetspeed)
					FanCTL.state = 0;	
				else						
					FanCTL.speed++;

				updatePWM = 1;                
			}		
		break;
		case 3: // Waiting to fade down
			if(FanCTL.delaycnt==0)
			{
				FanCTL.state = 4;
			}
			else
				FanCTL.delaycnt--;	
		break;		
		case 4:	// Fade Down
			if(++FanCTL.delaycnt>=FanCTL.fadespeed)
			{
				FanCTL.delaycnt = 0;
				if(FanCTL.speed<=FanCTL.targetspeed)
					FanCTL.state = 0;
				else				
					FanCTL.speed--;

				updatePWM = 1;
			}		
		break;
	}
	
	if(updatePWM == 1)
	{
		// write new duty cycle to PWM register	
		if(FanCTL.dir == FAN_DIR_INWARDS)
		{
			OC5RS = fspeed_in_table[FanCTL.speed] + FAN_PWM_OFFSET;
		}
		else
		{
			OC5RS = fspeed_out_table[FanCTL.speed] + FAN_PWM_OFFSET;
		}
	}
		   
	   
    *skiprate = 1;
}





void FanSpeedControl_Ramp(BYTE dir, WORD fdelay, BYTE level, WORD rampspeed)
{
    BYTE trpm;
    
    trpm = FanControl_Level2pwm(level);
    
    FanCTL.fanlevel = level;
    
	if(fdelay>0)
	{
		if(trpm > FanCTL.speed) // ramping up
			FanCTL.state = 1;
		else
			FanCTL.state = 3;		
	}
	else
	{
		if(trpm > FanCTL.speed) // ramping up
			FanCTL.state = 2;
		else
			FanCTL.state = 4;		
	}
	FanCTL.delaycnt = fdelay;
	FanCTL.targetspeed = trpm;
	FanCTL.fadespeed = rampspeed;
    
    if(dir == FAN_DIR_INWARDS)
        FanCTL.dir = FAN_DIR_INWARDS;
    else
        FanCTL.dir = FAN_DIR_OUTWARDS;    
    
    
            sprintf(txt,"FANCTL: trpm = %d.\n\r",trpm);
            DEBUG_puts(txt);       
    
}	

BYTE FanSpeedControl_getSpeed()
{
	return FanCTL.speed;
}

BYTE FanSpeedControl_getFanLevel()
{
	return FanCTL.fanlevel;
}


BYTE FanSpeedControl_getDir()
{
	return FanCTL.dir;
}


BYTE FanSpeedControl_getTaskState()
{
	return FanCTL.state;
}





// initialize fan control state machine and PWM / Input Capture hardware modules
void FanControl_Init()
{
    // setup IC module for PWM signal measurement
    SetupPWM_InputCapture();
    
    
    
    // setup OC module for PWM signal output   
    FanCTL.state = 0;
    FanCTL.delaycnt = 0;
    FanCTL.dir = 0;
    FanCTL.fadespeed = 0;
    FanCTL.speed = 0;
    FanCTL.targetspeed = 0;    
    FanCTL.fanlevel = 0;
    
    
  
    // Timer3 is shared with Motor Control (configured for 3.915kHz period, 10 bit)
    
    
 	// PWM init (fan speed control signal)
	OC5CON = 0;

	OC5CONbits.OCM = 6; 			// PWM mode, fault pin disabled
	OC5CONbits.OCTSEL = 0b001;		// select Timer3 as clock source

	OC5R = 0; //fspeed_out_table[0];     // fan disabled at startup
	OC5RS = fspeed_out_table[0] + FAN_PWM_OFFSET;    // PWM period

    FAN_PWM_OUT_ODC = 0;        // enable open drain config    
    
    
    OC5CONbits.ON = 1;              // enable output
    
    
    FAN_PWM_OUT_ODC = 0;        // enable open drain config    
    
        
}