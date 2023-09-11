// AutoDuct Hardware definition file
// (C) 2023-09-09 by Daniel Porzig

#ifndef HARDWARE_PROFILE_H
#define HARDWARE_PROFILE_H


#include "Compiler.h"

   /** CPU Clock Speed Assignment *************************************/
    
	#define GetSystemClock()		(40000000ul)      // Hz
//	#define GetSystemClock()		(48000000ul)      // Hz
	#define GetInstructionClock()	(GetSystemClock())
	#define GetPeripheralClock()	(GetInstructionClock() / 1)	// Set your divider according to your Peripheral Bus Frequency configuration fuse setting



// ====================================================
//   Hardware mappings
// ====================================================

// BDC Motor Driver
#define MOTOR_DIR_TRIS      (TRISBbits.TRISB15)
#define MOTOR_DIR           (LATBbits.LATB15)

#define MOTOR_PWM_TRIS      (TRISAbits.TRISA0)
#define MOTOR_PWM           (LATAbits.LATA0)


#define MOTOR_SLEEP_TRIS    (TRISBbits.TRISB7)
#define MOTOR_SLEEP         (LATBbits.LATB7)

#define MOTOR_FAULT_TRIS    (TRISBbits.TRISB11)
#define MOTOR_FAULT         (LATBbits.LATB11)

#define MOTOR_ADC_TRIS      (TRISBbits.TRISB12)

// % LED INDICATORS										
#define SYSLED_TRIS         (TRISBbits.TRISB10)
#define SYSLED              (LATBbits.LATB10)

#define STATLED_TRIS         (TRISAbits.TRISA1)
#define STATLED              (LATAbits.LATA1)


//UART 2   - DEBUG
#define 	RX_CON_TRIS		(TRISBbits.TRISB5)      // input (RX)
#define 	TX_CON_TRIS		(TRISAbits.TRISA3)      // output (TX)

//UART 1   - HM-17
#define 	RX_BLE_TRIS		(TRISBbits.TRISB2)      // input (RX)
#define 	TX_BLE_TRIS		(TRISBbits.TRISB3)      // output (TX)


// PWM in/out
#define 	FAN_PWM_IN_TRIS		(TRISAbits.TRISA2)
#define 	FAN_PWM_OUT_TRIS	(TRISBbits.TRISB6)
#define 	FAN_PWM_OUT_ODC     (ODCBbits.ODCB6)      // open drain config


// user buttons
#define 	BUTTON1_TRIS		(TRISBbits.TRISB3)
#define 	BUTTON1             (PORTBbits.RB3)
        
#define 	BUTTON2_TRIS		(TRISAbits.TRISA1)
#define 	BUTTON2             (PORTAbits.RA1)

#define 	BUTTON3_TRIS		(TRISBbits.TRISB2)
#define 	BUTTON3             (PORTBbits.RB2)

// end stop sensors
#define 	ENDSTOP_LOW_TRIS		(TRISBbits.TRISB14)
#define 	ENDSTOP_LOW             (PORTBbits.RB14)
        
#define 	ENDSTOP_HIGH_TRIS		(TRISBbits.TRISB13)
#define 	ENDSTOP_HIGH            (PORTBbits.RB13)

#define BAUDRATE_UART2      115200UL
//// for PBCLOCK 40MHz
#define BRG_DIV_UART2           4   // this is NOT the baud rate register value!
#define BRGH_UART2              1

#define BAUDRATE_UART1      9600UL
//// for PBCLOCK 40MHz
#define BRG_DIV_UART1           16   // this is NOT the baud rate register value!
#define BRGH_UART1              0


//// for PBCLOCK 48MHz
//#define BRG_DIV_UART2           16  // this is NOT the baud rate register value!    
//#define BRGH_UART2               0


#define DEBUG_puts      UART2PrintString




#endif // #ifndef HARDWARE_PROFILE_H


