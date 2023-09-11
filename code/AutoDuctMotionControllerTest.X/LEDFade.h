// LED fading engine and sequence player
// (C) 2023-09-09 by Daniel Porzig

#ifndef _LEDFADE_H_
#define _LEDFADE_H_

#include <stdlib.h>
#include <stdio.h>
#include "HardwareProfile.h"
#include <GenericTypeDefs.h>


void LEDFade_Init();
void LEDFade_Task(void *pvParameters, BYTE *skiprate);
void LEDFade_Fade(BYTE led, WORD delay, BYTE tbright, WORD fspeed);
void LEDFade_SetBrightness(BYTE led, BYTE br);
void LEDFade_SetGlobalBrightness(BYTE led, BYTE gbr);
void LEDFade_SetSequenceDynamicMax(BYTE led, BYTE dynmax);

BYTE LEDFade_getBrightness(BYTE led);
BYTE LEDFade_getTaskState(BYTE led);

typedef struct BTLEDSequence_Struct
{
	BYTE state;
	WORD counter;
    BYTE index;
    BYTE *seq;
    BYTE *seqnext;
    BYTE repcnt;
}BTLEDSequenceData;	

typedef struct LEDFade_Struct
{
	BYTE state;
	BYTE bright, targetbright, globalbright, dynmax;
	WORD fadespeed;
	WORD FadeDelay;
    BTLEDSequenceData Sequence;
}LEDFadeData;	

enum SeqCommands { CMD_SET,     // CMD_SET(brightness)
                   CMD_RAMP,    // CMD_RAMP(brightness,speed)
                   CMD_PAUSE,   // CMD_PAUSE(duration)
                   CMD_STOP,    // CMD_STOP()
                   CMD_REPEAT   // CMD_REPEAT(times)
                 };


extern const BYTE seq_breathing[];
extern const BYTE seq_breathing_fast[];
extern const BYTE seq_breathing_low[];
extern const BYTE seq_sawtooth[];
extern const BYTE seq_toggle_1Hz[];
extern const BYTE seq_toggle_4Hz[];
extern const BYTE seq_toggle_4Hzfadeout[];
extern const BYTE seq_toggle_4Hzfade2half[];
extern const BYTE seq_fade2half[];
extern const BYTE seq_sine30x[];
extern const BYTE seq_flash30x3[];
extern const BYTE seq_flash_low[];
extern const BYTE seq_2xflash_low[];
extern const BYTE seq_setfull[];
extern const BYTE seq_toggle_4Hz2x[];
extern const BYTE seq_breathing_dyn[];
extern const BYTE seq_const_dyn[];


void LEDFade_StartSeq(BYTE led, const BYTE* seq, const BYTE* seqnext);
void LEDFade_Sequence_Task(BYTE led);
void LEDFade_SoftShutdown(BYTE led);
void LEDFade_Disable(BYTE led);

#endif 

