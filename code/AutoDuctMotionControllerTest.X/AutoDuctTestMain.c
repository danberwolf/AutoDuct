// AutoDuct firmware main module
// (C) 2023-09-09 by Daniel Porzig

#include "HardwareProfile.h"
#include "uart2.h"
#include "uart1.h"
#include "circbuffer.h"
#include <time.h>
#include "userConsole.h"
#include <plib.h>
#include <proc/p32mx150f128b.h>
#include "LEDFade.h"
#include "Delay.h"
#include "sht3x.h"
#include "ValveMotionControl.h"
#include "FanControl.h"
#include "DeviceControl.h"
#include "BTCom.h"
#include "TimeKeeper.h"
#include "BTComCallbacksApp.h"
#include "BootLoader.h"
#include "RTC_RV3129.h"



// original config
/** CONFIGURATION **************************************************/

// oscillator setup for 40 MHz system clock
#pragma config FPLLIDIV = DIV_2         // PLL Input Divider
#pragma config FPLLMUL  = MUL_20        // PLL Multiplier       
#pragma config FPLLODIV = DIV_2         // PLL Output Divider

#pragma config FUSBIDIO = OFF                  // Controlled by Port Function  
#pragma config FVBUSONIO = OFF                 // Controlled by Port Function  

// PBClock 40 MHz
#pragma config FPBDIV   = DIV_1         // Peripheral Clock divisor

#pragma config FNOSC = FRCPLL           // Fast RC Osc (FRC) with PLL  
#pragma config FSOSCEN = ON             // Secondary osc enabled (RTCC)
#pragma config IESO = OFF               // Internal/External Switch Over disabled
#pragma config POSCMOD = OFF            // Primary osc disabled  
#pragma config OSCIOFNC = OFF           // CLKO Output Signal  Disabled  
#pragma config FCKSM = CSDCMD           // Clock Switch Disable, FSCM Disabled  
#pragma config WDTPS = PS4096           // Watchdog Timer Postscale (4096 should be ~4 seconds)
#pragma config FWDTEN = OFF             // WDT Disabled (SWDTEN Bit Controls)  
#pragma config DEBUG = OFF              // Debugger is disabled  
#pragma config ICESEL = ICS_PGx1        // ICE EMUC1/EMUD1 pins shared with PGC1/PGD1  
#pragma config PWP = OFF                // Flash write protect Disabled
#pragma config BWP = OFF                // Boot Flash Write Protection Disabled  
#pragma config CP = OFF                 // Protection Disabled  
        
#pragma config JTAGEN = OFF             // JTAG Disabled  

#pragma config PMDL1WAY = OFF           // Allow multiple reconfigurations  
#pragma config IOL1WAY = OFF            // Allow multiple reconfigurations  


// firmware version string:
//  2 digits - Major FW version
//  2 chars  - TC: test version, PR: production version
//  6 digits - date: YYMMDD
//  1 digit  - minor FW version

const char __attribute__((space(prog),address(APP_FWSTRING_BASE_ADDRESS)))  FW_VERS_STRING[] = "01TC230612.0";        // 13 chars


char txt[256];


static void InitializeBoard(void);


 static enum {
      EXCEP_IRQ = 0,            // interrupt
      EXCEP_AdEL = 4,            // address error exception (load or ifetch)
      EXCEP_AdES,                // address error exception (store)
      EXCEP_IBE,                // bus error (ifetch)
      EXCEP_DBE,                // bus error (load/store)
      EXCEP_Sys,                // syscall
      EXCEP_Bp,                // breakpoint
      EXCEP_RI,                // reserved instruction
      EXCEP_CpU,                // coprocessor unusable
      EXCEP_Overflow,            // arithmetic overflow
      EXCEP_Trap,                // trap (possible divide by zero)
      EXCEP_IS1 = 16,            // implementation specfic 1
      EXCEP_CEU,                // CorExtend Unuseable
      EXCEP_C2E                // coprocessor 2
  } _excep_code;
  
  static unsigned int _epc_code;
  static unsigned int _excep_addr;

  void _general_exception_handler(void)
  {
      asm volatile("mfc0 %0,$13" : "=r" (_excep_code));
      asm volatile("mfc0 %0,$14" : "=r" (_excep_addr));
  
      _excep_code = (_excep_code & 0x0000007C) >> 2;
      
      DEBUG_puts("\r\nGeneral Exception ");
      switch(_excep_code){
        case EXCEP_IRQ: DEBUG_puts ("interrupt");break;
        case EXCEP_AdEL: DEBUG_puts ("address error exception (load or ifetch)");break;
        case EXCEP_AdES: DEBUG_puts ("address error exception (store)");break;
        case EXCEP_IBE: DEBUG_puts ("bus error (ifetch)");break;
        case EXCEP_DBE: DEBUG_puts ("bus error (load/store)");break;
        case EXCEP_Sys: DEBUG_puts ("syscall");break;
        case EXCEP_Bp: DEBUG_puts ("breakpoint");break;
        case EXCEP_RI: DEBUG_puts ("reserved instruction");break;
        case EXCEP_CpU: DEBUG_puts ("coprocessor unusable");break;
        case EXCEP_Overflow: DEBUG_puts ("arithmetic overflow");break;
        case EXCEP_Trap: DEBUG_puts ("trap (possible divide by zero)");break;
        case EXCEP_IS1: DEBUG_puts ("implementation specfic 1");break;
        case EXCEP_CEU: DEBUG_puts ("CorExtend Unuseable");break;
        case EXCEP_C2E: DEBUG_puts ("coprocessor 2");break;
      }
//      Uart1TxStringPolled(" at 0x");
//      Uart1TxUint32HexPolled(_excep_addr);
//      Uart1TxStringPolled("\r\n");
      while (1) 
      {
          asm("nop");
          // Examine _excep_code to identify the type of exception
          // Examine _excep_addr to find the address that caused the exception
      }
      
  }


