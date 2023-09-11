// LED fading engine and sequence player
// (C) 2023-09-09 by Daniel Porzig

#include "LEDFade.h"

// 10-bit brightness table
const WORD pwmtable_10[64] = {
0x0000,0x0001,0x0001,0x0002,0x0002,0x0002,0x0002,0x0002,0x0003,0x0003,0x0003,0x0004,0x0004,0x0005,0x0005,0x0006,
0x0006,0x0007,0x0008,0x0009,0x000A,0x000B,0x000C,0x000D,0x000F,0x0011,0x0013,0x0015,0x0017,0x001A,0x001D,0x0020,
0x0024,0x0028,0x002C,0x0031,0x0037,0x003D,0x0044,0x004C,0x0055,0x005E,0x0069,0x0075,0x0083,0x0092,0x00A2,0x00B5,
0x00CA,0x00E1,0x00FA,0x0117,0x0137,0x015A,0x0182,0x01AE,0x01DF,0x0216,0x0253,0x0297,0x02E3,0x0338,0x0396,0x03FF,
};

// 16-bit brightness table
const WORD pwmtable_16[64] = {
0,1,2,2,2,3,3,4,5,6,7,8,10,11,13,16,19,23,27,32,38,45,54,64,76,91,108,128,152,
181,215,256,304,362,431,512,609,724,861,1024,1218,1448,1722,2048,2435,2896,3444,
4096,4871,5793,6889,8192,9742,11585,13777,16384,19484,23170,27554,32768,38967,
46340,55108,65535,
};


// LED flashing / fading sequences
const BYTE seq_flash_low[] = {CMD_SET, 30, CMD_PAUSE, 2, CMD_SET, 0, CMD_PAUSE, 50, CMD_REPEAT, 0};     // single flash every couple of seconds, low brightness
const BYTE seq_2xflash_low[] = {CMD_SET, 30, CMD_PAUSE, 2, CMD_SET, 0, CMD_PAUSE, 2, CMD_SET, 30, CMD_PAUSE, 2, CMD_SET, 0, CMD_PAUSE, 50, CMD_REPEAT, 0};     // double flash every couple of seconds, low brightness
const BYTE seq_breathing[] = {CMD_RAMP, 63, 0, CMD_RAMP, 0, 0, CMD_PAUSE, 40, CMD_REPEAT, 0};
const BYTE seq_breathing_fast[] = {CMD_RAMP, 63, 0, CMD_RAMP, 0, 0, CMD_PAUSE, 5, CMD_REPEAT, 0};
const BYTE seq_breathing_low[] = {CMD_RAMP, 40, 2, CMD_RAMP, 0, 2, CMD_PAUSE, 40, CMD_REPEAT, 0};
const BYTE seq_sawtooth[] = {CMD_RAMP, 31, 0, CMD_RAMP, 15, 4, CMD_REPEAT, 0};
const BYTE seq_toggle_1Hz[] = {CMD_SET, 31, CMD_PAUSE, 8, CMD_SET, 0, CMD_PAUSE, 8, CMD_REPEAT, 0};
const BYTE seq_toggle_4Hz[] = {CMD_SET, 63, CMD_PAUSE, 2, CMD_SET, 0, CMD_PAUSE, 2, CMD_REPEAT, 0};
//const BYTE seq_toggle_4Hzfadeout[] = {CMD_SET, 31, CMD_PAUSE, 2, CMD_SET, 0, CMD_PAUSE, 2, CMD_SET, 31, CMD_PAUSE, 2, CMD_SET, 0, CMD_PAUSE, 2,CMD_SET, 31, CMD_PAUSE, 2, CMD_SET, 0, CMD_PAUSE, 2,CMD_SET, 31, CMD_PAUSE, 2, CMD_RAMP, 0, 0, CMD_STOP};
const BYTE seq_toggle_4Hzfadeout[] = {CMD_SET, 63, CMD_PAUSE, 2, CMD_SET, 0, CMD_PAUSE, 2, CMD_SET, 63, CMD_PAUSE, 2, CMD_SET, 0, CMD_PAUSE, 2,CMD_SET, 63, CMD_PAUSE, 2, CMD_SET, 0, CMD_PAUSE, 2,CMD_SET, 63, CMD_PAUSE, 2, CMD_RAMP, 0, 0, CMD_STOP};
//const BYTE seq_toggle_4Hzfade2half[] = {CMD_SET, 31, CMD_PAUSE, 2, CMD_SET, 0, CMD_PAUSE, 2, CMD_SET, 31, CMD_PAUSE, 2, CMD_SET, 0, CMD_PAUSE, 2,CMD_SET, 31, CMD_PAUSE, 2, CMD_SET, 0, CMD_PAUSE, 2,CMD_SET, 31, CMD_PAUSE, 2, CMD_RAMP, 14, 2, CMD_STOP};
const BYTE seq_toggle_4Hzfade2half[] = {CMD_SET, 63, CMD_PAUSE, 2, CMD_SET, 0, CMD_PAUSE, 2, CMD_SET, 63, CMD_PAUSE, 2, CMD_SET, 0, CMD_PAUSE, 2,CMD_SET, 63, CMD_PAUSE, 2, CMD_SET, 0, CMD_PAUSE, 2,CMD_SET, 63, CMD_PAUSE, 2, CMD_RAMP, 31, 0, CMD_STOP};
const BYTE seq_fade2half[] = {CMD_RAMP, 14, 1, CMD_STOP};
const BYTE seq_setfull[] = {CMD_SET, 63, CMD_STOP};

