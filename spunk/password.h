/*****************************************************************************/
/*                                                                           */
/*                                PASSWORD.H                                 */
/*                                                                           */
/* (C) 1994     Ullrich von Bassewitz                                        */
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



#ifndef __PASSWORD_H
#define __PASSWORD_H



#include "coll.h"



/*****************************************************************************/
/*                                Global data                                */
/*****************************************************************************/



extern const String& CUN;               // Current user name
extern const String& CUID;              // Current user id
extern const u32& CPL;                  // Current users security level



/*****************************************************************************/
/*                              PasswordEditor                               */
/*****************************************************************************/



void PasswordEditor (const String& Filename);
// Loads a password collection from the given file and allows editing users/
// passwords



/*****************************************************************************/
/*                               Login/Logout                                */
/*****************************************************************************/



void Login (const String& PWName, const String& Logname);
// Ask for user id and password and set the variables CUN CUID and CPL
// according to the users password entry.
// If Logname is not empty, a binary log of all login attempts is stored there.



void Logout (const String& Logname);
// Reset the user data. If Logname is not empty, a binary log of the
// login/logout sequences is kept there.



// End of PASSWORD.H

#endif

