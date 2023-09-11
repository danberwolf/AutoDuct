// Main AutoDuct control state machine
// (C) 2023-09-09 by Daniel Porzig

#include <proc/p32mx150f128b.h>

#include "ValveMotionControl.h"
#include "DeviceControl.h"
#include "TimeKeeper.h"
#include "FanControl.h"
#include "LEDFade.h"
#include "sht3x.h"





typedef struct DeviceCTL_Struct
{
    BYTE mode;                      // slave/manual/smart
    
    BYTE ventEventActive;           // indicator for active venting events 
                                    // Bit 0 = manual venting active
                                    // Bit 1 = scheduled venting active
                                    // Bit 2 = humidity control venting active
    
	BYTE state;
    BYTE substate;
    
    BYTE fandir, fandir_last;       // measured fan direction (PWM input)
    BYTE fanspeed, fanspeed_last;   // measured fan speed (PWM input)

    BYTE fandir_out, fanspeed_out;  // fan speed and direction output (PWM out)
                                    // (only used for manual vent event)
    
    BYTE fanmode_manual;

	WORD delaycnt;
    
    
    BYTE SHT31_state;
    WORD SHT31_delaycnt;
    WORD SHT31_errorcnt;
    DWORD SHT31_serial;     // serial number of SHT31
    float SHT31_hum;
    float SHT31_temp;
    
    // FanCTLSequenceData Sequence;
}DeviceCTLData;	

DeviceCTLData DevCTL;

char txt[200];

// initialize main device state machine
void DeviceControl_Init()
{
    DevCTL.mode = DEVICEMODE_SLAVE;
    
    DevCTL.ventEventActive = 0;
    
    DevCTL.fandir = 0;
    DevCTL.fandir_last = 0;
    
    DevCTL.fanspeed = 0;
    DevCTL.fanspeed_last = 0;
    
    DevCTL.state = 0;   // uninitialized
    
    DevCTL.delaycnt = 0;
    
    DevCTL.SHT31_state = SHT31_UNINITIALIZED;
    DevCTL.SHT31_serial = 0xFFFFFFFF;
    DevCTL.SHT31_delaycnt = 0;
    DevCTL.SHT31_hum = 0.0f;
    DevCTL.SHT31_temp = 0.0f;
    DevCTL.SHT31_errorcnt = 0;
}

