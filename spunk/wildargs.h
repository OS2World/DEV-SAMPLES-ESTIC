/*****************************************************************************/
/*									     */
/*				  WILDARGS.H				     */
/*									     */
/* (C) 1996	Ullrich von Bassewitz					     */
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



// This is a special module for DOS and OS/2. It is used to expand the
// argument vector if any of the arguments has wildcards in it.



#ifndef __WILDARGS_H
#define __WILDARGS_H



#include "str.h"



/*****************************************************************************/
/*				     Types				     */
/*****************************************************************************/



typedef char**	_PPCHAR;



/*****************************************************************************/
/*				     Code				     */
/*****************************************************************************/



void ExpandArgs (int& ArgCount, _PPCHAR& ArgVec);
// Expand the argument list



// End of WILDARGS.H

#endif
