// user debug console module
// (C) 2023-09-09 by Daniel Porzig

#include "userConsole.h"
#include "timekeeper.h"
#include "Delay.h"
#include "ValveMotionControl.h"
#include "LEDFade.h"
#include "DeviceControl.h"
#include "FanControl.h"
#include "Bootloader.h"
#include "Config.h"
#include <plib.h>
#include "M24512.h"
#include "sht3x.h"


static char CommandString[255];
static char *comstr_ptr;
static WORD nchars; 
static BYTE n_parms; // number of parameters entered after the command

extern const char FW_VERS_STRING[];     // example: "00TC180215.3";  
extern char txt[256];


typedef struct command_TD
{
   const char   *cmd;
   void   (*func)   (void);
   const char *descr;
   const char *syntax;
}command;

#define MAX_PARMS       100
#define NUM_COMMANDS    13
char *parms[MAX_PARMS];



static void cmd_dumpcfg(void);
static void cmd_getclock(void);
static void cmd_setclock(void);
static void cmd_help(void);
static void cmd_eepromtest();
static void cmd_fwrev();
static void cmd_memdump();
static void cmd_reset();
static void cmd_WDTtest();
static void cmd_motor();
static void cmd_valve();
static void cmd_test();
static void cmd_setupBT();
static void cmd_mvent();
static void cmd_sensor();

// table with valid commands, function pointers and help text
const command commands[] = {
   {"help",      cmd_help, "show help","help [command]"},
   {"dumpcfg",   cmd_dumpcfg, "output config raw data to console","dumpcfg <>"},
   {"getclock",  cmd_getclock, "read and display current time and date","getclock <>"},
   {"setclock",  cmd_setclock, "set current date and time","setclock <hour> <minute> [weekday] [day] [month] [year]"},
   {"eepromtest",cmd_eepromtest, "write data to external EEPROM page, read back and display","eepromtest <page>"},
   {"fwrev",     cmd_fwrev, "read an display firmware revisions (installed and images)","fwrev <>"},
   {"reset",     cmd_reset, "perform system reset","reset <>"},
   {"setupbt",   cmd_setupBT, "set Bluetooth device name and reset system","setupbt <name>"},   
   {"motor",     cmd_motor, "unused","unused"},
   {"valve",     cmd_valve, "actuate or stop valve","valve [direction][speed]"},
   {"test",      cmd_test, "activate valve test mode",""},
   {"mvent",     cmd_mvent, "activate manual venting",""},
   {"sensor",    cmd_sensor, "read and display current temperature and humidity ","sensor <>"},
   
};




// initialize user console variables
void UserConsole_Init()
{
    comstr_ptr = CommandString;
    nchars = 0;
}

// main user console handling task
void UserConsole_Task()
{

    char ch;

    // check user inputs
    while(!UART2_BufIsEmpty())
    {
        ch=UART2_BufRead();

        if(ch == 0x0D)
        {
            *comstr_ptr = '\0';


            // start parsing
            UserConsole_Parse();

            comstr_ptr = CommandString;
            nchars = 0;
        }
        else if(ch == '\b')
        {
            if(nchars > 0)
            {
                // output space and move cursor back
                DEBUG_puts(" \b");
                nchars--;
                comstr_ptr--;
            }
            else
            {
                // buffer empty, do nothing
            }
        }
        else
        {
            if(nchars == 255)
            {
                // maximum buffer length reached, parse line
                *comstr_ptr = '\0';
                // start parsing
                UserConsole_Parse();

                comstr_ptr = CommandString;
                nchars = 0;                
            }
            else
            {
                *comstr_ptr = ch; //tolower(ch);
                comstr_ptr++;
                nchars++;
            }
            
            
        }
    }

}

char *getNextArg(char *inp)
{
    BYTE l;
    l = strlen(inp);
    return inp+l+1;
}


BYTE checkNargs(BYTE nargs)
{
    if(n_parms < nargs)
    {
        sprintf(txt,"\n\rError: %u arguments expected!",nargs);
        DEBUG_puts(txt);
        return 0;
    }
    else
        return 1;
}

