// timekeeper and event scheduling module
// (C) 2023-09-09 by Daniel Porzig

#include "TimeKeeper.h"
#include "Config.h"
#include "TaskScheduler.h"
#include "DeviceControl.h"
#include "RTC_RV3129.h"
#include <time.h>

#define TC_NUM_ROOMS				1 		// numer of rooms to control
#define TC_NUM_EVENTS				4 		// numer of control events per day

BYTE lastEvent_day = 0xFF, lastEvent_ID = 0xFF;       // day and ID of last switching event    
BYTE TimeKeeper_state = 0, TK_minuteschanged = 1;
BYTE MinutesPassed;

TimeStruct gTime;
TimeStruct gLastTime;
DateStruct gDate;
DateStruct gLastDate;


#define NUM_TIMEOUT_EVENTS	7
TimeoutEvent Timeout[NUM_TIMEOUT_EVENTS];





// weekday representation of different devices/OS
// RV3129: weekday (1..7) (1 = Sunday, 7=Saturday)
// Android: weekday (1..7) (1 = Sunday, 7=Saturday)
// PIC RTCC: weekday (0..6) (0 = Sunday, 6 = Saturday)
// Timekeeper: wday (0..6) 0 = Sunday

// representation of year: 
//  - PIC RTCC BCD 0-99


char txt[60];


void TimeKeeper_checkTimeoutEvents(BYTE MinutesPassed);


void puts_weekday(BYTE weekday)
{
    switch(weekday)
    {
        case 1: DEBUG_puts("Sunday"); break;
        case 2: DEBUG_puts("Monday"); break;
        case 3: DEBUG_puts("Tuesday"); break;
        case 4: DEBUG_puts("Wednesday"); break;
        case 5: DEBUG_puts("Thursday"); break;
        case 6: DEBUG_puts("Friday"); break;
        case 7: DEBUG_puts("Saturday"); break;        
        default: DEBUG_puts("-undefined-"); break;        
    }
}



// helper function to sort schedule events
BYTE sortDayEvents(BYTE day, BYTE *firstID, BYTE *lastID)
{
    BYTE idfirst=0xFF, idlast=0xFF, i;
    signed int dist;

    for(i = 0; i<TC_NUM_EVENTS; i++)
    {
        if( (CFGventSched.flagsActive[day] & (1<<i)))
        {

            if(idfirst == 0xFF)
            {
                idfirst = i;
            }
            else
            {

                // calculate time in minutes between current and idfirst event
                dist = (CFGventSched.hour[day][idfirst] - CFGventSched.hour[day][i]) * 60
                          + (CFGventSched.min[day][idfirst] - CFGventSched.min[day][i]);
                
                if( dist >= 0)
                {
                    // current event triggers earlier -> update idfirst
                    idfirst = i;
                }
            }

            if(idlast == 0xFF)
            {
                idlast = i;
            }
            else
            {

                // calculate time in minutes between current and idlast event
                  dist = (CFGventSched.hour[day][idlast] - CFGventSched.hour[day][i]) * 60
                          + (CFGventSched.min[day][idlast] - CFGventSched.min[day][i]);

                if( dist <= 0)
                {
                    // current event triggers later -> update idlast
                    idlast = i;
                }
            }

        }
    }

    if(idlast == 0xFF || idfirst == 0xFF)
    {
        return 0;
    }
    else
    {
        *firstID = idfirst;
        *lastID = idlast;
        return 1;
    }
}



// get IDs of the previous and the next schedule event
BYTE getPrevNextEventToday(BYTE day, BYTE hour, BYTE min, BYTE *prevID, BYTE *nextID, signed int *retdist)
{
    BYTE idprev=0xFF, idnext=0xFF, i;
    signed int nextdist, prevdist, dist;
    char txt[100];

    for(i = 0; i<TC_NUM_EVENTS; i++)
    {
        if( (CFGventSched.flagsActive[day] & (1<<i)))        
        {
            // calculate time in minutes to current event
            dist = (CFGventSched.hour[day][i] - hour) * 60 + (CFGventSched.min[day][i] - min);            

            sprintf(txt,"\n\rday: %u, ID: %u, dist: %i",day,i,dist);
            DEBUG_puts(txt);

            if(dist > 0)
            {
                // next event candidate
                if(idnext == 0xFF)
                {
                    idnext = i;
                    nextdist = dist;
                }
                else
                {
                    if(nextdist > dist)
                    {
                        nextdist = dist;
                        idnext = i;
                    }
                }
            }
            else
            {
                // prev event candidate
                if(idprev == 0xFF)
                {
                    idprev = i;
                    prevdist = dist;
                }
                else
                {
                    if(prevdist < dist)
                    {
                        prevdist = dist;
                        idprev = i;
                    }
                }

            }

        }
    }

    *prevID = idprev;
    *nextID = idnext;
    *retdist = prevdist; // return distance in minutes to previous event

    if(idnext == 0xFF || idprev == 0xFF)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}


