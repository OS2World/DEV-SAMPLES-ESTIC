/*****************************************************************************/
/*                                                                           */
/*                                ICALIAS.H                                  */
/*                                                                           */
/* (C) 1995     Ullrich von Bassewitz                                        */
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



// This module holds aliases for devices



#ifndef _ICALIAS_H
#define _ICALIAS_H



#include "str.h"



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



// Name of the alias file, default is the empty string
extern String AliasFile;

// If true, reread the alias file before trying to resolve call data
extern int AutoReadAliases;



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



void NewAlias (String Number, const String& Alias);
// Create a new number alias.

void NewAlias (unsigned char Dev, const String& Alias);
// Create a new device alias. The valid range for dev is 21...

const String& GetAlias (String Number);
// Return the alias of a number. Return an empty string if there is no alias.

const String& GetAlias (unsigned char Dev);
// Return the alias of a device. Return an empty string if there is no alias.
// The valid range for Dev is 21...

void ReadAliasFile ();
// Delete all existing aliases and read in the aliasfile with the given name
// The function does nothing if there is no external aliasfile defined.



// End of ICALIAS.H

#endif

