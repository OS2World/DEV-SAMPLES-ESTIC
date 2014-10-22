/*****************************************************************************/
/*									     */
/*				    SCREEN.CC				     */
/*									     */
/* (C) 1993-95	Ullrich von Bassewitz					     */
/*		Zwehrenbuehlstrasse 33					     */
/*		D-72070 Tuebingen					     */
/* EMail:	uz@ibb.schwaben.com					     */
/*									     */
/*****************************************************************************/



// $Id$
//
// $Log$
//
//



#include <stdlib.h>
#include <sys/ioctl.h>
#ifdef FREEBSD
#    include <machine/console.h>
#endif

#include "../screen.h"



/*****************************************************************************/
/*				     Code				     */
/*****************************************************************************/



static int ScrIsConsole ()
// Return true if this is a console screen
{
#ifdef FREEBSD
    // This is the console if we can request the keyboard leds
    int Dummy;
    return ioctl (0, KDGETLED, &Dummy) == 0;
#else
    return 0;
#endif
}



static int ScrHasColor ()
// Return true if this is a console that supports colors
{
#ifdef FREEBSD
    int Color;
    if (ioctl (0, GIO_COLOR, &Color) != 0) {
	// Error - this is not the console, assume no colors
	return 0;
    }

    // Return the value from the console driver
    return Color;
#else
    // Assume no colors
    return 0;
#endif
}



/*****************************************************************************/
/*				 class Screen				     */
/*****************************************************************************/



Screen::Screen ():
    Color (ScrHasColor ()),
    Console (ScrIsConsole ()),
    CP437 (0),
    TransTable (NULL)
{
    // Initialize the termcap system
    TCInit ();
}



char* Screen::GetIS (char* IS)
// Return a replacement for the init strings IS and RS. Used for *nixen
// only.
{
    return IS;
}



char* Screen::GetRS (char* RS)
// Return a replacement for the init strings IS and RS. Used for *nixen
// only.
{
    return RS;
}