// command line parser
BYTE UserConsole_Parse()
{
    char *p;
    WORD len, i,np;
    WORD arg;
    BYTE res;
    float temp;


   // clear the pointer array
   n_parms = 0;
   for (i = 0; i < MAX_PARMS; i++)
      parms[i] = NULL;

    // a command line was completed. Parse input line
    p = CommandString;

    // skip leading whitespaces
    while(*p == ' ')
    {
        p++;
    }

    len=strlen(p);

   // scan the command line, dork any spaces with null chars
   // and save the location of the first char after the null
    for(i=0, np=0; i<len; i++)
    {
        if(p[i]==' ')
        {
            p[i]='\0';
            parms[np++] = &p[i] + 1;
            n_parms++;
        }

    }


    // echo:
    DEBUG_puts("\n\r");
    DEBUG_puts(p);
    DEBUG_puts("\n\r");    


    res = 0;
   // Scan the command table looking for a match
   for (i = 0; i < NUM_COMMANDS; i++)
   {
 
      if (strcmp (commands[i].cmd, (char *)p) == 0)
      {
         commands[i].func();  // command found, run its function
         res = 1;
         break;
      }
   }

    if(!res)
    {
        // invalid command
        DEBUG_puts("\n\rInvalid Command! Type help for command list.");
    }


}


// display help information
static void cmd_help(void)
{
   BYTE i,res;

   if(n_parms == 0)
   {
    DEBUG_puts("\n\rCommand overview: ");

    // Scan the command table looking for a match
    for (i = 0; i < NUM_COMMANDS; i++)
    {
         DEBUG_puts("\n\r");
         sprintf(txt,"%-20s",commands[i].cmd);
         DEBUG_puts(txt);         
         DEBUG_puts(commands[i].descr);
    }
   }
   else
   {
        res = 0;
        // Scan the command table looking for a match
        for (i = 0; i < NUM_COMMANDS; i++)
        {

          if (strcmp (commands[i].cmd, parms[0]) == 0)
          {
            DEBUG_puts("\n\r");
            sprintf(txt,"%-20s",commands[i].cmd);
            DEBUG_puts(txt);
            DEBUG_puts(commands[i].descr);             
            DEBUG_puts("\n\r");
            DEBUG_puts(commands[i].syntax);             

            res = 1;
            break;
          }
        }       
        
        if(res == 0)
        {
            DEBUG_puts("\n\rUnknown Command!\n\r");
        }
   }
       
    DEBUG_puts("\n\r");   

}




// command function ///////////////

// dump the raw configuration data bytes to the console window
static void cmd_dumpcfg(void)
{
    LoadConfig();   // make sure the config data from EEPROM is dumped
    DumpConfig();
}

// perform system reset
static void cmd_reset()
{
    Reset();
}

// activate shutter test mode
static void cmd_test()
{
    WORD i;
    DEBUG_puts("\n\rActivating Testmode. Perform device reset to stop");
    
    DeviceControl_Testmode(0);
}

// configure HM-17 BLE module, program device name
static void cmd_setupBT(void)
{
    UART1PrintString("AT+GAIT1");     // enable high TX gain
    Delayms(250);
    UART1PrintString("AT+GAIN1");     // open RX gain
    Delayms(250);    
    UART1PrintString("AT+POWE7");     // maximum module power
    Delayms(250);        
    
    UART1PrintString("AT+NAME"); 
    UART1PrintString(parms[0]);
    Delayms(250); 
    
    UART1PrintString("AT+RESET");     // reset module
    Delayms(250);       
    
    DEBUG_puts("\n\rReset...");
    Delayms(250);       
    
    Reset();
    
    
}






// basic EEPROM function test: write 64 bytes of data to a specified page address
// and read back written data.
// NOTE: will overwrite any data in the EEPROM page (including configuration data)
static void cmd_eepromtest()
{
	BYTE i;
    BYTE buf[64];
    WORD page;

    page = atol(parms[0]);

    
	for(i=0; i<64; i++)
	{
        buf[i]=64-i;
    }

    if(EEPROM_check_busy() == 0)
    {
        DEBUG_puts("\n\rEEPROM acknowledged.");
    }
    else
        DEBUG_puts("\n\rEEPROM NOT acknowledged.");
    
    
	sprintf(txt,"\n\rWriting data to EEPROM page %u...",page);
    DEBUG_puts(txt);
    
    EEPROM_write(page*EEPROM_getPageSize(), buf, 64);
   
    DEBUG_puts("Done!\n\r");
	
    sprintf(txt,"\n\rReadback from EEPROM page %u:\n\r",page);
    DEBUG_puts(txt);

    EEPROM_read(page*EEPROM_getPageSize(), buf, 64);
    
	for(i=0; i<64; i++)
	{
        sprintf(txt,"%u ",buf[i]);
        DEBUG_puts(txt);
    }

}		




