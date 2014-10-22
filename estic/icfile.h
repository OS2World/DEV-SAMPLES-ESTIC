/*****************************************************************************/
/*                                                                           */
/*                                 ICFILE.H                                  */
/*                                                                           */
/* (C) 1996     Ullrich von Bassewitz                                        */
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



// Load/save configuration data to a file



#ifndef _ICFILE_H
#define _ICFILE_H



#include "str.h"

#include "icconfig.h"



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



String IstecSaveFile (const String& Filename, const IstecConfig& Config);
// Save a configuration to a file. The function returns an error message or
// the empty string if all is well.

String IstecLoadFile (const String& Filename, IstecConfig& Config);
// Load a configuration from a file. The function returns an error message or
// the empty string if all is well.



// End of ICFILE.H

#endif

