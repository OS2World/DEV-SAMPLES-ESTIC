/*****************************************************************************/
/*                                                                           */
/*                                ICBASEED.H                                 */
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



#ifndef _ICBASEED_H
#define _ICBASEED_H



#include "icconfig.h"



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



void LoadConfigDefault (IstecConfig& Config, unsigned IstecType);
// Load the default parameter settings for the given istec type.

void EditBaseConfig (IstecConfig& Config, int IstecPresent, int& Changed);
// Allow editing the istec base configuration. If the istec is not present,
// the type of the istec can be changed, resulting in a load of the default
// for this istec type. If Config has been changed, Changed is set to 1,
// otherwise, Changed is left untouched.



// End of ICBASEED.H

#endif