// check if a scheduled event is due to be executed
// function is called whenever minutes change
void TimeKeeper_CheckScheduleUpdates(BYTE weekday, BYTE hour, BYTE minute)
{
    BYTE wdtemp;

    BYTE previd, nextid, prevday, junk;
    BYTE fanmode, fandir, fanspeed;
    signed int prevdist;
    char txt[100];

    // Timekeeper: wday (0..6) 0 = Sunday
    // RV3049 / Android: wday (1..7) 1 = Sunday
    // PIC32 RTCC: wday (0..6) 0 = Sunday
    weekday--;
              
    // find last control event and check if current power mode matches
    // schedule information
    getPrevNextEventToday(weekday, hour, minute, &previd, &nextid, &prevdist);

    if(previd == 0xFF)
    {
        // no previous event found for today
        wdtemp = weekday;

        // cycle trough previous days to find last control event
        do
        {
            if(wdtemp>0)
            {
                wdtemp--;
            }
            else
            {
                wdtemp = 6;
            }

            if(sortDayEvents(wdtemp, &junk, &previd))
            {
                // found a day with control events
                prevday = wdtemp;
                break;
            }

        }
        while(wdtemp != weekday);

        if(previd == 0xFF)
        {
            ; // cycled through all days but did not find any control event
              // this case will be handled later
        }


    }
    else
    {
        // previous event found today!
        prevday = weekday;

    }


    // done looking for last switching event
    if(previd == 0xFF)
    {
        DEBUG_puts("\n\rNo Schedule Events defined. ");

    }
    else
    {
        if( (lastEvent_ID == previd) && (lastEvent_day == prevday))
        {
            ; // nothing to do
            //sprintf(txt,"\n\rR: %u, nothing to do.",roomID);
            //DEBUG_puts(txt);
        }
        else
        {
            sprintf(txt,"\n\rnew previd: %u, prevday: %u (today: %u ",previd,prevday,weekday);
            DEBUG_puts(txt);
            puts_weekday(weekday+1);
            DEBUG_puts(")\n\r");

            // save new event id and day
            lastEvent_ID = previd;
            lastEvent_day = prevday;

        
            // check if we are still in activation window of previous event
            prevdist += CFGventSched.duration[prevday][previd];
            if(prevdist > 0)
            {


                fandir = (CFGventSched.fanMode[prevday][previd]) & 0x03;
                if(fandir == 0x02)
                {
                    fanmode = FAN_ALTERNATING;
                    fandir = 0;
                }
                else
                    fanmode = 0;

                fanspeed = (CFGventSched.fanMode[prevday][previd] >> 2) & 0x07;


                DeviceControl_SmartVent(fanmode, fandir, fanspeed, CFGventSched.duration[prevday][previd]);
                DEBUG_puts("Activating scheduled venting event:\n\r");
                sprintf(txt,"fanmode: %u, fandir: %u, fanspeed: %u, duration: %u\n\r",fanmode, fandir, fanspeed, CFGventSched.duration[prevday][previd]);
                DEBUG_puts(txt);            
            }
            else
            {
                DEBUG_puts("Previous event expired.\n\r");
            }
            

        }
    }

}




// reset previous event IDs so schedule events are checked and executed on next call to ..ScheduleUpdates())
void TimeKeeper_ForceScheduleUpdates()
{
    lastEvent_day = 0xFF;
    lastEvent_ID = 0xFF;       // day and ID of last switching event      
    
    TimeKeeper_CheckScheduleUpdates(gDate.weekday,gTime.hour,gTime.min);
}



// initialize timekeeper module and state machine
void TimeKeeper_Init()
{

	TK_minuteschanged = 1;
	TimeKeeper_state = 0;
	MinutesPassed = 0;
    
	// get Time
	rtc_get_time(&gLastTime.hour, &gLastTime.min, &gLastTime.sec);
	gTime.hour = gLastTime.hour;
	gTime.min = gLastTime.min;
	gTime.sec = gLastTime.sec;					

    
	rtc_get_date(&gDate.weekday, &gDate.day, &gDate.month, &gDate.year);
	gLastDate = gDate;
    
	// reset any active Timeout Events
	TimeKeeper_ResetTimeouts();	
    

    if(CFGbase.daylightsavingAuto == 0x01)
    {
		// check rare occasion, if daylight saving time switching must be done
		// if hours just changed, check if we have to adjust current time due to
		// daylight saving mode change
		AdjustDaylightSavingTime();
    }
    	
}


