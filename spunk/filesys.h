/*****************************************************************************/
/*									     */
/*				   FILESYS.H				     */
/*									     */
/* (C) 1995	Ullrich von Bassewitz					     */
/*		Zwehrenbuehlstrasse 33					     */
/*		D-72070 Tuebingen					     */
/* EMail:	uz@ibb.schwaben.com					     */
/*									     */
/*****************************************************************************/



//
// $Id$
//
// $Log$
//
//



#ifndef __FILESYS_H
#define __FILESYS_H



#include "str.h"



/*****************************************************************************/
/*		     File system & OS dependent constants		     */
/*****************************************************************************/



// The following constants describe features of the file system. To avoid
// compiler warnings (regarding dead code) the constants are not declared as
// static but as extern. This will result in somewhat larger code because
// in some places code is included that is never executed, but as there are
// only a few places, I thought it would be safer to get rid of the compiler
// warnings than to minimize code size.

extern const char FileSysPathSep;	// Path separator
extern const char FileSysListSep;	// Path list separator
extern const FileSysMaxPath;		// Maximum path length
extern const FileSysMaxDir;		// Maximum directory length
extern const FileSysMaxName;		// Maximum file name length
extern const FileSysMaxExt;		// Maximum extension length (including the dot)



/*****************************************************************************/
/*			      struct FileSysInfo			     */
/*****************************************************************************/



struct FileSysInfo {
    char	fsName [80];		// Name of the file system
//  char	fsValidChars [32];	// Bitset, 1 == valid file name char
    unsigned	fsMaxPath;		// Maximum length of a path
    unsigned	fsMaxDir;		// Maximum length of a directory
    unsigned	fsMaxName;		// Maximum length of a file name
    unsigned	fsMaxExt;		// Maximum length of a extension (including the dot)
    char	fsCurDir [256];		// Current directory
    int		fsPreservesCase;	// 1 if filesys preserves case
    int		fsIgnoresCase;		// 1 if filesys ignores case
};



/*****************************************************************************/
/*				     Code				     */
/*****************************************************************************/



int FileSysGetDrive ();
// Return the current drive. 1 = A:, 2 = B: and so on. On error, -1 is returned.
// This function may be called only if the system supports drives!

int FileSysSetDrive (unsigned Drive);
// Set the current drive. 1 = A:, 2 = B: and so on. On error, -1 is returned,
// otherwise zero. This function may be called only if the system supports
// drives!

int FileSysExtractDrive (const String& Path);
// Return the drive spec from the given path. If none given or if the os
// does not support drives (linux), the return value will be zero (== current).

void FileSysGetInfo (FileSysInfo& Info, int Drive = 0);
// Return detailed information regarding the file system on the given drive.
// See declaration of struct FileSysInfo above.

inline void FileSysGetInfo (FileSysInfo& Info, const String& Path)
// Return detailed information regarding the file system on the given drive.
// See declaration of struct FileSysInfo above.
{
    FileSysGetInfo (Info, FileSysExtractDrive (Path));
}

int FileSysPreservesCase (int Drive = 0);
// Return 1 if the file system on the given drive preserves the case in
// filenames

inline int FileSysPreservesCase (const String& Path)
// Return 1 if the file system on the given drive preserves the case in
// filenames
{
    return FileSysPreservesCase (FileSysExtractDrive (Path));
}

int FileSysIgnoresCase (int Drive = 0);
// Return 1 if the file system on the given drive ignores the case in file
// names. Beware: This is not the same as FileSysPreservesCase! The latter
// will return true if the file system preserves case in file names given,
// but when searching for files, the case can be ignored (this is the case
// with OS/2's HPFS file systems).

inline int FileSysIgnoresCase (const String& Path)
// Return 1 if the file system on the given drive ignores the case in file
// names. Beware: This is not the same as FileSysPreservesCase! The latter
// will return true if the file system preserves case in file names given,
// but when searching for files, the case can be ignored (this is the case
// with OS/2's HPFS file systems).
{
    return FileSysIgnoresCase (FileSysExtractDrive (Path));
}

int FileSysValidChar (const char C);
// Return 1 if the given char is a valid part of a directory or file name.
// Because the function has no information about the file system, it assumes
// "worst case" and rejects every character that may be illegal on any of the
// supported file systems.

int FileSysValidName (const String& Path);
// Return 1 if the given path name consists of valid chars for the given
// file system. Beware: This function does not check if the path exists!

#ifdef FILESYS_HAS_DRIVES
String FileSysCurrentDir (int IncludeDrive = 1, int Drive = 0);
#else
String FileSysCurrentDir (int IncludeDrive = 0, int Drive = 0);
#endif
// Return the current directory. If IncludeDrive is true, the drive letter is
// included in the current directory. The default is to include the drive
// letter on systems that support drives. If the given drive is invalid, an
// empty string is returned.
// The returned path includes a trailing path separator.



// End of FILESYS.H

#endif





