
#include <xc.h>
#include <stdint.h>

#include "buttons.h"
#include "HardwareProfile.h"

#define PUSH_BUTTON_SHORT   55
#define PUSH_BUTTON_RELEASE  20

/* Service PushButton
 * Called at a 1mS rate.  Times the length of a button press and sets the appropriate bit.
 */
buttonStates_t  service_pushbutton()
{
    static uint32_t  PB_Timer = 0;
    static uint8_t   Release_Timer = 0;
    buttonStates_t bs = NoButton;

    if ((SWITCH_1_PORT == 0) || (SWITCH_2_PORT == 0) ) // pushed
    {  // run 1mS timer when pushbutton is pressed
        ++PB_Timer;
        
        // Determine if press was invalid (switch bounce), a short press or a long press.
            // Check for jitter
            if (PB_Timer == PUSH_BUTTON_SHORT)
            {
                Release_Timer = 0;
                // short button press detected
                if (SWITCH_1_PORT == 0)
                {
                    bs = ShortPress1;
                }
                else
                {
                    bs = ShortPress2;
                }
            }
            else
            {
                bs = NoButton;
            }
    }
    else // released
    {
        if (Release_Timer == PUSH_BUTTON_RELEASE)
        {
            PB_Timer = 0;
            Release_Timer = 0;
        }
        else
        {
            Release_Timer++;
        }
        bs = Released;
    }

    return bs;
}