// set time and date for timekeeper module, also update time and date on RTC chip
void TimeKeeper_setTimeDate(BYTE weekday, BYTE day, BYTE month, BYTE year,BYTE hour, BYTE min, BYTE sec, BYTE hourmode, BYTE calibrateRTCC)
{
	TK_minuteschanged = 1;
	TimeKeeper_state = 0;
	MinutesPassed = 0;
	
    gLastTime.hour = hour;
    gLastTime.min = min;
    gLastTime.sec = sec;
    
	gTime.hour = gLastTime.hour;
	gTime.min = gLastTime.min;
	gTime.sec = gLastTime.sec;					

    gDate.day = day;
    gDate.month = month;
    gDate.weekday = weekday;
    gDate.year = year + 2000;
    
	gLastDate = gDate;
    
	rtc_set_time(gLastTime.hour, gLastTime.min, gLastTime.sec, hourmode);
	rtc_set_date(gDate.weekday, gDate.day, gDate.month, gDate.year);	

    sprintf(txt,"\n\r - %4u/%02u/%02u - %2u:%02u, Weekday: %u",year,month,day,hour,min,weekday);
    DEBUG_puts(txt);   

    
	// reset any active Timeout Events
	TimeKeeper_ResetTimeouts();	
	

    // check rare occasion, if daylight saving time switching must be done
    // if hours just changed, check if we have to adjust current time due to
    // daylight saving mode change

    // In this case, update flag variable but DO NOT change current time
    // (override blockbit state)
    CheckDaylightSavingMode(1); 
    
    
    // reset prevday/previd variables and check for schedule updates
    TimeKeeper_ForceScheduleUpdates();    
    
    
}

// read current time and date from RTC chip
void TimeKeeper_getTimeDate(BYTE* weekday, BYTE* day, BYTE* month, BYTE* year, BYTE* hour, BYTE* min, BYTE* sec)
{
    WORD res;
    // read current time from RTC
	rtc_get_time(hour, min, sec);       
    rtc_get_date(weekday, day, month, &res);		
    *year = res - 2000;
    
}

// read current time from RTC, update date as required.
// handle daylight saving mode time adjustment
BYTE TimeKeeper_UpdateTime()
{
	BYTE res = 0;
    
	// save last time
	gLastTime.hour = gTime.hour;
	gLastTime.min = gTime.min;
	gLastTime.sec = gTime.sec;					

	// read current time from RTC
	rtc_get_time(&gTime.hour, &gTime.min, &gTime.sec);
           
	
	// check if hours changed
	if((gTime.hour != gLastTime.hour))
	{
		// in case hours wrapped around, read current Date too
		if(gTime.hour == 0)
		{
			rtc_get_date(&gDate.weekday, &gDate.day, &gDate.month, &gDate.year);		
		}	
		
        if(CFGbase.daylightsavingAuto == 0x01)
        {
            // check rare occasion, if daylight saving time switching must be done
            // if hours just changed, check if we have to adjust current time due to
            // daylight saving mode change
            res = AdjustDaylightSavingTime();
        }
	}	
	    
	return res;
}	

// get time from timekeeper data structure
void TimeKeeper_getTime(BYTE *h, BYTE *m, BYTE *s)
{
	*h = gTime.hour;
	*m = gTime.min;
	*s = gTime.sec;	

}


// calculates the minutes passed between the two given times (current / last)
// It is assumed that the difference is less or equal 60 minutes!
static BYTE GetPassedMinutes(TimeStruct *current, TimeStruct *last)
{
	BYTE res;
	
	if(current->min < last->min)
	{
		// hour seems to have changed
		res = current->min + 60 - last->min;
	}
	else
	{
		res = current->min - last->min;
	}
	return res;
}			

// calculate number of passed minutes between last two Clock Updates
BYTE GetPassedMinutesCurrent()
{
	return GetPassedMinutes(&gTime, &gLastTime);
}	

// main timekeeper state machine task
void TimeKeeper_Task(void *pvParameters, BYTE *skiprate)
{
	BYTE i, ms;

    
	switch(TimeKeeper_state)
	{
		case 0:	// time check

			// get current time
			TimeKeeper_UpdateTime();

			// calculate how many minutes have passed since last TimeUpdate
			MinutesPassed = GetPassedMinutes(&gTime,&gLastTime);            
            if(MinutesPassed > 0)
            {
                sprintf(txt,"\n\r -- %02u:%02u",gTime.hour, gTime.min);
                DEBUG_puts(txt);
                
                // check if any scheduled venting events need to be activated
                if(DeviceControl_GetMode() == DEVICEMODE_SMART)
                {
                    TimeKeeper_CheckScheduleUpdates(gDate.weekday,gTime.hour,gTime.min);    
                }

                // check if configuration has changed within the last 10 minutes and need to be saved to external EEPROM
                if((gTime.min % 10 ) == 0)
                {
                    SaveConfig_UdatesOnly();
                }
            }
            
            
			TimeKeeper_state = 1;
			
			*skiprate = 2;
		break;
		case 1:
			// Request HeaderBar Update in case minutes and/or hours have changed since last timecheck
			if((gTime.hour != gLastTime.hour) || (gTime.min != gLastTime.min))
			{
				TK_minuteschanged = MinutesPassed;	
			}
			else
			{
				// Minutes did not change since last Task Execution. Reset Flag
				TK_minuteschanged = 0;
			}
				
			TimeKeeper_state = 2;
			*skiprate = 2;		
				
		break;
		case 2: // Wait State/Process Timeout Events -> Time check		
		
            TimeKeeper_checkTimeoutEvents(MinutesPassed);
			TimeKeeper_state = 0;
			*skiprate = 250;			
		break;
	}	
}