// state machine for temperature/humidity measurement
void DeviceControl_SHT31_StateMachine()
{
    DWORD serialNumber;
    regStatus status;
    etError error;

    float humidity, temperature; 
    
    switch(DevCTL.SHT31_state)
    {
        case SHT31_UNINITIALIZED:
            
            if(DevCTL.SHT31_delaycnt == 0)
            {
                // reset SHT31 into known state
                SHT3X_SoftReset();

                // read serial / part number
                error = SHT3x_ReadSerialNumber(&DevCTL.SHT31_serial);    

//                sprintf(txt,"\n\rSHT31 Serial: %X, error: %X", DevCTL.SHT31_serial, error);
//                DEBUG_puts(txt); 

                // read status
                error = SHT3X_ReadStatus(&status.u16);    
//                sprintf(txt,"\n\rSHT31 Status: %X, error: %X", status.u16, error);
//                DEBUG_puts(txt);        
                
                if(error == NO_ERROR)
                {
                    // start Measurement
                    error = SHT3X_StartMeasurement_Polling(REPEATAB_HIGH);
                    
                    if(error == NO_ERROR)
                    {
                        DevCTL.SHT31_state = SHT31_MEASURING;
                        DevCTL.SHT31_delaycnt = SHT31_MEASURE_DELAY;
                        
                        DevCTL.SHT31_errorcnt = 0;
                    }
                    else
                    {
                        // wait a couple of seconds and try to initialize again
                        DevCTL.SHT31_delaycnt = SHT31_MEASURE_DELAY;
                        
                        sprintf(txt,"\n\r(A) SHT31 error: %X, cnt: %d", error, DevCTL.SHT31_errorcnt);
                        DEBUG_puts(txt);                        
                        
                        DevCTL.SHT31_errorcnt++;
                        if(DevCTL.SHT31_errorcnt == SHT31_TIMEOUT_CNT)
                        {
                            // wait and try again
                            DevCTL.SHT31_delaycnt = SHT31_MEASURE_INTERVAL;                    
                        }                        
                    }
                }
                else
                {
                    // wait a couple of seconds and try to initialize again
                    DevCTL.SHT31_delaycnt = SHT31_MEASURE_DELAY;
                }
                
            }
            else
                DevCTL.SHT31_delaycnt--;
            
            
        break;
        case SHT31_MEASURING:
            
            if(DevCTL.SHT31_delaycnt == 0)
            {
                error = SHT3X_ReadTempAndHumi_Polling(&DevCTL.SHT31_temp, &DevCTL.SHT31_hum); 
                
                if(error == NO_ERROR)
                {
                    sprintf(txt,"\n\rtemp: %3.2f, humidity: %3.2f", DevCTL.SHT31_temp, DevCTL.SHT31_hum);
                    DEBUG_puts(txt);       
                    
                    DevCTL.SHT31_delaycnt = SHT31_MEASURE_INTERVAL;
                    DevCTL.SHT31_state = SHT31_WAITING;
                    
                    DevCTL.SHT31_errorcnt = 0;
                }
                else
                {
                    // wait a couple of seconds and try again
                    DevCTL.SHT31_delaycnt = SHT31_MEASURE_DELAY;
                    
                    sprintf(txt,"\n\r(B) SHT31 error: %X, cnt: %d", error, DevCTL.SHT31_errorcnt);
                    DEBUG_puts(txt);       
                    
                    DevCTL.SHT31_errorcnt++;
                    if(DevCTL.SHT31_errorcnt == SHT31_TIMEOUT_CNT)
                    {
                        // wait and try again
                        DevCTL.SHT31_delaycnt = SHT31_MEASURE_INTERVAL;
                        DevCTL.SHT31_state = SHT31_WAITING;                        
                    }
                    
                }
            }
            else
                DevCTL.SHT31_delaycnt--;
            
        break;
        case SHT31_WAITING:
            
            if(DevCTL.SHT31_delaycnt == 0)
            {
                // start Measurement
                error = SHT3X_StartMeasurement_Polling(REPEATAB_HIGH);

                if(error == NO_ERROR)
                {
                    DevCTL.SHT31_state = SHT31_MEASURING;
                    DevCTL.SHT31_delaycnt = SHT31_MEASURE_DELAY;
                }
                else
                {
                    // wait a couple of seconds and try to initialize again
                    DevCTL.SHT31_delaycnt = SHT31_MEASURE_DELAY;

                    sprintf(txt,"\n\r(C) SHT31 error: %X", error);
                    DEBUG_puts(txt);                        

                }
            }
            else
                DevCTL.SHT31_delaycnt--;
            
        break;
            
    }
    
}




// callback function for a manual vent event timeout
void DeviceControl_callbackStopVent()
{
    if(Valve_isOpened())
    {
        // close valve
        DEBUG_puts("DEVCTL: closing valve\n\r"); 
        Valve_Close(63, 0);                            
    }    
    
    // ramp down fan speed 
    FanSpeedControl_Ramp(DevCTL.fandir_out, 0, 0, 1);    
    
    // disable timer for alternating fan direction (if enabled)
    TimeKeeper_SetupTimeout(TIMEOUT_ID_FANALT, 0, NULL);
    
    
}


// callback function for a smart vent event timeout
void DeviceControl_callbackStopSmartVent()
{
    if(Valve_isOpened())
    {
        // close valve
        DEBUG_puts("DEVCTL: closing valve\n\r"); 
        Valve_Close(63, 0);                            
    }    
    
    // ramp down fan speed 
    FanSpeedControl_Ramp(DevCTL.fandir_out, 0, 0, 1);    
    
    // disable timer for alternating fan direction (if enabled)
    TimeKeeper_SetupTimeout(TIMEOUT_ID_FANALT, 0, NULL);
    
}


// callback function for fan direction change in alternate/smart mode
void DeviceControl_callbackAlternateDir()
{
    
    
    if(DevCTL.fanspeed_out > 0)
    {
        if(DevCTL.fandir_out)
        {
            DevCTL.fandir_out = 0;
            DEBUG_puts("DEVCTL: callbackAlternateDir, fandir = 0\n\r"); 
        }
        else
        {
            DevCTL.fandir_out = 1;
            DEBUG_puts("DEVCTL: callbackAlternateDir, fandir = 1\n\r"); 
        }
        // ramp up fan speed after a short delay
        FanSpeedControl_Ramp(DevCTL.fandir_out, 0, DevCTL.fanspeed_out, 2);          
        
        // set timer to alternate direction every 2 minutes
        TimeKeeper_SetupTimeout(TIMEOUT_ID_FANALT, 2, DeviceControl_callbackAlternateDir);    
    }
}



