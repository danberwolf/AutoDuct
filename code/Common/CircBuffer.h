#ifndef __CIRCBUFFER_H_
#define __CIRCBUFFER_H_

#include <sys/attribs.h>
#include <plib.h>
#include "Compiler.h"
#include "HardwareProfile.h"
#include "uart2.h"

// UART RingBuffer

#define CIRC_BUFFER_SIZE    256

typedef struct CircBuffer_TD
{
    BYTE buffer[CIRC_BUFFER_SIZE];
    BYTE *read;
    BYTE *write;
    BYTE count;
}CircBuffer;


void CircBufferInit(CircBuffer *CB);
BYTE CircBufferIsEmpty(CircBuffer *CB);
BYTE CircBufferIsFull(CircBuffer *CB);
BYTE CircBufferGetCount(CircBuffer *CB);
void CircBufferWrite(CircBuffer *CB, BYTE val);
BYTE CircBufferRead(CircBuffer *CB);


#endif



