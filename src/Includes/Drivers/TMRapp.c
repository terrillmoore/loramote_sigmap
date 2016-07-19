
#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include "TMRapp.h"

static uint16_t   timerMsTickCount = 0;
static uint16_t   timerMinTickCount = 0;

void TMRapp_Tick(void)
{
    // Handle Bit flags
    appTimer.mSecTick = true;       // Single Tick
}

void TMRapp_ClearBitFlags(void)
{
    // Clear all bit Flags
    appTimer.TicksBitMap = 0x00;
}

void TMRapp_Handler(void)
{
    // Increment Counter
    timerMsTickCount++;
    // Handle Tick Increments
    if ( (timerMsTickCount % 10) == 0)
    {
        appTimer.mSecTenTick = true;
        if ( (timerMsTickCount % 100) == 0)
        {
            appTimer.mSecHundredTick = true;
            if ( (timerMsTickCount % 1000) == 0)
            {
                appTimer.SecTick = true;
                if ( (timerMsTickCount % 60000) == 0)
                {
                    timerMinTickCount++;
                    appTimer.MinTick = true;
                    if ( (timerMinTickCount % 60) == 0)
                    {
                        appTimer.HourTick = true;
                        if ( (timerMinTickCount % 720) == 0)
                        {
                            appTimer.HalfDayTick = true;
                            if ( (timerMinTickCount % 1440) == 0)
                            {
                                appTimer.DayTick = true;
                                // Reset Counters
                                timerMsTickCount = 0;
                                timerMinTickCount = 0;
                            }
                        }
                    }
                }
            }
        }
    }
}