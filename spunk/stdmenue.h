/*****************************************************************************/
/*                                                                           */
/*                                STDMENUE.H                                 */
/*                                                                           */
/* (C) 1993,94  Ullrich von Bassewitz                                        */
/*              Wacholderweg 14                                              */
/*              D-70597 Stuttgart                                            */
/* EMail:       uz@ibb.schwaben.com                                          */
/*                                                                           */
/*****************************************************************************/



// $Id$
//
// $Log$
//
//



#ifndef _STDMENUE_H
#define _STDMENUE_H


#include "menue.h"



/*****************************************************************************/
/*                 Result codes for the Abort/No/Yes menues                  */
/*****************************************************************************/



const unsigned arAbort          = 0;
const unsigned arNo             = 1;
const unsigned arYes            = 2;



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



unsigned SimpleMenue (const String & Header, const String &MenueText);
// Builds a menue from the given strings, shows that menue and returns the
// users choice. The return value is 0 if the menue was aborted.
// Use '|' to separate ther entries, use '^' to create a new item that is
// centered. Use '@' to mark hotkeys (as usual).



unsigned AskYesNo (const String & Header);
// Pop up a menue with the given header and the entries "Yes" and "No",
// "Yes" being the default entry.
// Returns 0 = Abort, 1 = No, 2 = Yes



unsigned AskNoYes (const String & Header);
// Pop up a menue with the given header and the entries "No" and "Yes",
// "No" being the default entry.
// Returns 0 = Abort, 1 = No, 2 = Yes



unsigned AskReallyQuit ();
// Pops up a menue with the question "Really quit" and returns the result
// 0 = Abort, 1 = No, 2 = Yes



unsigned AskDiscardChanges ();
// Pops up a menue with the question "Discard changes?" and returns the result
// 0 = Abort, 1 = No, 2 = Yes



unsigned AskSaveChanges ();
// Pops up a menue with the question "Save changes?" and returns the result
// 0 = Abort, 1 = No, 2 = Yes



unsigned AskAreYouShure ();
// Pops up a menue with the question "Are you shure?" and returns the result
// 0 = Abort, 1 = No, 2 = Yes



unsigned MenueChoice (const String& ResName);
// Loads a menue with the name ResName from the resource file, gets the user
// response and deletes the menue.



// End of STDMENUE.H

#endif

