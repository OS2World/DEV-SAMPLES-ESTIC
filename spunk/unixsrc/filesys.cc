/*****************************************************************************/
/*									     */
/*				   FILESYS.CC				     */
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


// This is a generic unix file system module. It assumes file names and
// paths are 255 chars long.



#include <unistd.h>
#include <string.h>

#include "../check.h"
#include "../str.h"
#include "../filepath.h"
#include "../filesys.h"



/*****************************************************************************/
/*		     File system & OS dependent constants		     */
/*****************************************************************************/



extern const char FileSysPathSep	= '/';	// Path separator
extern const char FileSysListSep	= ':';	// Path list separator
extern const FileSysMaxPath		= 255;	// Maximum path length
extern const FileSysMaxDir		= 255;	// Maximum directory length
extern const FileSysMaxName		= 255;	// Maximum file name length
extern const FileSysMaxExt		= 255;	// Maximum extension length (including the dot)



/*****************************************************************************/
/*				     Code				     */
/*****************************************************************************/



int FileSysGetDrive ()
// Return the current drive. 1 = A:, 2 = B: and so on. On error, -1 is returned.
// This function may be called only if the system supports drives!
{
    FAIL ("Call to FileSysGetDrive on system without drives");
    return 0;
}



int FileSysSetDrive (unsigned /*Drive*/)
// Set the current drive. 1 = A:, 2 = B: and so on. On error, -1 is returned,
// otherwise zero. This function may be called only if the system supports
// drives!
{
    FAIL ("Call to FileSysSetDrive on system without drives");
    return 0;
}



int FileSysExtractDrive (const String& /*Path*/)
// Return the drive spec from the given path. If none given or if the os
// does not support drives (*nix), the return value will be zero (== current).
{
    // *nix does not support drives
    return 0;
}



void FileSysGetInfo (FileSysInfo& Info, int /*Drive*/)
// Return detailed information regarding the file system on the given drive.
// See declaration of struct FileSysInfo above.
{
    // Use a generic description, even if this is not correct...
    strcpy (Info.fsName, "UNKNOWN");
    Info.fsMaxPath	 = FileSysMaxPath;
    Info.fsMaxDir	 = FileSysMaxDir;
    Info.fsMaxName	 = FileSysMaxName;
    Info.fsMaxExt	 = FileSysMaxExt;
    Info.fsPreservesCase = 1;
    Info.fsIgnoresCase	 = 0;
    getcwd (Info.fsCurDir, sizeof (Info.fsCurDir));
}



int FileSysPreservesCase (int /*Drive*/)
// Return 1 if the file system on the given drive preserves the case in
// filenames
{
    // *nix always(?) preserves the case of names
    return 1;
}



int FileSysIgnoresCase (int /*Drive*/)
// Return 1 if the file system on the given drive ignores the case in file
// names. Beware: This is not the same as FileSysPreservesCase! The latter
// will return true if the file system preserves case in file names given,
// but when searching for files, the case can be ignored (this is the case
// with OS/2's HPFS file systems).
{
    // *nix never(?) ignores case in file names
    return 0;
}



int FileSysValidChar (const char C)
// Return 1 if the given char is a valid part of a directory or file name.
// Because the function has no information about the file system, it assumes
// "worst case" and rejects every character that may be illegal on any of the
// supported file systems.
{
    // Invald chars
    static char InvalidChars [] = " <>|=:\"\\[]";

    // Characters below and including the space are invalid
    if (C <= ' ') {
	return 0;
    }

    return strchr (InvalidChars, C) == NULL;
}



int FileSysValidName (const String& Path)
// Return 1 if the given path name consists of valid chars for the given
// file system. Beware: This function does not check if the path exists!
{
    for (int I = 0; I < Path.Len (); I++) {
	if (FileSysValidChar (Path [I]) == 0) {
	    return 0;
	}
    }
    return 1;
}



String FileSysCurrentDir (int IncludeDrive, int /* Drive */)
// Return the current directory. If IncludeDrive is true, the drive letter is
// included in the current directory. The default is to include the drive
// letter on systems that support drives. If the given drive is invalid, an
// empty string is returned.
// The returned path includes a trailing path separator.
{
    // *nix does not support drives, so a request to include the drive
    // letter is invalid
    PRECONDITION (IncludeDrive == 0);

    // Return the current working directory
    char Buf [1024];
    if (getcwd (Buf, sizeof (Buf)) == NULL) {
	// OOPS, working dir is not readable
	return "";
    } else {
	String Dir (Buf);
	AddPathSep (Dir);
	return Dir;
    }
}




