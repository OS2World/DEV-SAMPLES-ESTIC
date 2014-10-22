/*****************************************************************************/
/*									     */
/*				  SYSERROR.H				     */
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



#ifndef _SYSERROR_H
#define _SYSERROR_H



#include <errno.h>

#include "str.h"



/*****************************************************************************/
/*				     Code				     */
/*****************************************************************************/



String GetSysErrorMsg (int Errno);
// This function tries to map a system error code to an error message in the
// current language. This is not as easy as it seems, since the error codes
// not only differ from operating system to operating system, but also from
// compiler to compiler. If there is no predefined message, a default message
// including the error number and the error string from sys_errlist (in
// english) is returned.
// Please note: The use of this function is not, to provide a verbose error
// message for each and every error code, but to provide messages of the more
// common errors. So, for example, EBADF ("bad file number") will _not_ get
// mapped, since this error code denotes a program bug, but ENOENT _will_
// map to a verbose message, because this error may happen on a bad user input.



// End of SYSERROR.H

#endif
