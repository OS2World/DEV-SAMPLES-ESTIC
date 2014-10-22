/*****************************************************************************/
/*									     */
/*				   FILESYS.CC				     */
/*									     */
/* (C) 1995-96	Ullrich von Bassewitz					     */
/*		Wacholderweg 14						     */
/*		D-70597 Stuttgart					     */
/* EMail:	uz@ibb.schwaben.com					     */
/*									     */
/*****************************************************************************/



//
// $Id$
//
// $Log$
//
//



#include <string.h>
#include <dos.h>
#define INCL_BASE
#include <os2.h>

#include "check.h"
#include "str.h"
#include "filepath.h"
#include "filesys.h"



/*****************************************************************************/
/*		     File system & OS dependent constants		     */
/*****************************************************************************/



extern const char FileSysPathSep	= '\\'; // Path separator
extern const char FileSysListSep	= ';';	// Path list separator
extern const FileSysMaxPath		= 259;	// Maximum path length
extern const FileSysMaxDir		= 254;	// Maximum directory length
extern const FileSysMaxName		= 254;	// Maximum file name length
extern const FileSysMaxExt		= 254;	// Maximum extension length (including the dot)



/*****************************************************************************/
/*				     Code				     */
/*****************************************************************************/



static char* FSType (int Drive, char* Buf, unsigned BufSize, ULONG& Error)
{
    char fsqbuf [512];
    char* P;
    ULONG  fsqbuflen = sizeof (fsqbuf);
    char DriveSpec [3];

    // Set up drive
    if (Drive == 0) {
	// No Path or no path in drive
	Drive = FileSysGetDrive ();
    }
    DriveSpec [0] = Drive + 'A' - 1;
    DriveSpec [1] = ':';
    DriveSpec [2] = '\0';

    Error = DosQueryFSAttach (DriveSpec, 0, FSAIL_QUERYNAME, (PFSQBUFFER2) fsqbuf,
			      &fsqbuflen);
    if (Error != 0) {
	return NULL;
    }

    // Set 'P' to point to the file system name
    P = fsqbuf + sizeof(USHORT);    // Point past device type
    P += *((PUSHORT) P) + 3*sizeof(USHORT) + 1;
				    // Point past drive name and FS name length */

    return strncpy (Buf, P, BufSize - 1);
}



int FileSysGetDrive ()
// Return the current drive. 1 = A:, 2 = B: and so on. On error, -1 is returned.
// This function may be called only if the system supports drives!
{
    ULONG Drive;
    ULONG DriveMap;
    if (DosQueryCurrentDisk (&Drive, &DriveMap) != 0) {
	// An error occured
	return -1;
    } else {
	// Completed without errors
	return Drive;
    }
}



int FileSysSetDrive (unsigned Drive)
// Set the current drive. 1 = A:, 2 = B: and so on. On error, -1 is returned,
// otherwise zero. This function may be called only if the system supports
// drives!
{
    return DosSetDefaultDisk (Drive) == 0? 0 : -1;
}



int FileSysExtractDrive (const String& Path)
// Return the drive spec from the given path. If none given or if the os
// does not support drives (linux), the return value will be zero (== current).
{
    if (Path.Len () >= 2 && Path [1] == ':') {
	// Drive in first char
	int Drive = Path [0];
	return (Drive >= 'a') ? Drive - 'a' + 1 : Drive - 'A' + 1;
    } else {
	// No drive spec
	return 0;
    }
}