const BYTE seq_sine30x[] = {CMD_RAMP, 63, 0, CMD_RAMP, 0, 0, CMD_REPEAT, 30};
const BYTE seq_flash30x3[] = {CMD_SET, 63, CMD_PAUSE, 1, CMD_SET, 0, CMD_PAUSE, 1, CMD_SET, 63, CMD_PAUSE, 1, CMD_SET, 0, CMD_PAUSE, 1, CMD_SET, 63, CMD_PAUSE, 1, CMD_RAMP, 0, 0, CMD_PAUSE, 50, CMD_REPEAT, 30};

const BYTE seq_toggle_4Hz2x[] = {CMD_SET, 45, CMD_PAUSE, 2, CMD_SET, 0, CMD_PAUSE, 2, CMD_REPEAT, 1};
const BYTE seq_breathing_dyn[] = {CMD_RAMP, 0xFF, 4, CMD_RAMP, 0, 4, CMD_PAUSE, 40, CMD_REPEAT, 0};
const BYTE seq_const_dyn[] = {CMD_RAMP, 0xFF, 1, CMD_STOP};



//// setup for 10bit PWM
//#define PWM_RES     1023
//#define PWMTABLE_LED pwmtable_10

// setup for 16bit PWM
#define PWM_RES     65535
#define PWMTABLE_LED pwmtable_16

// setup number of hardware LED channels
#define NUM_LEDS    2

LEDFadeData ledFade[NUM_LEDS];

// initialize fading and sequencing state machines
void LEDFade_Init()
{
    // orange system led
    ledFade[0].bright = 0;
    ledFade[0].state = 0;
    ledFade[0].Sequence.state = 0;
    ledFade[0].globalbright = 63;
    ledFade[0].dynmax = 63;

    // white status led
    ledFade[1].bright = 0;
    ledFade[1].state = 0;
    ledFade[1].Sequence.state = 0;
    ledFade[1].globalbright = 63;
    ledFade[1].dynmax = 63;

    
    
// PWM-Module INFO
// OCxR sets duty cycle
// OCxRS sets period

    // Configure standard PWM mode for output compare modules
    OC3CON = 0x0006; 
 
    // A write to OCxRS configures the duty cycle
    // : OCxRS / PRy = duty cycle
    OC3RS = PWM_RES;  
    
    // configure Timer 2 with 1:1 prescaler, gives 610 Hz PWM Frequency at 16 bit
    T2CONSET = 0x8000;          // Enable Timer2
    T2CONbits.TCKPS = 0b000;    // timer2 prescaler 1:1
    PR2 = PWM_RES;
    //PR2 = 0;
    
    
    OC3CONSET = 0x8000;     // Enable Output Compare Module 5    


    // Configure standard PWM mode for output compare modules
    OC2CON = 0x0006; 
 
    // A write to OCxRS configures the duty cycle
    // : OCxRS / PRy = duty cycle
    OC2RS = PWM_RES;  
    OC2CONSET = 0x8000;     // Enable Output Compare Module 5    
              
}

void LEDFade_SetGlobalBrightness(BYTE led, BYTE gbr)
{
    ledFade[led].globalbright = gbr;
}

void LEDFade_SetSequenceDynamicMax(BYTE led, BYTE dynmax)
{
    ledFade[led].dynmax = dynmax;
}

