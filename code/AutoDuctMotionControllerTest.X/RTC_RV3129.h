// RV3129-C3 Real Time Clock Module Driver
// (C) 2023-09-09 by Daniel Porzig

#include "HardwareProfile.h"
#include "GenericTypeDefs.h"
#include "Compiler.h"
#include <stdio.h>
#include <stdlib.h>

#define bcd2dec(bcd)	(((((bcd)>>4) & 0x0F) * 10) + ((bcd) & 0x0F)) 
#define dec2bcd(dec)	((((dec)/10)<<4)|((dec)%10)) 


void rtc_get_time(BYTE* hour, BYTE* min, BYTE* sec);
void rtc_set_time(BYTE hour, BYTE min, BYTE sec, BYTE hourmode);
void rtc_get_date(BYTE* weekday, BYTE* day, BYTE* month, WORD* year);
void rtc_set_date(BYTE weekday, BYTE day, BYTE month, WORD year);


void rtc_writeDaylightSavingMode(BYTE dsm);
BYTE rtc_readDaylightSavingMode();
void rv3129_init();