/*****************************************************************************/
/*                                                                           */
/*                                  ICMSG.H                                  */
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



#ifndef _ICMSG_H
#define _ICMSG_H



#include "machine.h"



/*****************************************************************************/
/*                            Message constants                              */
/*****************************************************************************/



const u16 MSGBASE_ISTEC         =         0;
const u16 MSGBASE_ICERROR       =       200;
const u16 MSGBASE_ICIDENT       =       300;
const u16 MSGBASE_ICDEVS        =       400;
const u16 MSGBASE_ICBASEED      =       500;
const u16 MSGBASE_ICDIAG        =       600;
const u16 MSGBASE_ICALIAS       =       700;
const u16 MSGBASE_ICIMON        =       800;
const u16 MSGBASE_ICDLOG        =       900;
const u16 MSGBASE_ICCWIN        =      1100;
const u16 MSGBASE_CHARGWIN      =      1200;
const u16 MSGBASE_ICCRON        =      1300;
const u16 MSGBASE_ICCLIWIN      =      1400;
const u16 MSGBASE_ICCTI         =      1500;
const u16 MSGBASE_ICFILE        =      1600;
const u16 MSGBASE_ICSHORT       =      1700;



// End of ICMSG.H

#endif