void LEDFade_SetBrightness(BYTE led, BYTE br)
{
    WORD brcalc;
    
    ledFade[led].state = 0;
    
    if(br == 0xFF)
    {
        br = ledFade[led].dynmax;
    }
    
    if(br>63)
        ledFade[led].bright = 63;
    else
        ledFade[led].bright = br;

    brcalc = (ledFade[led].bright * ledFade[led].globalbright) / 63;
    
    
    switch(led)
    {
        case 0:
            // write new brightness to PWM register	
            OC3RS = PWM_RES-PWMTABLE_LED[brcalc];		// red sys led	
        break;
        case 1:
            // write new brightness to PWM register	
            OC2RS = PWM_RES-PWMTABLE_LED[brcalc];		// red sys led	
        break;
    }
		

}	



void LEDFade_Fade(BYTE led, WORD fdelay, BYTE tbright, WORD fspeed)
{
    
    if(tbright == 0xFF)
    {
        tbright = ledFade[led].dynmax;
    }    
    
	if(fdelay>0)
	{
		if(tbright > ledFade[led].bright) // fading up
			ledFade[led].state = 1;
		else
			ledFade[led].state = 3;		
	}
	else
	{
		if(tbright > ledFade[led].bright) // fading up
			ledFade[led].state = 2;
		else
			ledFade[led].state = 4;		
	}
	ledFade[led].FadeDelay = fdelay;
	ledFade[led].targetbright = tbright;
	ledFade[led].fadespeed = fspeed;
}	

BYTE LEDFade_getBrightness(BYTE led)
{
	return ledFade[led].bright;
}

BYTE LEDFade_getTaskState(BYTE led)
{
	return ledFade[led].state;
}
			



// +---------------------------------------------------------------------------+
// | LED sequence playback engine                                              |
// +---------------------------------------------------------------------------+
void LEDFade_StartSeq(BYTE led, const BYTE* seq, const BYTE* seqnext)
{
    ledFade[led].Sequence.state = 1;
    ledFade[led].Sequence.index = 0;
    ledFade[led].Sequence.seq = seq;
    ledFade[led].Sequence.seqnext = seqnext;

}


void LEDFade_SoftShutdown(BYTE led)
{
    // fade down BTLed in case it was active, deactivate sequence player
    ledFade[led].Sequence.state = 0;
    LEDFade_Fade(led,0,0,0);
}

void LEDFade_Disable(BYTE led)
{
    // disable LED fade module and shutdown LED
    ledFade[led].Sequence.state = 0;    
    
    if(led == 0)
    {
        OC3CON = 0x0000; 
        T2CONSET = 0x0000;          // Disable Timer2

        SYSLED = 1; // set to high to disable LED
    }
    else
    {
        OC2CON = 0x0000; 
        T2CONSET = 0x0000;          // Disable Timer2
               
    }
    
}