// activate manual venting (either infinite or with specific duration)
void DeviceControl_ManualVent(BYTE fanmode, BYTE fandir, BYTE fanspeed, BYTE minutes)
{
    
    // reset any timeouts in case manual/auto vent active
    TimeKeeper_ResetTimeout(TIMEOUT_ID_AUTOVENT);
    TimeKeeper_ResetTimeout(TIMEOUT_ID_FANALT);
    TimeKeeper_ResetTimeout(TIMEOUT_ID_MANUALVENT);    
    
    if(fanmode == FAN_ALTERNATING)
    {
        // alternating mode active
        
        // set timer to alternate direction every 2 minutes
        TimeKeeper_SetupTimeout(TIMEOUT_ID_FANALT, 2, DeviceControl_callbackAlternateDir);
    }
    
    if(fanspeed > 4)
        fanspeed = 4;
    
    DevCTL.fandir_out = fandir;
    DevCTL.fanspeed_out = fanspeed;
    
    if(fanspeed > 0)
    {
        // open Valve
        Valve_Open(63, 0);

        // ramp up fan speed after a short delay
        FanSpeedControl_Ramp(DevCTL.fandir_out, FAN_STARTUP_DELAY, DevCTL.fanspeed_out, 2);    
    
    
        // set timeout
        if(minutes > 0)
        {
            TimeKeeper_SetupTimeout(TIMEOUT_ID_MANUALVENT, minutes, (VoidFnctpv) DeviceControl_callbackStopVent);
        }
        else
        {
            ; // infinite 
        }
    }
    else
    {
        if(Valve_isOpened())
        {
            // close valve
            DEBUG_puts("DEVCTL: closing valve\n\r"); 
            Valve_Close(63, 0);                            
        }            
        
        // ramp down fan speed 
        FanSpeedControl_Ramp(DevCTL.fandir_out, 0, 0, 1);            
    }
    
    // acknowledge by LED sequence
    DeviceControl_AcknowledgeLED();
}




void DeviceControl_SmartVent(BYTE fanmode, BYTE fandir, BYTE fanspeed, BYTE minutes)
{
    
    if(fanmode == FAN_ALTERNATING)
    {
        // alternating mode active
        
        // set timer to alternate direction every 2 minutes
        TimeKeeper_SetupTimeout(TIMEOUT_ID_FANALT, 2, DeviceControl_callbackAlternateDir);
    }
    
    if(fanspeed > 4)
        fanspeed = 4;
    
    DevCTL.fandir_out = fandir;
    DevCTL.fanspeed_out = fanspeed;
    
    if(fanspeed > 0)
    {
        // open Valve
        Valve_Open(63, 0);

        // ramp up fan speed after a short delay
        FanSpeedControl_Ramp(DevCTL.fandir_out, FAN_STARTUP_DELAY, DevCTL.fanspeed_out, 2);    
    
    
        // set timeout
        if(minutes > 0)
        {
            TimeKeeper_SetupTimeout(TIMEOUT_ID_AUTOVENT, minutes, (VoidFnctpv) DeviceControl_callbackStopSmartVent);
        }
        else
        {
            ; // infinite 
        }
    }
    else
    {
        if(Valve_isOpened())
        {
            // close valve
            DEBUG_puts("DEVCTL: closing valve\n\r"); 
            Valve_Close(63, 0);                            
        }            
        
        // ramp down fan speed 
        FanSpeedControl_Ramp(DevCTL.fandir_out, 0, 0, 1);            
    }
    
    // acknowledge by LED sequence
    //DeviceControl_AcknowledgeLED();
}







// display blink/fade pattern on front panel LED as user feedback
void DeviceControl_AcknowledgeLED()
{
    // acknowledge by LED sequence
     
    
    //extern const BYTE seq_toggle_4Hz2x[];
    //extern const BYTE seq_breathing_dyn[];
    //extern const BYTE seq_const_dyn[];    
    
    switch(DevCTL.mode)
    {
        case DEVICEMODE_SLAVE:
            
            LEDFade_StartSeq(1,seq_toggle_4Hz2x,NULL); 
            
        break;

        case DEVICEMODE_MANUAL:
            
            LEDFade_StartSeq(1,seq_toggle_4Hz2x,seq_breathing_dyn); 
            
        break;

        case DEVICEMODE_SMART:
            
            LEDFade_StartSeq(1,seq_toggle_4Hz2x,seq_const_dyn); 
            
        break;
    }
        
}