BYTE TimeKeeper_needsTimeUpdate()
{
	// Did time change so we need to update time display?
	return ((gTime.hour != gLastTime.hour) || (gTime.min != gLastTime.min));
}	

// returns number of Minutes that have passed since last call to TimeKeeper Task
// NOTE: calling this function will clear the minutechanges var
BYTE TimeKeeper_getMinuteOffset()
{
	BYTE res;
	res = TK_minuteschanged;
	TK_minuteschanged = 0;
	return res;
}	


// reset ALL timeout events to inactive state
void TimeKeeper_ResetTimeouts()
{
	BYTE i;
	
	for(i=0;i<NUM_TIMEOUT_EVENTS;i++)
	{
		Timeout[i].Active = 0;
	}				
}				

void TimeKeeper_ResetTimeout(BYTE tnum)
{
    Timeout[tnum].Active = 0;
}

void TimeKeeper_SetupTimeout(BYTE tnum, BYTE Minutes, VoidFnct Task)
{
	if(Minutes == 0)
	{
		Timeout[tnum].Active = 0;
	}
	else
	{
		Timeout[tnum].Active = 1;
		Timeout[tnum].TimedOut = 0;
		Timeout[tnum].MinutesLeft = Minutes;
        Timeout[tnum].Task = Task;
		
	}
}

BYTE TimeKeeper_TimeoutOver(BYTE tnum)
{
	if(	Timeout[tnum].Active )
	{
		return Timeout[tnum].TimedOut;
	}
	else
		return 0;
}


void TimeKeeper_GetTimeoutStats(BYTE tnum, BYTE *state, BYTE *minutes)
{
    *state = Timeout[tnum].Active;
    *minutes = Timeout[tnum].MinutesLeft;
}


void TimeKeeper_checkTimeoutEvents(BYTE MinutesPassed)
{
    BYTE i;
    // Adjust Timeout Structures only if Time has changed since last task call
    
    if(MinutesPassed == 0)
        return;
	               
    
    for(i=0;i<NUM_TIMEOUT_EVENTS;i++)
    {
        if(Timeout[i].Active)
        {
            if(MinutesPassed >= Timeout[i].MinutesLeft)
            {
                // Set Timed-Out-Flag
                Timeout[i].TimedOut = 1;	
                
                // execute Task
                if(Timeout[i].Task != NULL)
                {
                    DEBUG_puts("\n\rcalling timeout task.\n\r");
                    Timeout[i].Active = 0;  // needs to be set prior to calling the task, otherwise recurring calls won't work
                    
                    Timeout[i].Task();
                    
                }
                    
            }	
            else
            {
                Timeout[i].MinutesLeft -= MinutesPassed;
            }	
        }
    }				    
}



// +------------------------------------------------------+
// | Automatic Daylight Saving Time Switching             |
// +------------------------------------------------------+
// Rules:
// - last Sunday in March -> switch from 2h to 3h (start of DaylightSaving phase)
// - last Sunday in October -> switch from 3h to 2h (end of    "             "  )




// check current time and date configuration
// + enables correct daylight saving time mode
// + adjustes current time if necessary
// returns:
// 0: nothing changed
// 1: daylight saving mode disabled, hours decreased
// 2: daylight saving mode enabled, hours increased
//
// When to call:
// + on every boot (after TimeKeeper_Init())
// + everytime hours change
BYTE AdjustDaylightSavingTime()
{
	BYTE res;
    rtccTime    tm1;            // time structure

    

	res = CheckDaylightSavingMode(0);
	switch(res)
	{
		case CDSM_NOCHANGE:
			return 0;
		break;
		case CDSM_DISABLED_NOW:
		case CDSM_ERROR_DISABLED_NOW:
	
			// End of Daylight Saving phase detected.
			// Change System Time (decrease hours)
			if(gTime.hour > 0)
			{
				gTime.hour--;
			}
			else
			{
				gTime.hour = 23;
			}		
			// set Time         
			rtc_set_time(gTime.hour, gTime.min, gTime.sec, 1);
			
			DEBUG_puts("\n\rDaylight Saving disabled. Hours decreased.");
			
			return 1;
		break;
		
		case CDSM_ENABLED_NOW:
		case CDSM_ERROR_ENABLED_NOW:
				
			// Begin of Daylight Saving phase detected.
			// Change System Time (increase hours)
			if(gTime.hour < 23)
			{
				gTime.hour++;
			}
			else
			{
				gTime.hour = 0;
			}		
			// set Time

			rtc_set_time(gTime.hour, gTime.min, gTime.sec, 1);
            
			DEBUG_puts("\n\rDaylight Saving enabled. Hours increased.");
					
			return 2;
		break;
		
	}	
	
	return 0;
}	
	


