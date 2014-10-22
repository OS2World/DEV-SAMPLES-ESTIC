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



#include <unistd.h>

#include "../environ.h"
#include "../screen.h"



/*****************************************************************************/
/*                               class Screen                                */
/*****************************************************************************/



Screen::Screen ():
    Color (0),                          // Default is no color
    Console (0),
    CP437 (0),                          // Default is no codepage 437
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



