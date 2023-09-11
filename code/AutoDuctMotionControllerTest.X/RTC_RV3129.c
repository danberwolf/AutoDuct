// RV3129-C3 Real Time Clock Module Driver
// (C) 2023-09-09 by Daniel Porzig
#include "rtc_rv3129.h"
#include <plib.h>
#include "HardwareProfile.h"
#include "GenericTypeDefs.h"
#include "Compiler.h"

// I²C address
#define RV3129_ADDRESS          0xAC    // 0b10101100 (write)

// register addresses
#define REG_CONTROL_1           0x00
#define REG_CONTROL_INT         0x01
#define REG_CONTROL_INT_FLAG    0x02
#define REG_CONTROL_STATUS      0x03
#define REG_CONTROL_RESET       0x04

#define REG_CLOCK_SEC           0x08
#define REG_CLOCK_MIN           0x09
#define REG_CLOCK_HOUR          0x0A
#define REG_CLOCK_DAY           0x0B
#define REG_CLOCK_WDAY          0x0C
#define REG_CLOCK_MONTH         0x0D
#define REG_CLOCK_YEAR          0x0E

//...

#define REG_TEMPERATURE         0x20

//...

#define RTC_REG_RAM_BASE        0x38




//
////check if rv3129 is still busy
//
//BYTE rv3129_check_busy()
//{
//    StartI2C1();        // start condition
//    IdleI2C1();     //Wait to complete    
//
//    MasterWriteI2C1( (RV3129_ADDRESS ) | 0x00  );      // send slave address (write)    
//    
//    IdleI2C1();		//Wait to complete
//
//    if( I2C1STATbits.ACKSTAT == 0 )	//eeprom has acknowledged
//    {
//        StopI2C1();	//Send the Stop condition
//        IdleI2C1();	//Wait to complete
//        return 0;
//    }
//    else
//    {
//        StopI2C1();	//Send the Stop condition
//        IdleI2C1();	//Wait to complete
//        return 1;       
//        
//    }
//    
//}



// write access to RV3129 register
BYTE rv3129_write_reg(BYTE address, BYTE *data, BYTE length)
{

	BYTE i;
	BYTE *bptr = data;

    // write bytes to page
    StartI2C1();        // start condition
    IdleI2C1();     //Wait to complete    

    MasterWriteI2C1( (RV3129_ADDRESS) | 0x00  );      // send slave address (write)    
    IdleI2C1();		//Wait to complete

    MasterWriteI2C1( address );    
    IdleI2C1();		//Wait to complete

    for(i=0; i<length; i++)
    {
        MasterWriteI2C1( *bptr );    // send data byte
        IdleI2C1();		//Wait to complete     
        bptr++;
    }

    StopI2C1();	//Send the Stop condition
    IdleI2C1();	//Wait to complete        
      
	return 1;
}



// read access to RV3129 register
BYTE rv3129_read_reg(BYTE address, BYTE *data, BYTE length)
{
	BYTE i;
    
    StartI2C1();        // start condition
    IdleI2C1();     //Wait to complete    

    MasterWriteI2C1( (RV3129_ADDRESS) | 0x00  );      // send slave address (write)    
    IdleI2C1();		//Wait to complete

    MasterWriteI2C1( address );    
    IdleI2C1();		//Wait to complete
    
    
    StopI2C1();	//Send the Stop condition
    IdleI2C1();	//Wait to complete        

    
    StartI2C1();        // start condition
    IdleI2C1();     //Wait to complete    
    
    MasterWriteI2C1( (RV3129_ADDRESS) | 0x01  );      // send slave address (read)    
    IdleI2C1();		//Wait to complete
    
    for(i=0; i<length; i++)
    {
        data[i] = MasterReadI2C1();    
        IdleI2C1();     //Wait to complete    
        
        if(i == (length - 1))
        {
            NotAckI2C1();  
            IdleI2C1();     //Wait to complete  
        }
        else
        {
            AckI2C1(); 
            IdleI2C1();     //Wait to complete  
        }
    }
    
    StopI2C1();	//Send the Stop condition
    IdleI2C1();	//Wait to complete
    return 1;       
    
}




// read time from RV3129
void rtc_get_time(BYTE* hour, BYTE* min, BYTE* sec)
{
	BYTE res;
    rv3129_read_reg(REG_CLOCK_SEC,&res,1);  // seconds
	*sec = bcd2dec(res);

	rv3129_read_reg(REG_CLOCK_MIN,&res,1);		// minutes
	*min = bcd2dec(res);

	rv3129_read_reg(REG_CLOCK_HOUR,&res,1);		// hour
	res = res & 0b00111111;
	*hour = bcd2dec(res);
}