// activate test mode (alternating open/close of shutter)
void DeviceControl_Testmode(BYTE mode)
{
    if(DevCTL.state != DEVCTL_STATE_UNINITIALIZED)
    {
        DevCTL.state = DEVCTL_STATE_TESTMODE1;
        DevCTL.substate = 0;
        DevCTL.delaycnt = 0;
        
        // disable fan control
        FanSpeedControl_Ramp(0, 0, 0, 2); 
    }
    
}


// set device mode (passive / manual / smart)
void DeviceControl_SetMode(BYTE mode)
{
    switch(mode)
    {
        case DEVICEMODE_SLAVE:
            
            DEBUG_puts("DeviceMode: Passive/Slave\n\r");
            
            // reset any timeouts in case manual/auto vent active
            TimeKeeper_ResetTimeout(TIMEOUT_ID_AUTOVENT);
            TimeKeeper_ResetTimeout(TIMEOUT_ID_FANALT);
            TimeKeeper_ResetTimeout(TIMEOUT_ID_MANUALVENT);
            
            DevCTL.mode = DEVICEMODE_SLAVE;
            DevCTL.ventEventActive = 0;
            DevCTL.fandir = 0;
            DevCTL.fandir_last = 0;

            DevCTL.fanspeed = 0;
            DevCTL.fanspeed_last = 0;

            DevCTL.state = 0;   // uninitialized

            DevCTL.delaycnt = 0;            
            
        break;

        case DEVICEMODE_MANUAL:
            
            DEBUG_puts("DeviceMode: Manual\n\r");
            // -> do not change any fan settings but reset auto vent timeout
            
            // reset any timeouts in case manual/auto vent active
            TimeKeeper_ResetTimeout(TIMEOUT_ID_AUTOVENT);
            
            
        break;

        case DEVICEMODE_SMART:
            
            DEBUG_puts("DeviceMode: Smart\n\r");
            
            // check if we are in the activation window of any scheduled event
            TimeKeeper_ForceScheduleUpdates();
            
            // todo:
            // - check activation of a humControl event
            
        break;
    }
    
    DevCTL.mode = mode;
    
    DeviceControl_AcknowledgeLED();
    
    
}

BYTE DeviceControl_GetMode()
{
    return DevCTL.mode;
}




// Task function for Smart device mode
void DeviceControl_Smart_Task()
{
    // event-based venting will be controlled from Timekeeper module
    
    // still some work to do:
    // check humidity levels, if humidity control mode is enabled
    
    // check if humidity value is valid (SHT31 state == SHT31_WAITING)
//    if(C)
        
        // check if current humidity is above threshold. If not already venting       
        
    
    // handle alternating fan direction
    
}



