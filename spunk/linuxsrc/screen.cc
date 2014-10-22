/*****************************************************************************/
/*                                                                           */
/*                                  SCREEN.CC                                */
/*                                                                           */
/* (C) 1993-95  Ullrich von Bassewitz                                        */
/*              Zwehrenbuehlstrasse 33                                       */
/*              D-72070 Tuebingen                                            */
/* EMail:       uz@ibb.schwaben.com                                          */
/*                                                                           */
/*****************************************************************************/



// $Id$
//
// $Log$
//
//



#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/kd.h>

#include "../screen.h"



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



static int ScrIsConsole ()
// Return true if this is a console screen
{
    // This is the console if we can request the keyboard meta mode
    int Dummy;
    return ioctl (STDOUT_FILENO, KDGETMODE, &Dummy) == 0;
}



/*****************************************************************************/
/*                               class Screen                                */
/*****************************************************************************/



Screen::Screen ():
    Color (ScrIsConsole ()),    // Default is to use color at the console
    Console (ScrIsConsole ()),
    CP437 (ScrIsConsole ()),    // Default is to use CP437 at the console
    TransTable (NULL)
{
    // Initialize the termcap system
    TCInit ();
}



char* Screen::GetIS (char* IS)
// Return a replacement for the init strings IS and RS. Used for *nixen
// only.
{
    // We have a new string if we are on the console and are using CP437
    return (Console && CP437)? "\033(U\033>\033[4;20l" : IS;
}



char* Screen::GetRS (char* RS)
// Return a replacement for the init strings IS and RS. Used for *nixen
// only.
{
    // We have a new string if we are on the console and are using CP437
    return (Console && CP437)? "\033(B\033>\033[4;20l\033[?25h" : RS;
}



