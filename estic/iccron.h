/*****************************************************************************/
/*                                                                           */
/*                                 ICCRON.H                                  */
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



// Cron for ESTIC



#ifndef _ICCRON_H
#define _ICCRON_H



#include "datetime.h"



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



// Name of the cron file, default is the empty string
extern String CronFile;



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



void HandleCronEvent (const Time& T);
// Execute all events that match the given time

void ReadCronFile ();
// Delete all existing cron events and reread the cron file. The function
// does nothing if there is no cron file defined.



// End of ICCRON.H

#endif