void LEDFade_Task(void *pvParameters, BYTE *skiprate)
{
    BYTE led,updatePWM,loop;
    WORD brcalc;

	*skiprate = 3;		// default skiprate for all states	
	
 
    for(led=0; led<NUM_LEDS; led++)
    {
        // --------------------------------------------------------------------        
        // LED Sequence Player Engine
        // --------------------------------------------------------------------        
        
        loop = 1;
        switch(ledFade[led].Sequence.state)
        {
            case 0:
                // idle - do nothing
            break;
            case 1:
                // command execution

                // continue command execution only if led fade is in idle state
                if(ledFade[led].state == 0)
                {

                    // execute commands until next pause command is issued
                    while(loop)
                    {
                        switch(ledFade[led].Sequence.seq[ledFade[led].Sequence.index])
                        {
                            case CMD_SET:
                                // arg1 = brightness (0...31)
                                LEDFade_SetBrightness(led,ledFade[led].Sequence.seq[ledFade[led].Sequence.index+1]);
                                ledFade[led].Sequence.index+=2;
                            break;
                            case CMD_RAMP:
                                // arg1 = brightness (0...31)
                                // arg2 = speed (0...255)
                                LEDFade_Fade(led,0, ledFade[led].Sequence.seq[ledFade[led].Sequence.index+1], ledFade[led].Sequence.seq[ledFade[led].Sequence.index+2]);
                                ledFade[led].Sequence.index+=3;     
                                loop = 0;   // do not continue with next command until fade operation complete
                            break;
                            case CMD_PAUSE:
                                // arg1 = duration (0...255), duration = arg1 * 60ms (~15sec max.)
                                ledFade[led].Sequence.state = 2;
                                ledFade[led].Sequence.counter = ledFade[led].Sequence.seq[ledFade[led].Sequence.index+1]*5;
                                ledFade[led].Sequence.index+=2;
                                loop = 0;   // do not continue with next command 
                            break;
                            case CMD_STOP:
                                // no arguments
                                ledFade[led].Sequence.state = 0; // move to idle state
                                ledFade[led].Sequence.index+=1;
                                loop = 0;   // do not continue with next command until fade operation complete
                                
                                // check if a second sequence needs to be activated
                                if(ledFade[led].Sequence.seqnext != NULL)
                                {
//                                    DEBUG_puts("followup sequence. \n\r");
                                    ledFade[led].Sequence.seq = ledFade[led].Sequence.seqnext;
                                    ledFade[led].Sequence.state = 1;
                                    ledFade[led].Sequence.index = 0;                                    
                                    ledFade[led].Sequence.seqnext = NULL;
                                }
                                
                            break;
                            case CMD_REPEAT:
                                // arg1 = number of repetitions (0...255, 0 = infinite)                       

                                if(ledFade[led].Sequence.repcnt == 0)
                                {
                                    // it is the first time the REPEAT-command is encountered. copy over repeat-value
                                    ledFade[led].Sequence.repcnt = ledFade[led].Sequence.seq[ledFade[led].Sequence.index+1];

                                    // set sequence index counter to sequence start
                                    ledFade[led].Sequence.index=0;
                                }
                                else
                                {
                                    if(--ledFade[led].Sequence.repcnt == 0)
                                    {
                                        // sequence completed. go to idle state
                                        ledFade[led].Sequence.state = 0; // move to idle state
                                        loop = 0;   // do not continue with next command until fade operation complete
                                        
                                        // check if a second sequence needs to be activated
                                        if(ledFade[led].Sequence.seqnext != NULL)
                                        {
                                            ledFade[led].Sequence.seq = ledFade[led].Sequence.seqnext;
                                            ledFade[led].Sequence.state = 1;
                                            ledFade[led].Sequence.index = 0;                                    
                                            ledFade[led].Sequence.seqnext = NULL;
                                        }                                        
                                        
                                    }
                                    else
                                    {
                                        // set sequence index counter to sequence start
                                        ledFade[led].Sequence.index=0;
                                    }
                                }
                            break;

                        }

                    }
                }            

            break;
            case 2:
                // wait state (task cycle time 3*4ms)
                if(--ledFade[led].Sequence.counter == 0)
                {
                    // continue with command execution
                    ledFade[led].Sequence.state = 1; 
                }

            break;
        }



        // --------------------------------------------------------------------
        // LED Fading engine
        // --------------------------------------------------------------------        
        updatePWM = 0;
        
        switch(ledFade[led].state)
        {
            case 0: // continuos brigthness
                // do nothing
            break;
            case 1: // Waiting to fade up
                if(ledFade[led].FadeDelay==0)
                {
                    ledFade[led].state = 2;
                }
                else
                    ledFade[led].FadeDelay--;
            break;			
            case 2:	// Fade Up
                if(++ledFade[led].FadeDelay>=ledFade[led].fadespeed)
                {
                    ledFade[led].FadeDelay = 0;	
                    if(ledFade[led].bright>=ledFade[led].targetbright)
                        ledFade[led].state = 0;	
                    else						
                        ledFade[led].bright++;

                    updatePWM = 1;                
                }		
            break;
            case 3: // Waiting to fade down
                if(ledFade[led].FadeDelay==0)
                {
                    ledFade[led].state = 4;
                }
                else
                    ledFade[led].FadeDelay--;	
            break;		
            case 4:	// Fade Down
                if(++ledFade[led].FadeDelay>=ledFade[led].fadespeed)
                {
                    ledFade[led].FadeDelay = 0;
                    if(ledFade[led].bright<=ledFade[led].targetbright)
                        ledFade[led].state = 0;
                    else				
                        ledFade[led].bright--;

                    updatePWM = 1;
                }		
            break;
        }
        
        if(updatePWM == 1)
        {
            // scale LED brightness with global brighness
            brcalc = (ledFade[led].bright * ledFade[led].globalbright) / 63;            
            
            // write new brightness to PWM register	
            switch(led)
            {
                case 0:
                    OC3RS = PWM_RES-PWMTABLE_LED[brcalc];		// red sys led
                break;
                case 1:
                    OC2RS = PWM_RES-PWMTABLE_LED[brcalc];		// red sys led
                break;
                
            }
        }

    }
}