#define DAYLIGHT_SAVING_DEBUG



// check current time and date configuration and enable / disable daylight saving time
// flag if necessary
// if BBoverride == 1, state of blockbit is ignored / cleared
// returns:
// 0 	- no change of time or mode 
// 3 	- Daylight saving mode changed, hours must be decreased
// 4 	- Daylight saving mode changed, hours must be increased
// 0xF3 - Daylight saving mode was not set correctly, should be disabled
// 0xF4 - Daylight saving mode was not set correctly, should be enabled
BYTE CheckDaylightSavingMode(BYTE BBoverride)
{
	BYTE adelta, bdelta, DaylightSaving, res = 0, Blockbit, bbittemp, wdtemp;

    
    // RV3049 / Android: wday (1..7) 1 = Sunday
    // wdtemp: 1 = Monday ... 7 = Sunday
    wdtemp = gDate.weekday + 6;
    if(wdtemp > 7)
        wdtemp -= 7;
             
    
    
	// NOTE:
	//  Blockbit function: if daylight saving mode is disabled in october,
	// blockbit is set for the duration of at least one hour to signal that 
	// mode switching was already performed.

	// read current state from RTC Memory
	DaylightSaving = rtc_readDaylightSavingMode();
	
	if(BBoverride)
	{
		Blockbit = 0;
		bbittemp = 0;
	}
	else
	{	
		// get blockbit (bit1)
		Blockbit = (DaylightSaving & 0x02) >> 1;
		// backup current Blockbit state
		bbittemp = Blockbit;
	}	
	
	// remove blockbit
	DaylightSaving &= 0x01;

	#ifdef DAYLIGHT_SAVING_DEBUG
		if(DaylightSaving == 1)
			DEBUG_puts("\n\rSRAM: 1");
		else 
			DEBUG_puts("\n\rSRAM: 0");

		if(Blockbit)
			DEBUG_puts("\n\rSRAM: special blockbit set");
				


        sprintf(txt,"\n\rwdtemp: %u",wdtemp);
        DEBUG_puts(txt);

	
	#endif


	// clock currently is set to daylight saving time.
	// check if it has to be disabled.
	
	// check month first
	if(gDate.month > 10 || gDate.month < 3)
	{
		// check which daylight saving mode we are currently in
		if(DaylightSaving == 1)
		{
			// daylight saving mode not correct.				
			// Daylight saving time must be switched off during november - february
			DaylightSaving = 0;
		
			#ifdef DAYLIGHT_SAVING_DEBUG
				DEBUG_puts("\n\rDSD#1: DaylightSaving disabled.");
			#endif
		
			// set mode change flag	
			res = 0xF3;
		}
		else
		{
			
			#ifdef DAYLIGHT_SAVING_DEBUG
				DEBUG_puts("\n\rDSD: No change #1");
			#endif
			
			
			// daylight saving mode seems to be correct
			res = 0;
		}		
		
		// clear blockbit
		Blockbit = 0;
	}
	else if(gDate.month == 10)
	{
		// october: take a closer look
        // RV3049 / Android: wday (1..7) 1 = Sunday      
        
		adelta = 7 - wdtemp;	// days remaining until sunday
		bdelta = 31 - gDate.day;	// days remaining until end of month

		if(bdelta <= 6)
		{
			// daylight switching happens today or lies in the past				
			if( adelta == 0)
			{
				// today is sunday, switching day! :-)
				// take a closer look at the time
                
                #ifdef DAYLIGHT_SAVING_DEBUG
                    DEBUG_puts("\n\rDSD: Today is switch day!");
                #endif                

				// im Herbst folgt auf die Sekunde 02:59:59 A MESZ die Sekunde 02:00:00 B MEZ.
				
				if(gTime.hour < 3)
				{
					// switching moment lies in the FUTURE. 
					
					// check which daylight saving mode we are currently in
					if(DaylightSaving == 1)
					{					
						
						#ifdef DAYLIGHT_SAVING_DEBUG
							DEBUG_puts("\n\rDSD: No change #2");
						#endif
						
						// no changes made
						res = 0;						
					}
					else
					{
						if(!Blockbit)
						{
							// Blockbit not set.
							// daylight saving mode not correct.				
							// Daylight saving should still be switched ON
							DaylightSaving = 1;
						
							#ifdef DAYLIGHT_SAVING_DEBUG
								DEBUG_puts("\n\rDSD#2: DaylightSaving enabled.");
							#endif
											
							// set mode change flag	
							res = 0xF4;
							
							// set blockbit flag to avoid reenabling 
							Blockbit = 1;
						}
						else
						{
							// Blockbit set, daylight saving mode was just disabled
							#ifdef DAYLIGHT_SAVING_DEBUG
								DEBUG_puts("\n\rDSD: No change #2 (B)");
							#endif
							
							// no changes made
							res = 0;						
						}
								
								
					}
						
				}
				else
				{	
					// switching moment is NOW or lies in the PAST
										
					// check which daylight saving mode we are currently in
					if(DaylightSaving == 1)
					{						
						// we are still in Daylight-Saving mode and the moment to switch already lies in the PAST or is NOW.
						// Perform regular switching
						// -> Disable Daylight saving mode
						// -> update hour
						DaylightSaving = 0;
						
						#ifdef DAYLIGHT_SAVING_DEBUG
							DEBUG_puts("\n\rDSD#3: DaylightSaving disabled.");
						#endif
						
						// set mode change and hour update flag
						res = 3;
						
						// set blockbit flag to avoid reenabling 
						Blockbit = 1;
					}
					else
					{
						// Daylight saving mode is disabled as it should be.
						
						#ifdef DAYLIGHT_SAVING_DEBUG
							DEBUG_puts("\n\rDSD: No change #3");
						#endif
						
						// no changes made
						res = 0;
						
						// Blockbit can be disabled now
						Blockbit = 0;
					}		
				}	
				
				
			}
			else
			{
				// check if switching date lies in the future or in the past
				if( bdelta >= adelta)
				{
					// switching time lies in the FUTURE
					// DaylightSaving should still be == 1 

					// check which daylight saving mode we are currently in
					if(DaylightSaving == 1)
					{						
						
						#ifdef DAYLIGHT_SAVING_DEBUG
							DEBUG_puts("\n\rDSD: No change #4");
						#endif
						
						// no changes made
						res = 0;
					}
					else
					{
						// daylight saving mode not correct.				
						// Daylight saving should still be switched ON
						DaylightSaving = 1;
					
						#ifdef DAYLIGHT_SAVING_DEBUG
							DEBUG_puts("\n\rDSD#4: DaylightSaving enabled.");
						#endif
					
						// set mode change flag	
						res = 0xF4;
					}

				}
				else
				{
					// DaylightSaving mode should be disabled by now.
					
					// check which daylight saving mode we are currently in
					if(DaylightSaving == 1)
					{						
						// daylight saving mode not correct.				
						// Daylight saving should already be switched OFF
						
						DaylightSaving = 0;
						
						#ifdef DAYLIGHT_SAVING_DEBUG
							DEBUG_puts("\n\rDSD#5: DaylightSaving disabled.");
						#endif
												
						res = 0xF3;	
					}
					else
					{
						
						#ifdef DAYLIGHT_SAVING_DEBUG
							DEBUG_puts("\n\rDSD: No change #5");
						#endif
						
						
						// no changes made
						res = 0;
					}		
				}	

				// clear blockbit
				Blockbit = 0;	
			
			}	
	
		}
		else
		{
			// daylight switching day lies in the future

			// check which daylight saving mode we are currently in
			if(DaylightSaving == 1)
			{					
				
				#ifdef DAYLIGHT_SAVING_DEBUG
					DEBUG_puts("\n\rDSD: No change #6");
				#endif
				
				// no changes made
				res = 0;						
			}
			else
			{
				// daylight saving mode not correct.				
				// Daylight saving should still be switched ON
				DaylightSaving = 1;
				
				#ifdef DAYLIGHT_SAVING_DEBUG
					DEBUG_puts("\n\rDSD#6: DaylightSaving enabled.");
				#endif
							
				// set mode change flag	
				res = 0xF4;
			}

			// clear blockbit
			Blockbit = 0;

		}	
	}
	else if(gDate.month == 3)
	{
		// march: take a closer look
		
		adelta = 7 - wdtemp;	// days remaining until sunday
		bdelta = 31 - gDate.day;	// days remaining until end of month

		if(bdelta <= 6)
		{
			// daylight switching happens today or lies in the past		
			if( adelta == 0)
			{
				// today is sunday, switching day! :-)
				// take a closer look at the time

				// Bei der Umstellung im Frühjahr folgt auf die Sekunde 01:59:59 MEZ die Sekunde 03:00:00 MESZ; 
                #ifdef DAYLIGHT_SAVING_DEBUG
                    DEBUG_puts("\n\rDSD: today is switchday!");
                #endif				
                
                
				if(gTime.hour < 2)
				{
					// switching moment lies in the future. 
					
					// check which daylight saving mode we are currently in
					if(DaylightSaving == 0)
					{					
						
						#ifdef DAYLIGHT_SAVING_DEBUG
							DEBUG_puts("\n\rDSD: No change #7");
						#endif
						
						
						// no changes made
						res = 0;						
					}
					else
					{
						// daylight saving mode not correct.				
						// Daylight saving should still be switched OFF
						DaylightSaving = 0;
					
						#ifdef DAYLIGHT_SAVING_DEBUG
							DEBUG_puts("\n\rDSD#7: DaylightSaving disabled.");
						#endif
					
					
						// set mode change flag	
						res = 0xF3;
					}
					
				}
				else
				{
					// check which daylight saving mode we are currently in
					if(DaylightSaving == 0)
					{						
						// we are not yet in Daylight-Saving mode and the moment to switch is NOW already lies in the PAST.
						// -> Enable Daylight saving mode
						// -> update hour
						DaylightSaving = 1;
												
						#ifdef DAYLIGHT_SAVING_DEBUG
							DEBUG_puts("\n\rDSD#8: DaylightSaving enabled.");
						#endif
																								
						// set mode change and hour update flag
						res = 4;
					}
					else
					{
						// Daylight saving mode is enabled as it should be.
						
						#ifdef DAYLIGHT_SAVING_DEBUG
							DEBUG_puts("\n\rDSD: No change #8");
						#endif
						
						
						// no changes made
						res = 0;
					}		

				}	
				
				
			}
			else
			{
				// check if switching date lies in the future or in the past
				if( bdelta >= adelta)
				{
					// not yet time to switch
					// DaylightSaving should still be == 0 
					
					// check which daylight saving mode we are currently in
					if(DaylightSaving == 0)
					{					
						
						#ifdef DAYLIGHT_SAVING_DEBUG
							DEBUG_puts("\n\rDSD: No change #9");
						#endif
							
						// no changes made
						res = 0;
					}
					else
					{
						// daylight saving mode not correct.				
						// Daylight saving should still be switched OFF
						DaylightSaving = 0;
					
						#ifdef DAYLIGHT_SAVING_DEBUG
							DEBUG_puts("\n\rDSD#9: DaylightSaving disabled.");
						#endif
										
						// set mode change flag	
						res = 0xF3;
					}

				}
				else
				{
					// DaylightSaving mode should be enabled by now.
					
					// check which daylight saving mode we are currently in
					if(DaylightSaving == 0)
					{						
						// daylight saving mode not correct.				
						// Daylight saving should already be switched OFF
						
						DaylightSaving = 1;
						
						#ifdef DAYLIGHT_SAVING_DEBUG
							DEBUG_puts("\n\rDSD#10: DaylightSaving enabled.");
						#endif
						
						res = 0xF4;	
					}
					else
					{
						#ifdef DAYLIGHT_SAVING_DEBUG
							DEBUG_puts("\n\rDSD: No change #10");
						#endif
						
						
						// no changes made
						res = 0;
					}		
				}	
			}	
	
		}
		else
		{
			// daylight switching day lies in the FUTURE

			// check which daylight saving mode we are currently in
			if(DaylightSaving == 0)
			{					
				
				#ifdef DAYLIGHT_SAVING_DEBUG
					DEBUG_puts("\n\rDSD: No change #11");
				#endif
				
				
				// no changes made
				res = 0;						
			}
			else
			{
				// daylight saving mode not correct.				
				// Daylight saving should still be switched OFF
				DaylightSaving = 0;
				
				#ifdef DAYLIGHT_SAVING_DEBUG
					DEBUG_puts("\n\rDSD#11: DaylightSaving disabled.");
				#endif
				
			
				// set mode change flag	
				res = 0xF3;
			}
		}
		
		// clear blockbit. Not needed for mode switching in March
		Blockbit = 0;

	}
	else
	{
		// month > 4 && < 10: daylight saving time
		
		// check which daylight saving mode we are currently in
		if(DaylightSaving == 0)
		{
			// daylight saving mode not correct.				
			// Daylight saving time must be switched off during november - february
			DaylightSaving = 1;
		
			#ifdef DAYLIGHT_SAVING_DEBUG
				DEBUG_puts("\n\rDSD#12: DaylightSaving enabled.");
			#endif
		
		
			// set mode change flag	
			res = 0xF4;
		}
		else
		{
			
			#ifdef DAYLIGHT_SAVING_DEBUG
				DEBUG_puts("\n\rDSD: No change #12");
			#endif
			
			// daylight saving mode seems to be correct
			res = 0;
		}
		
		// clear blockbit
		Blockbit = 0;
		
	}
	
	if(res || (bbittemp != Blockbit))
	{
		
		#ifdef DAYLIGHT_SAVING_DEBUG
			if(DaylightSaving)
				DEBUG_puts("\n\rwriting SRAM: 1");
			else
				DEBUG_puts("\n\rwriting SRAM: 0");
				
			if(Blockbit)
				DEBUG_puts("\n\rSRAM: writing special blockbit");
				
		#endif
		
		// add state of blockbit
		DaylightSaving |= (Blockbit << 1);
		
		// Daylight Saving Mode changed.
		// Write new Flag state to RTC SRAM
		rtc_writeDaylightSavingMode(DaylightSaving);
	}

	// return result
	return res;	

}