// display firmware version string of (currently executed) firware and bootcode
static void cmd_fwrev(void)
{
    char fwstr[20];
    
    DEBUG_puts("\n\rFIRMWARE REVISIONS:\n\r");    
    DEBUG_puts("\n\r- Main Application (running):  ");    
    DEBUG_puts(FW_VERS_STRING);
    DEBUG_puts("\n\r- Bootcode:                    ");    
    getBootcodeVersionString(fwstr);
    DEBUG_puts(fwstr);
    
    DEBUG_puts("\n\r\n\r");    
  
}


// read temperature and humidity from SHT31
static void cmd_sensor(void)
{
    
    float temperature, humidity;
    BYTE error;
    
    error = SHT3X_ReadMeasurementBuffer(&temperature, &humidity);
//    sht3x_singleshot_measure(&dev, &temperature, &humidity);

    sprintf(txt,"\n\rtemp: %3.1f, humidity: %3.1f %%\n\r", temperature, humidity);
    DEBUG_puts(txt);    
       
}



// define and execute a low-level motor motion pattern
static void cmd_motor(void)
{
    WORD m,d;
    BYTE seq[1000];

    for(m=0; m<n_parms; m++)
    {
        seq[m]=atol(parms[m]);
    }
    
    seq[m+1] = CMD_STOP;
    
    MotorControl_StartSeq(seq);
    
}

// open or close shutter
static void cmd_valve(void)
{
    WORD m,d;

    
	if(n_parms == 0)
	{
		Valve_Stop();
		return;
	}
	else if(n_parms == 1)
	{
	    m = atol(parms[0]);
		d = 63;	// full speed
	}
	else
	{
		m = atol(parms[0]);
		d = atol(parms[1]);


	}

    if(m == 0)
    {
        Valve_Open(d,0);
    }
    else
    {
        Valve_Close(d,0);
    }    
    
}

// activate manual venting mode
static void cmd_mvent(void)
{
    WORD m,d;

    
	if(n_parms == 0)
	{
        // stop manual venting
		DeviceControl_ManualVent(0, 0, 0, 0);
		return;
	}
	else if(n_parms == 1)
	{
	    m = atol(parms[0]);

        DeviceControl_ManualVent(0, FAN_DIR_INWARDS, m, 0);
	}
	else
	{
		m = atol(parms[0]);
		d = atol(parms[1]);

        DeviceControl_ManualVent(0, FAN_DIR_INWARDS, m, d);

	}    
    
}

// read current time and date from RTC IC and display in console
static void cmd_getclock(void)
{
    BYTE hour, min, sec, weekday, day, month;
    WORD year;
    
	rtc_get_time(&hour, &min, &sec);       
    rtc_get_date(&weekday, &day, &month, &year);		

    sprintf(txt,"\n\r - %4u/%02u/%02u - %2u:%02u, Weekday: %u",year,month,day,hour,min,weekday);
    DEBUG_puts(txt);
}

// set time and date, write updated values to RTC IC
static void cmd_setclock(void)
{
    BYTE hour, min, sec, weekday, day, month;
    WORD year;
    
    // Set set-Temperature for selected room
    // Syntax: setClock hour minute wday day month year
    // Syntax: setClock HH MM W DD MM YYYY
    //         wday = 0 -> sunday
    if(checkNargs(2))
    {
       
        if(n_parms == 2)
        {
            hour = (atol(parms[0]));
            min = (atol(parms[1]));
            sec = 0;
            
            
            rtc_set_time(hour, min, sec, 1);       
            rtc_get_time(&hour, &min, &sec);       

            sprintf(txt,"\n\r Time is %2u:%02u",hour,min);
            DEBUG_puts(txt);

        }
        else if(n_parms == 6)
        {
        
            hour = (atol(parms[0]));
            min = (atol(parms[1]));    
            sec = 0;
            weekday = (atol(parms[2]));
            day = (atol(parms[3]));
            month = (atol(parms[4]));
            year = (atol(parms[5]));

            
            rtc_set_time(hour, min, sec, 1);  
            rtc_set_date(weekday, day, month, year);
            
            rtc_get_time(&hour, &min, &sec);       
            rtc_get_date(&weekday, &day, &month, &year);


            sprintf(txt,"\n\r %4u/%02u/%02u ",year,month,day);
            DEBUG_puts(txt);

            sprintf(txt,"- %2u:%02u",hour,min);
            DEBUG_puts(txt);
        }
    }    
    
}





