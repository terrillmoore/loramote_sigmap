/* 
 * File:   io.h
 * Author: joe
 *
 * Created on October 28, 2013, 12:16 PM
 */

#ifndef BUTTONS_H
#define	BUTTONS_H


#include <xc.h>
#include <stdbool.h>

// Enumerators
typedef enum { NoButton, ShortPress1, ShortPress2, Released} buttonStates_t;

// Prototype Functions
/* Service PushButton
 * Called at a 1mS rate.  Times the length of a button press and sets the appropriate bit.
 */
buttonStates_t  service_pushbutton(void);
#endif	/* BUTTONS_H */