// calculate the time in minutes until the moment where the daylight saving time is changed
//	return:
//   0 - daylight saving switchover will happen in > 1 week, minutes returned = 0xFFFF;
//   1 - last Sunday in March is in < 1 week, calculated minutes remaining will be returned
//   2 - last Sunday in October is in < 1 week, calculated minutes remaining will be returned
BYTE GetTimeTillDaylightSwitch(WORD *timerem)
{
	BYTE bdelta, lastSun, daysremaining, wdtemp;
	WORD timetillswitch, tnow;
	
	#ifdef DAYLIGHT_SAVING_DEBUG
		BYTE dbg[50];
	#endif
	
	
    // RV3049 / Android: wday (1..7) 1 = Sunday
    // wdtemp: 1 = Monday ... 7 = Sunday
    // PIC RTCC: 0 = Sunday
    wdtemp = gDate.weekday + 6;
    if(wdtemp > 7)
        wdtemp -= 7;        
	
// Rules:
// - last Sunday in March -> switch from 2h to 3h (start of DaylightSaving phase)
// - last Sunday in October -> switch from 3h to 2h (end of    "             "  )


	if(gDate.month == 10)
	{
		// october: take a closer look
		bdelta = 31 - gDate.day;	// days remaining until end of month

		// calculate Weekday of 31th October
		// (example: Friday, 16th October -> bdelta = 15 means 31th October is in 2 weeks + 1 day -> 31th October is a Saturday)
		
		// calculate last Sunday in October
		// lastSun = 31 - Wd31;
		lastSun = 31 - ( (bdelta - 7 + wdtemp) % 7);

		
		if(lastSun < gDate.day)
		{
			// switching day already over
			*timerem = 0xFFFF;
			
			#ifdef DAYLIGHT_SAVING_DEBUG
				DEBUG_puts("\n\rGTTDS#1: DS ignored.");
			#endif
			
			return 0;
		}
		else
		{
			daysremaining = lastSun - gDate.day;
			
			// calc current time minutes			
			tnow = gTime.hour * 60 + gTime.min;
				
			// tswitch = 180;	// 3h -> 2h (3 * 60)
				
			if(daysremaining == 0 && (180 < tnow))
			{
				// switching moment was today and is already over
				
				// TODO:
				// + special case: setting up alarm during shifting-hour
				
				*timerem = 0xFFFF;
				
			#ifdef DAYLIGHT_SAVING_DEBUG
				DEBUG_puts("\n\rGTTDS#2: DS just over.");
			#endif
				
				
				return 0;
			}
			else
			{
							
					
				// calculate minutes until daylight switching
				timetillswitch = daysremaining * 1440 + 180;
				timetillswitch -= tnow;
				
				// return remaining minutes and October switching ID
				*timerem = timetillswitch;
				
			#ifdef DAYLIGHT_SAVING_DEBUG
				sprintf(dbg,"\n\rGTTDS#3: tts: %u min",timetillswitch);
				DEBUG_puts(dbg);
			#endif
				
				
				return 2;
			}	
			
		}	
	}
	else if(gDate.month == 3)
	{
		// March: take a closer look
		bdelta = 31 - gDate.day;	// days remaining until end of month

		// calculate Weekday of 31th March
		// Wd31 = gDate.weekday + (bdelta % 7);
		
		// calculate last Sunday in March
		lastSun = 31 - ( (bdelta - 7 + wdtemp) % 7);       

		#ifdef DAYLIGHT_SAVING_DEBUG
			sprintf(dbg,"\n\rGTTDS#6: lS:%u,WD:%u,bd:%u",lastSun,gDate.weekday,bdelta);
			DEBUG_puts(dbg);
		#endif


		if(lastSun < gDate.day)
		{
			// switching day already over
			*timerem = 0xFFFF;
			
			#ifdef DAYLIGHT_SAVING_DEBUG
				DEBUG_puts("\n\rGTTDS#4: DS ignored.");
			#endif
			
			
			return 0;
		}
		else
		{
			daysremaining = lastSun - gDate.day;
			
			// calc current time minutes			
			tnow = gTime.hour * 60 + gTime.min;
				
			// tswitch = 120;	// 2h -> 3h (2 * 60)
				
			if(daysremaining == 0 && (120 < tnow))
			{
				// switching moment was today and is already over
				
				// TODO:
				// + special case: setting up alarm during shifting-hour
				
				*timerem = 0xFFFF;
				
			#ifdef DAYLIGHT_SAVING_DEBUG
				DEBUG_puts("\n\rGTTDS#5: DS just over.");
			#endif
				
				return 0;
			}
			else
			{
				// calculate minutes until daylight switching
				timetillswitch = daysremaining * 1440 + 120;
				timetillswitch -= tnow;
				
				// return remaining minutes and March switching ID
				*timerem = timetillswitch;
				
			#ifdef DAYLIGHT_SAVING_DEBUG
				sprintf(dbg,"\n\rGTTDS#6: tts: %u min",timetillswitch);
				DEBUG_puts(dbg);
			#endif
				
				return 1;
			}	
			
		}	
	}
	else	
	{
		// daylight saving time will not be changed within this month
		*timerem = 0xFFFF;
		
		#ifdef DAYLIGHT_SAVING_DEBUG
			DEBUG_puts("\n\rGTTDS#7: DS ignored.");
		#endif
		
		return 0;
	}
}