// Handle communication via BLE interface and debug console
void UserInput_Task(void *pvParameters, BYTE *skiprate)
{
    BTCom_Task();
    UserConsole_Task();
    
    *skiprate = 1;
}



// setup SHT31 sensor for periodic measurement
void ConfigSHT31()
{
    DWORD serialNumber;
    regStatus status;
    BYTE error;
    
    float humidityHighSet, temperatureHighSet, humidityHighClear, temperatureHighClear,
            humidityLowClear, temperatureLowClear,humidityLowSet,temperatureLowSet;
    
    SHT3X_Init(0x44);
    
    SHT3X_SoftReset();
    
    error = SHT3x_ReadSerialNumber(&serialNumber);    

    
    sprintf(txt,"\n\rSHT31 Serial: %X, error: %X", serialNumber, error);
    DEBUG_puts(txt); 


    error = SHT3X_ReadStatus(&status.u16);    
    sprintf(txt,"\n\rSHT31 Status: %X, error: %X", status.u16, error);
    DEBUG_puts(txt);     
    
    //override default temperature and humidity alert limits (red LED)
    error = SHT3X_SetAlertLimits(   70.0f,  50.0f,  // high set:   RH [%], T [°C]
                            68.0f,  48.0f,  // high clear: RH [%], T [°C]
                            32.0f,  -2.0f,  // low clear:  RH [%], T [°C]
                            30.0f,  -4.0f); // low set:    RH [%], T [°C]  
    
    sprintf(txt,"\n\rSHT31 SetAlertLimits, error: %X", error);
    DEBUG_puts(txt);      
    
    //start periodic measurement, with high repeatability and .5 measurements per second
    error = SHT3X_StartPeriodicMeasurment(REPEATAB_HIGH, FREQUENCY_HZ5);    

    sprintf(txt,"\n\rSHT3X_StartPeriodicMeasurment, error: %X", error);
    DEBUG_puts(txt);      

    
    SHT3X_GetAlertLimits(&humidityHighSet,   &temperatureHighSet,
                             &humidityHighClear, &temperatureHighClear,
                             &humidityLowClear,  &temperatureLowClear,
                             &humidityLowSet,    &temperatureLowSet);
    
    sprintf(txt,"\n\rlimits: %3.1f, %3.1f, %3.1f, %3.1f, %3.1f, %3.1f, %3.1f, %3.1f",humidityHighSet,   temperatureHighSet,
                             humidityHighClear, temperatureHighClear,
                             humidityLowClear,  temperatureLowClear,
                             humidityLowSet,    temperatureLowSet);
    DEBUG_puts(txt);     
    
    
}




