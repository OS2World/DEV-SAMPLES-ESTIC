/*****************************************************************************/
/*                                                                           */
/*                                 STATDEF.H                                 */
/*                                                                           */
/* (C) 1995     Ullrich von Bassewitz                                        */
/*              Zwehrenbuehlstrasse 33                                       */
/*              D-72070 Tuebingen                                            */
/* EMail:       uz@ibb.schwaben.com                                          */
/*                                                                           */
/*****************************************************************************/



// $Id$
//
// $Log$
//
//



// This file adds some defines that are missing in the DOS & OS/2 compilers for
// handling the fields of struct stat and return codes for access()



#ifndef __STATDEF_H
#define __STATDEF_H



#include <sys/stat.h>



/*****************************************************************************/
/*                                  Defines                                  */
/*****************************************************************************/



#ifndef R_OK
#define R_OK    4
#endif

#ifndef W_OK
#define W_OK    2
#endif

#ifndef X_OK
#define X_OK    1
#endif

#ifndef F_OK
#define F_OK    0
#endif

#ifndef S_IRUSR
#define S_IRUSR                 S_IREAD
#endif

#ifndef S_IWUSR
#define S_IWUSR                 S_IWRITE
#endif

#ifndef S_IXUSR
#define S_IXUSR                 S_IEXEC
#endif

#ifndef S_IRGRP
#define S_IRGRP                 S_IRUSR
#endif

#ifndef S_IWGRP
#define S_IWGRP                 S_IWUSR
#endif

#ifndef S_IXGRP
#define S_IXGRP                 S_IXUSR
#endif

#ifndef S_IROTH
#define S_IROTH                 S_IRUSR
#endif

#ifndef S_IWOTH
#define S_IWOTH                 S_IWUSR
#endif

#ifndef S_IXOTH
#define S_IXOTH                 S_IXUSR
#endif

#ifndef S_IRWXU
#define S_IRWXU                 (S_IRUSR | S_IWUSR | S_IXUSR)
#endif

#ifndef S_IRWXG
#define S_IRWXG                 (S_IRGRP | S_IWGRP | S_IXGRP)
#endif

#ifndef S_IRWXO
#define S_IRWXO                 (S_IROTH | S_IWOTH | S_IXOTH)
#endif

#ifndef S_ISUID
#define S_ISUID                 0
#endif

#ifndef S_ISGID
#define S_ISGID                 0
#endif

#ifndef S_ISVTX
#define S_ISVTX                 0
#endif

#ifndef S_ISBLK
#define S_ISBLK(__mode)         0
#endif

#ifndef S_ISFIFO
#define S_ISFIFO(__mode)        0
#endif

#ifndef S_ISSOCK
#define S_ISSOCK(__mode)        0
#endif

#ifndef S_ISLNK
#define S_ISLNK(__mode)         0
#endif

#ifndef S_ISCHR
#define S_ISCHR(__mode)         (((__mode) & S_IFMT) == S_IFCHR)
#endif

#ifndef S_ISDIR
#define S_ISDIR(__mode)         (((__mode) & S_IFMT) == S_IFDIR)
#endif

#ifndef S_ISREG
#define S_ISREG(__mode)         (((__mode) & S_IFMT) == S_IFREG)
#endif



// End of STATDEF.H

#endif


