#ifndef APP_TIMER_H
#define	APP_TIMER_H

#include <xc.h>
#include <stdbool.h>

typedef union
{
    uint8_t TicksBitMap;
    struct
    {
        unsigned mSecTick           : 1;
        unsigned mSecTenTick        : 1;
        unsigned mSecHundredTick    : 1;
        unsigned SecTick            : 1;
        unsigned MinTick            : 1;
        unsigned HourTick           : 1;
        unsigned HalfDayTick        : 1;
        unsigned DayTick            : 1;
    };
}appTimerHandlerBit_t;
appTimerHandlerBit_t appTimer;

void TMRapp_Tick(void);
void TMRapp_Handler(void);
void TMRapp_ClearBitFlags(void);

#endif	/* APP_TIMER_H */

