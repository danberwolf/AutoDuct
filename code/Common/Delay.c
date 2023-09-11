#include "HardwareProfile.h"
#include <plib.h>
#include "Delay.h"


void Delay10us(DWORD dwCount)
{
	volatile DWORD _dcnt;

	_dcnt = dwCount*((DWORD)(0.00001/(1.0/GetInstructionClock())/10));
	while(_dcnt--)
	{
        Nop();
        Nop();
        Nop();
	}
}
    
void Delayms(WORD wCount)
{
	volatile WORD _dcnt;

	_dcnt = wCount;
	while(_dcnt--)
	{
        Delay10us(100);
	}
}
