/*****************************************************************************/
/*                                                                           */
/*                                 ICDEVS.H                                  */
/*                                                                           */
/* (C) 1995-96  Ullrich von Bassewitz                                        */
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



#ifndef _ICDEFS_H
#define _ICDEFS_H



#include "icconfig.h"



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



void DeviceList (IstecConfig& Config, int& Changed);
// List all devices and there settings, including the charges. Allow editing
// the settings and charges.



// End of ICDEVS.H

#endif