// set time
void rtc_set_time(BYTE hour, BYTE min, BYTE sec, BYTE hourmode)
{
	BYTE res;

	res = dec2bcd(hour) & 0b00111111;
	
	if(hourmode == 0)	// 12h mode
		res |= (1<<6);
	else				// 24h mode
		res &= ~(1<<6);
	
	rv3129_write_reg(REG_CLOCK_HOUR,&res,1);
    
    res = dec2bcd(sec);
    rv3129_write_reg(REG_CLOCK_SEC,&res,1);
    
    res = dec2bcd(min);
    rv3129_write_reg(REG_CLOCK_MIN,&res,1);

}

	
// read date
void rtc_get_date(BYTE* weekday, BYTE* day, BYTE* month, WORD* year)
{
    BYTE res;
    
    rv3129_read_reg(REG_CLOCK_WDAY,&res,1); 
	*weekday = res  & 0b00000111;				// weekday (1..7) (1 = Sunday, 7=Saturday)
//    // convert weekday to Timekeeper representation
//    *weekday -= 1; // weekday (0..6) (0 = Sunday, 6=Saturday)

    rv3129_read_reg(REG_CLOCK_DAY,&res,1); 
	*day     = bcd2dec(res & 0b00111111);		// day
    
    rv3129_read_reg(REG_CLOCK_MONTH,&res,1); 
	*month   = bcd2dec(res & 0b00011111);		// month
    
    rv3129_read_reg(REG_CLOCK_YEAR,&res,1); 
	*year    = bcd2dec(res) + 2000;		// year
}

// set date
void rtc_set_date(BYTE weekday, BYTE day, BYTE month, WORD year)
{
    BYTE res;
    
	res = weekday; // + 1;
    rv3129_write_reg(REG_CLOCK_WDAY,&res,1);
    
	res = dec2bcd(day);
    rv3129_write_reg(REG_CLOCK_DAY,&res,1);
    
	res = dec2bcd(month & 0b00011111);
    rv3129_write_reg(REG_CLOCK_MONTH,&res,1);
    
	res = dec2bcd(year-2000);
    rv3129_write_reg(REG_CLOCK_YEAR,&res,1);
}
	

// initialize RV3129 after a poweron-reset 
void rv3129_init()
{
    BYTE res, clockinit = 0;
    
    rv3129_read_reg(REG_CONTROL_STATUS,&res,1); 
    
    // check if PON flag is set
    if(res & (1<<5))
    {
        res &= ~(1<<5);     // clear PON flag

        // write back modified register value
        rv3129_write_reg(REG_CONTROL_STATUS,&res,1);         
        
        // recommended by datasheet to perform system reset after PowerON
        // send system reset command (set Bit 4 in SysR register)
        DEBUG_puts("\n\rperforming rv3129 System Reset.\n\r");
        res = 0x10;
        rv3129_write_reg(REG_CONTROL_RESET,&res,1);         
              
        clockinit = 1;
    }

    rv3129_read_reg(REG_CONTROL_STATUS,&res,1); 
    
    // check if SR flag is set
    if(res & (1<<4))
    {
        res &= ~(1<<4);     // clear SR flag
        clockinit = 1;
    }

    // write back modified register value
    rv3129_write_reg(REG_CONTROL_STATUS,&res,1); 

    
    if(clockinit == 1)
    {
        // initialize time/date with default values
        DEBUG_puts("\n\rSetting rv3129 default time/date.\n\r");        
        rtc_set_time(21, 30, 0, 1);
        rtc_set_date(1, 4, 9, 2022);    // Sunday, Sep. 4th, 2022
    }
    
}
	
// write to general purpose SRAM
void rtc_write_SRAM(BYTE address, BYTE val)
{
    BYTE res = val;
    rv3129_write_reg(RTC_REG_RAM_BASE + address, &res, 1);
}

// read from general purpose SRAM
BYTE rtc_read_SRAM(BYTE address)
{
    BYTE res;
    rv3129_read_reg(RTC_REG_RAM_BASE + address, &res, 1);
    return res;
}

// write the daylight saving mode config byte
void rtc_writeDaylightSavingMode(BYTE dsm)
{
    rtc_write_SRAM(0x01, dsm);
}

// read the daylight saving mode config byte
BYTE rtc_readDaylightSavingMode()
{
    // read flag variable (only bit 0 & 1)
    return (rtc_read_SRAM(0x01) & 0x03);
}
