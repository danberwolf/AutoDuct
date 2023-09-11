
#include "circbuffer.h"


void CircBufferInit(CircBuffer *CB)
{
    CB->count = 0;
    CB->read = CB->buffer;
    CB->write = CB->buffer;
}

BYTE CircBufferIsEmpty(CircBuffer *CB)
{
    return (CB->count == 0);
}

BYTE CircBufferIsFull(CircBuffer *CB)
{
    return (CB->count == CIRC_BUFFER_SIZE);
}

BYTE CircBufferGetCount(CircBuffer *CB)
{
    return (CB->count);
}


void CircBufferWrite(CircBuffer *CB, BYTE val)
{
    // store value in ring buffer
    *CB->write = val;
    // move write pointer
    CB->write++;
    // check write pointer range
    if(CB->write >= (CB->buffer + CIRC_BUFFER_SIZE))
    {
        // wrap write pointer around
        CB->write = CB->buffer;
    }
    // increase element count
    CB->count++;
}

BYTE CircBufferRead(CircBuffer *CB)
{
    BYTE t;
    // get value from ring buffer
    t = *CB->read;
    // move read pointer
    CB->read++;
    // check write pointer range
    if(CB->read >= (CB->buffer + CIRC_BUFFER_SIZE))
    {
        // wrap write pointer around
        CB->read = CB->buffer;
    }
    // decrease element count
    CB->count--;
    // return ring buffer content
    return t;
}