// Task function when device is acting as slave/passive
void DeviceControl_Slave_Task()
{
    switch(DevCTL.state)
    {
        case DEVCTL_STATE_UNINITIALIZED:     // uninitialized
            
            if(DevCTL.delaycnt == 0)
            {
                // get initial fan control setting
                FanControl_getFanLevel(&DevCTL.fanspeed, &DevCTL.fandir);    
                
                sprintf(txt,"DEVCTL: initial fan speed: %d, dir: %d.\n\r",DevCTL.fanspeed, DevCTL.fandir);
                DEBUG_puts(txt);                     
            }
            
            if(++DevCTL.delaycnt == DEVICE_CONTROL_PWM_TIMEOUT_INIT)
            {
                
                DevCTL.fanspeed_last = DevCTL.fanspeed;
                DevCTL.fandir_last = DevCTL.fandir;
                
                // get second fan control setting
                FanControl_getFanLevel(&DevCTL.fanspeed, &DevCTL.fandir);                
                
                if( (DevCTL.fanspeed_last == DevCTL.fanspeed) && (DevCTL.fandir_last == DevCTL.fandir))
                {
                    // fan PWM control signal seems to be stable. Adjust valve setting accordingly
                    
                    if(DevCTL.fanspeed > 0)
                    {
                        // open valve
                        DEBUG_puts("DEVCTL: Initialization - opening valve\n\r"); 
                        Valve_Open(63, 0);
                        
                        // ramp up fan speed after a short delay
                        FanSpeedControl_Ramp(DevCTL.fandir, FAN_STARTUP_DELAY, DevCTL.fanspeed, 2);
                    }
                    else
                    {
                        // close valve
                        DEBUG_puts("DEVCTL: Initialization - closing valve\n\r"); 
                        Valve_Close(63, 0);
                        
                        // ramp down fan speed 
                        FanSpeedControl_Ramp(DevCTL.fandir, 0, 0, 1);
                        
                    }
                    
                    
                   
                    DevCTL.state = DEVCTL_STATE_INITIALIZED;   // go to initialized state
                    DevCTL.delaycnt = 0;
                }
                else
                {

                    sprintf(txt,"DEVCTL #1: new fan speed: %d, dir: %d.\n\r",DevCTL.fanspeed, DevCTL.fandir);
                    DEBUG_puts(txt);                     

                    // update fan speed
                    FanSpeedControl_Ramp(DevCTL.fandir, FAN_STARTUP_DELAY, DevCTL.fanspeed, 2);                    
                    
                    DevCTL.state = DEVCTL_STATE_INITIALIZED;   // go to initialized state
                    DevCTL.delaycnt = 1;                
                }                
            }

            
        break;
        case DEVCTL_STATE_INITIALIZED:     // initialized state
                       
            if(++DevCTL.delaycnt == DEVICE_CONTROL_PWM_TIMEOUT_INIT)
            {
                
                DevCTL.fanspeed_last = DevCTL.fanspeed;
                DevCTL.fandir_last = DevCTL.fandir;
                
                // get next fan control setting
                FanControl_getFanLevel(&DevCTL.fanspeed, &DevCTL.fandir);                
                
                
                if( (DevCTL.fanspeed_last == DevCTL.fanspeed) && (DevCTL.fandir_last == DevCTL.fandir))
                {
                    // fan PWM control signal seems to be stable. Adjust valve setting accordingly
                    
                    DevCTL.state = DEVCTL_STATE_INITIALIZED;   // go to initialized state
                    DevCTL.delaycnt = 0;
                }
                else
                {
                    // fan speed or direction changed
                    
                    sprintf(txt,"DEVCTL #2: new fan speed: %d, dir: %d.\n\r",DevCTL.fanspeed, DevCTL.fandir);
                    DEBUG_puts(txt);                        
                    
                    if(DevCTL.fanspeed == 0)
                    {
                        // special case for zero crossing: if control unit is in "alternating mode" a longer delay is required
                        // before decision can be made to close the valve
                        
                        // ramp down fan speed
                        FanSpeedControl_Ramp(DevCTL.fandir, 0, 0, 2);
                      
                        
                        DevCTL.state = DEVCTL_STATE_ZEROCROSS;   // go to zero crossing special state
                        DevCTL.delaycnt = 0;                        
                    }
                    else
                    {
                        if(Valve_isClosed())
                        {
                            DEBUG_puts("DEVCTL: opening valve\n\r"); 
                            Valve_Open(63, 0);
                            
                            // change fan speed
                            FanSpeedControl_Ramp(DevCTL.fandir, FAN_STARTUP_DELAY, DevCTL.fanspeed, 2);                       
                            
                        }
                        else
                        {
                            // change fan speed
                            FanSpeedControl_Ramp(DevCTL.fandir, 0, DevCTL.fanspeed, 2);                       
                        }                            
                        
                        
                        DevCTL.state = DEVCTL_STATE_INITIALIZED;   // go to initialized state
                        DevCTL.delaycnt = 0;                        
                    }
                    
                    
                    // TODO:
                    // + adjust fan setting
                    
                    
                }
            }            
            
        break;
        case DEVCTL_STATE_ZEROCROSS:     // zero crossing special case

            if(++DevCTL.delaycnt == DEVICE_CONTROL_PWM_TIMEOUT_ZEROCROSS)
            {
                
                DevCTL.fanspeed_last = DevCTL.fanspeed;
                DevCTL.fandir_last = DevCTL.fandir;
                
                // get next fan control setting
                FanControl_getFanLevel(&DevCTL.fanspeed, &DevCTL.fandir);                
                
                
                if( (DevCTL.fanspeed_last == DevCTL.fanspeed) && (DevCTL.fandir_last == DevCTL.fandir))
                {
                    // fan PWM control signal seems to be stable. Adjust valve setting accordingly
                    
                    if(DevCTL.fanspeed == 0)
                    {                  
                        // verify fanspeed == 0
                        
                        if(Valve_isOpened())
                        {
                            // close valve
                            DEBUG_puts("DEVCTL: closing valve\n\r"); 
                            Valve_Close(63, 0);                            
                        }
                                               
                        
                        
                    }  
                    
                    DevCTL.state = DEVCTL_STATE_INITIALIZED;   // go to initialized state
                    DevCTL.delaycnt = 0;
                }  
                else
                {
                    sprintf(txt,"DEVCTL #3: new fan speed: %d, dir: %d.\n\r",DevCTL.fanspeed, DevCTL.fandir);
                    DEBUG_puts(txt);                     
                    
                    // change fan speed
                    FanSpeedControl_Ramp(DevCTL.fandir, 0, DevCTL.fanspeed, 2);                       
                    
                    // signal keeps changing. Go back to initialized state
                    DevCTL.state = DEVCTL_STATE_INITIALIZED;   // go to initialized state
                    DevCTL.delaycnt = 0;                    
                }
            }
            
        break;
        
        
        
        case DEVCTL_STATE_TESTMODE1:    // alternating open / close
            
            
            if(++DevCTL.delaycnt == DEVICE_CONTROL_PWM_TIMEOUT_INIT)
            {
                if(Valve_isClosed())
                {
                    Valve_Open(63, 0);
                    DevCTL.delaycnt = 0;
                }
                else if(Valve_isOpened())
                {
                    Valve_Close(63, 0); 
                    DevCTL.delaycnt = 0;
                    
                }
                else
                {
                    DevCTL.delaycnt = 0;    // wait
                }
            }
                
        break;
    }
	    
    
}