void FileSysGetInfo (FileSysInfo& Info, int Drive)
// Return detailed information regarding the file system on the given drive.
// See declaration of struct FileSysInfo above.
{
    // Create the info
    ULONG Error;
    FSType (Drive, Info.fsName, sizeof (Info.fsName), Error);
    if (Error == 0 && strcmp (Info.fsName, "HPFS") == 0) {
	// HPFS file system
	Info.fsMaxPath		= 259;
	Info.fsMaxDir		= 254;
	Info.fsMaxName		= 254;
	Info.fsMaxExt		= 254;
	Info.fsPreservesCase	= 1;
	Info.fsIgnoresCase	= 1;
    } else if (Error == 0 && strcmp (Info.fsName, "NFS") == 0) {
	// HPFS file system
	Info.fsMaxPath		= 259;
	Info.fsMaxDir		= 254;
	Info.fsMaxName		= 254;
	Info.fsMaxExt		= 254;
	Info.fsPreservesCase	= 1;
	Info.fsIgnoresCase	= 1;
    } else if (Error == 0 || Error == ERROR_NOT_READY) {
	// FAT or some other file system
	Info.fsMaxPath		= 79;
	Info.fsMaxDir		= 65;
	Info.fsMaxName		=  8;
	Info.fsMaxExt		=  4;
	Info.fsPreservesCase	= 0;
	Info.fsIgnoresCase	= 1;
    } else {
	FAIL ("FileSysGetInfo: Requesting info on a non existing drive");
    }

    // Get the current directory. Beware! OS/2 does not start the directory
    // with a path separator (contrary to the docs which state that the
    // directory is fully qualified). This may be a bug which will be corrected
    // in a future version. Since there is a path separator if the current
    // directory is the root dir, check before adding one...
    if (Error == 0) {
	char Buf [sizeof (Info.fsCurDir) - 1];
	ULONG BufLen = sizeof (Buf);
	DosQueryCurrentDir (Drive, Buf, &BufLen);
	if (Buf [0] == FileSysPathSep) {
	    // There is already a path separator, just copy the string
	    strcpy (Info.fsCurDir, Buf);
	} else {
	    // There is not path separator - add one
	    Info.fsCurDir [0] = FileSysPathSep;
	    strcpy (&Info.fsCurDir [1], Buf);
	}
    } else {
	// We had a "drive not ready error" before, assume root dir
	strcpy (Info.fsCurDir, "\\");
    }
}



int FileSysPreservesCase (int Drive)
// Return 1 if the file system on the given drive preserves the case in
// filenames
{
    // Get the name of the file system. On errors, assume FAT
    ULONG Error;
    char Buf [128];
    if (FSType (Drive, Buf, sizeof (Buf), Error) == NULL) {
	return 0;
    }

    // Only HPFS is case preserving
    return strcmp (Buf, "HPFS") == 0;
}



int FileSysIgnoresCase (int /*Drive*/ )
// Return 1 if the file system on the given drive ignores the case in file
// names. Beware: This is not the same as FileSysPreservesCase! The latter
// will return true if the file system preserves case in file names given,
// but when searching for files, the case can be ignored (this is the case
// with OS/2's HPFS file systems).
{
    // OS/2 does ignore case in file names
    return 1;
}



int FileSysValidChar (const char C)
// Return 1 if the given char is a valid part of a directory or file name.
// Because the function has no information about the file system, it assumes
// "worst case" and rejects every character that may be illegal on any of the
// supported file systems.
{
    // Invald chars
    static char InvalidChars [] = " <>|+=:;,\"/\\[]";

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



String FileSysCurrentDir (int IncludeDrive, int Drive)
// Return the current directory. If IncludeDrive is true, the drive letter is
// included in the current directory. The default is to include the drive
// letter on systems that support drives. If the given drive is invalid, an
// empty string is returned.
// The returned path includes a trailing path separator.
{
    String Dir;

    // OS/2 file systems support drives, so check for IncludeDrive
    if (IncludeDrive) {
	// If the current drive should be included, get that
	if (Drive == 0) {
	    ULONG DriveNumber, DriveMap;
	    DosQueryCurrentDisk (&DriveNumber, &DriveMap);
	    Drive = DriveNumber;
	}
	// Use lower case for the drive letter
	Dir += 'a' + Drive - 1;
	Dir += ':';
    }

     // Now get the current working directory
    char Buf [512];
    ULONG BufLen = sizeof (Buf);
    if (DosQueryCurrentDir (Drive, Buf, &BufLen) != 0) {
	// OOPS - unknown drive
	Dir.Clear ();
    } else {
	// Beware! Buf does not seem to contain a leading path separator under
	// OS/2 2.1. As the documentation states, DosQueryCurrentDrive returns
	// a fully qualified path name, so this seems to be a bug. Since there
	// is a path separator if the current directory is the root dir, check
	// before adding one...
	if (Buf [0] != FileSysPathSep) {
	    Dir += FileSysPathSep;
	}
	Dir += Buf;
	AddPathSep (Dir);

	// If the file system does not preserve case, make the name lowercase
	if (FileSysPreservesCase (Drive) == 0) {
	    Dir.ToLower ();
	}
    }

    // Return the result
    return Dir;
}