int main(void)
{ 
   
	// Initialize application specific hardware
	InitializeBoard();

    Delayms(20);     
    
    // initialize UART interfaces for BLE module and debug console
	UART2Init();   
    UART1Init();
 
    DEBUG_puts("\n\r");   
    DEBUG_puts("\n\r");   
    DEBUG_puts("\n\r");   
    DEBUG_puts("\n\r");   
    
    DEBUG_puts("\n\r+--------------------------------------------------+");   
    DEBUG_puts("\n\r|              AutoDuct Test Firmware              |");   
    DEBUG_puts("\n\r+--------------------------------------------------+");   
    DEBUG_puts("\n\r");   
    DEBUG_puts("\n\rFirmware version: ");       
    DEBUG_puts(FW_VERS_STRING);           
    DEBUG_puts("\n\rBootcode version: ");       
    getBootcodeVersionString(txt);
    DEBUG_puts(txt);   
    
    // initialize debug console
    UserConsole_Init();
    // initialize task scheduler
    Scheduler_Init();    
    // initialize LED fading controller
    LEDFade_Init();
    // initialize shutter motion controller
    ValveMotionControl_Init();
    // initialize PWM input and output for fan control
    FanControl_Init();  
    // initialize main device state machine 
    DeviceControl_Init();
    // initialize protocol handler for BLE communication
    BTCom_Init();
    BTCom_SetupCallbacks();
    // initialize SHT31 humidity/temperature sensor
    SHT3X_Init(0x44);
    // initialize configuration storage EEPROM
    EEPROM_init(0x50);
    // initialize Real Time Clock (RTC)
    rv3129_init();
    // load device config from EEPROM
    LoadConfig();
    // initialize time keeper module
    TimeKeeper_Init();        
    // setup LED fading engine
    LEDFade_SetGlobalBrightness(0, 40);    
    LEDFade_SetSequenceDynamicMax(1, 30);
    LEDFade_StartSeq(0,seq_breathing_fast, NULL);
    LEDFade_StartSeq(1,seq_breathing, NULL);            
    
      
    // register tasks
    Scheduler_AddTask(0, DeviceControl_Task, NULL, 1, 0);              
    Scheduler_AddTask(1, LEDFade_Task, NULL, 1, 0);
    Scheduler_AddTask(2, FanControl_Task, NULL, 1, 0);    
    Scheduler_AddTask(3, UserInput_Task, NULL, 1, 0);        
    Scheduler_AddTask(4, ValveMotionControl_Task, NULL, 1, 0);        
    Scheduler_AddTask(5, TimeKeeper_Task, NULL, 1, 0);           

    DEBUG_puts("\n\r\n\rBoard Init complete.\n\r\n\r");        
 
    
    // execute task scheduler
    while(1)
    {    
        Scheduler_Run();        
    }

}









// initialize PIC32 peripherals
static void InitializeBoard(void)
{	
    
    ANSELA = 0x0000;       // all pins digital
    ANSELB = 0x0000;        // all pins digital
    
    ANSELBbits.ANSB12 = 1;  // pin B12 used for motor current sense
    
    
	// +------------------------------+
	// | PIN SETUP                    |
	// +------------------------------+

	//UART 2   - FTDI
	RX_CON_TRIS = 1;
	TX_CON_TRIS = 0;

    //UART 1   - HM-17
 	RX_BLE_TRIS	= 1; // input (RX)
 	TX_BLE_TRIS = 0; // output (TX)    
    
    // % LED INDICATORS										
    // SYSLED	PWM	OC5	52	RD4				
    SYSLED_TRIS = 0;
    SYSLED = 1;
    
    STATLED_TRIS = 0;
    STATLED = 0;
    
    MOTOR_DIR_TRIS = 0;
    MOTOR_PWM_TRIS = 0;
    MOTOR_SLEEP_TRIS = 0;
    MOTOR_SLEEP = 1;
    
    MOTOR_FAULT_TRIS = 1;
    MOTOR_ADC_TRIS = 1;

    // PWM in/out
    FAN_PWM_IN_TRIS = 1;
    FAN_PWM_OUT_TRIS = 0;
    FAN_PWM_OUT_ODC = 0;        // enable open drain config

    // end stop sensors
    ENDSTOP_LOW_TRIS = 1;
    ENDSTOP_HIGH_TRIS = 1;

        
 	// +------------------------------+
	// | Setup PPS                    |
	// +------------------------------+
   
    // defined in <pps.h>

    // inputs
    PPSInput(2, INT3, RPB11); //INT3 -> B11       
    PPSInput(3, INT2, RPB13); //INT2 -> B13               
    PPSInput(3, IC1, RPA2); //IC1 -> A2    
    PPSInput(2, U2RX, RPB5); //U2RX -> B5        
    PPSInput(4, INT1, RPB14); //INT1 -> B14  
    PPSInput(3, U1RX, RPB2); //U1RX -> B2    
    
    
    // outputs
    PPSOutput(1, RPA0, OC1); //OC1  -> A0
    PPSOutput(2, RPA1, OC2); //OC2  -> A1
    PPSOutput(4, RPA3, U2TX); //U2TX -> A3
    PPSOutput(4, RPB10, OC3); //OC3  -> B10
    PPSOutput(3, RPB6, OC5); //OC5 -> B6    
    PPSOutput(1, RPB3, U1TX); //U1TX -> B3
    
    
	// configure for multi-vectored mode
	INTConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);
	
	// enable interrupts
	INTEnableInterrupts();

	mJTAGPortEnable(DEBUG_JTAGPORT_OFF);					// JTAG port must be disabled to use PORTA
		
    // Enable optimal performance
    SYSTEMConfigPerformance(GetSystemClock());
    mOSCSetPBDIV(OSC_PB_DIV_1);				// Use 1:1 CPU Core:Peripheral clocks
		
    
    // Disable JTAG port so we get our I/O pins back, but first
    DDPCONbits.JTAGEN = 0;
       
    
}

