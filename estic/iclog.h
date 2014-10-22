/*****************************************************************************/
/*									     */
/*				   ICLOG.H				     */
/*									     */
/* (C) 1995-96	Ullrich von Bassewitz					     */
/*		Wacholderweg 14						     */
/*		D-70597 Stuttgart					     */
/* EMail:	uz@ibb.schwaben.com					     */
/*									     */
/*****************************************************************************/



// $Id$
//
// $Log$
//
//



#ifndef _ICLOG_H
#define _ICLOG_H



#include "datetime.h"



/*****************************************************************************/
/*				     Data				     */
/*****************************************************************************/



// Names of the logfiles
extern String OutgoingLog1;
extern String OutgoingLog2;
extern String OutgoingLog3;
extern String IncomingLog1;
extern String IncomingLog2;
extern String IncomingLog3;

// If true, log calls with a chargecount of zero
extern int LogZeroCostCalls;

// Price of a charge unit
extern double PricePerUnit;



// End of ICLOG.H

#endif

