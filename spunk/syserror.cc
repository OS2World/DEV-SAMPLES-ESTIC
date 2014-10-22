/*****************************************************************************/
/*									     */
/*				  SYSERROR.CC				     */
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



#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "msgid.h"
#include "str.h"
#include "progutil.h"



/*****************************************************************************/
/*			       Message constants			     */
/*****************************************************************************/



const u16 msDefault		= MSGBASE_SYSERROR +  0;
const u16 msNoError		= MSGBASE_SYSERROR +  1;
const u16 msNoSuchEntry		= MSGBASE_SYSERROR +  2;
const u16 msNoMem		= MSGBASE_SYSERROR +  3;
const u16 msAccessDenied	= MSGBASE_SYSERROR +  4;
const u16 msTooManyOpenFiles	= MSGBASE_SYSERROR +  5;
const u16 msNoSpaceOnDevice	= MSGBASE_SYSERROR +  6;
const u16 msAgain		= MSGBASE_SYSERROR +  7;
const u16 msBusy		= MSGBASE_SYSERROR +  8;
const u16 msFileTooLarge	= MSGBASE_SYSERROR +  9;
const u16 msIOError		= MSGBASE_SYSERROR + 10;
const u16 msIsADirectory	= MSGBASE_SYSERROR + 11;
const u16 msNotADirectory	= MSGBASE_SYSERROR + 12;
const u16 msTooManyLinks	= MSGBASE_SYSERROR + 13;
const u16 msBlockDevRequired	= MSGBASE_SYSERROR + 14;
const u16 msNotACharDev		= MSGBASE_SYSERROR + 15;
const u16 msNoSuchDev		= MSGBASE_SYSERROR + 16;
const u16 msNotOwner		= MSGBASE_SYSERROR + 17;
const u16 msBrokenPipe		= MSGBASE_SYSERROR + 18;
const u16 msReadOnlyFS		= MSGBASE_SYSERROR + 19;
const u16 msIllegalSeek		= MSGBASE_SYSERROR + 20;
const u16 msNoSuchProcess	= MSGBASE_SYSERROR + 21;
const u16 msTextFileBusy	= MSGBASE_SYSERROR + 22;
const u16 msNameTooLong		= MSGBASE_SYSERROR + 23;
const u16 msNoLocksAvailable	= MSGBASE_SYSERROR + 24;
const u16 msDirNotEmpty		= MSGBASE_SYSERROR + 25;
const u16 msFileNotFound	= MSGBASE_SYSERROR + 26;
const u16 msPathNotFound	= MSGBASE_SYSERROR + 27;
const u16 msInvalidDrive	= MSGBASE_SYSERROR + 28;
const u16 msCannotRemoveCurDir	= MSGBASE_SYSERROR + 29;
const u16 msFileExists		= MSGBASE_SYSERROR + 30;
const u16 msUnknown		= MSGBASE_SYSERROR + 31;



/*****************************************************************************/
/*				     Code				     */
/*****************************************************************************/



String GetSysErrorMsg (int Code)
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
{
    unsigned Msg;

    // Load a specific message for some well known error codes
    switch (Code) {

#ifdef EZERO
	case EZERO:		Msg = msNoError;		break;
#endif

#if defined (ENOENT) && (!defined (ENOFILE) || ENOENT != ENOFILE)
	case ENOENT:		Msg = msNoSuchEntry;		break;
#endif

#ifdef ENOMEM
	case ENOMEM:		Msg = msNoMem;			break;
#endif

#ifdef EACCES
	case EACCES:		Msg = msAccessDenied;		break;
#endif

#ifdef EMFILE
	case EMFILE:		Msg = msTooManyOpenFiles;	break;
#endif

#ifdef ENOSPC
	case ENOSPC:		Msg = msNoSpaceOnDevice;	break;
#endif

#ifdef EAGAIN
	case EAGAIN:		Msg = msAgain;			break;
#endif

#ifdef EBUSY
	case EBUSY:		Msg = msBusy;			break;
#endif

#ifdef EFBIG
	case EFBIG:		Msg = msFileTooLarge;		break;
#endif

#ifdef EIO
	case EIO:		Msg = msIOError;		break;
#endif

#ifdef EISDIR
	case EISDIR:		Msg = msIsADirectory;		break;
#endif

#ifdef ENOTDIR
	case ENOTDIR:		Msg = msNotADirectory;		break;
#endif

#ifdef EMLINK
	case EMLINK:		Msg = msTooManyLinks;		break;
#endif

#ifdef ENOTBLK
	case ENOTBLK:		Msg = msBlockDevRequired;	break;
#endif

#ifdef ENOTTY
	case ENOTTY:		Msg = msNotACharDev;		break;
#endif

#ifdef ENXIO
	case ENXIO:		Msg = msNoSuchDev;		break;
#endif

#if defined (EPERM) && (!defined (EACCES) || EPERM != EACCES)
	case EPERM:		Msg = msNotOwner;		break;
#endif

#ifdef EPIPE
	case EPIPE:		Msg = msBrokenPipe;		break;
#endif

#ifdef EROFS
	case EROFS:		Msg = msReadOnlyFS;		break;
#endif

#ifdef ESPIPE
	case ESPIPE:		Msg = msIllegalSeek;		break;
#endif

#ifdef ESRCH
	case ESRCH:		Msg = msNoSuchProcess;		break;
#endif

#ifdef ETXTBSY
	case ETXTBSY:		Msg = msTextFileBusy;		break;
#endif

#ifdef ENAMETOOLONG
	case ENAMETOOLONG:	Msg = msNameTooLong;		break;
#endif

#if defined (ENODEV) && (!defined (ENXIO) || ENODEV != ENXIO)
	case ENODEV:		Msg = msNoSuchDev;		break;
#endif

#ifdef ENOLCK
	case ENOLCK:		Msg = msNoLocksAvailable;	break;
#endif

#ifdef ENOTEMPTY
	case ENOTEMPTY:		Msg = msDirNotEmpty;		break;
#endif

#ifdef ENOFILE
	case ENOFILE:		Msg = msFileNotFound;		break;
#endif

#ifdef ENOPATH
	case ENOPATH:		Msg = msPathNotFound;		break;
#endif

#if defined (EINVDRV) && EINVDRV != ENODEV
	case EINVDRV:		Msg = msInvalidDrive;		break;
#endif

#ifdef ECURDIR
	case ECURDIR:		Msg = msCannotRemoveCurDir;	break;
#endif

#ifdef EEXIST
	case EEXIST:		Msg = msFileExists;		break;
#endif

	default:
	    // Unknown error code. Beware: Some compilers (Watcom for example)
	    // give invalid error numbers, so check at least negative codes
	    // here...
	    if (Code < 0) {
		return FormatStr (LoadMsg (msUnknown).GetStr (), Code);
	    } else {
		const char* Msg = strerror (Code);
		return FormatStr (LoadMsg (msDefault).GetStr (), Code, Msg);

	    }

    }

    // If we get here, Msg contains a valid message number.
    return LoadMsg (Msg);
}



