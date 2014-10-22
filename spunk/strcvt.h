/*****************************************************************************/
/*                                                                           */
/*                                 STRCVT.H                                  */
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



#ifndef _STRCVT_H
#define _STRCVT_H



#include "str.h"



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



String I32Str (i32 Val, unsigned Base = 10);
// Convert an integer into a string

String U32Str (u32 Val, unsigned Base = 10);
// Convert an unsigned into a string

String FloatStr (double Val, unsigned LDigits, unsigned TDigits);
// Convert a double into a string. There is a difference between this function
// and the converting functions for integers: The result definitely has not
// more than the given width.

String TimeStr (u32 TimeInSec);
// Convert a time (given in seconds since midnight) into a string.



// End of STRCVT.H

#endif