// -react on changes of measured PWM control signal
// -handle fan control
// -offer test modes for debugging
void DeviceControl_Task(void *pvParameters, BYTE *skiprate)
{
	
    // perform SHT31 measurement
    DeviceControl_SHT31_StateMachine();
    
	// period: every 250ms sufficient?
	
    switch(DevCTL.mode)
    {
        case DEVICEMODE_SLAVE:
            DeviceControl_Slave_Task();
        break;
        case DEVICEMODE_MANUAL:
            ;   // do nothing
        break;
        case DEVICEMODE_SMART:
            DeviceControl_Smart_Task();
        break;
    }

    
    *skiprate = 1;    
}
    



// assemble status message to sent via BLE interface
void DeviceControl_StatusBTCom(BYTE *outbuf, BYTE *len)
{
//    FLOAT_VAL   fVal;
//    WORD_VAL    wVal;
    
    WORD_VAL wVal1, wVal2;    
    
    BYTE *bptr = &outbuf[1];
    BYTE *ptr;
    BYTE j, HumctlFan;

	// outbuf[0] = CMD              // 0

    outbuf[1] = DevCTL.mode;
            
    outbuf[2] = ValveMotionControl_getState();
    
    HumctlFan = 0;
    HumctlFan &= 0x1C;  // clear bits 0..1
    HumctlFan |= ((FanSpeedControl_getDir()) & 0x03);  


    
    HumctlFan &= 0x03;  // clear bits 2..7
//    HumctlFan |= ((FanSpeedControl_getSpeed() << 2) & 0x1C);    
    HumctlFan |= ((FanSpeedControl_getFanLevel() << 2) & 0x1C);    
    
    outbuf[3] = HumctlFan;
    
    // current Temperature
    wVal1.Val = (WORD)(DevCTL.SHT31_temp * 100.0f);
    outbuf[4] = wVal1.v[1];
    outbuf[5] = wVal1.v[0];
    
    // current Humidity
    wVal1.Val = (WORD)(DevCTL.SHT31_hum * 100.0f);
    outbuf[6] = wVal1.v[1];
    outbuf[7] = wVal1.v[0];
    
    // venting indicator
    outbuf[8] = DevCTL.ventEventActive;
    
    // reserved
    
    outbuf[9]  = 0xFF;
    outbuf[10] = 0xFF;
    outbuf[11] = 0xFF;
    outbuf[12] = 0xFF;
    outbuf[13] = 0xFF;
    outbuf[14] = 0xFF;
    outbuf[15] = 0xFF;

    // DEBUGGING 2023-06-12:
    // send measured PWM signal pulse/duration values
    FanControl_getPWMSignalRaw(&wVal1.Val, &wVal2.Val);
    outbuf[9] = wVal1.v[1];
    outbuf[10] = wVal1.v[0];
    outbuf[11] = wVal2.v[1];
    outbuf[12] = wVal2.v[0];
    outbuf[13] = DevCTL.state;

    
    *len = 16;
}
