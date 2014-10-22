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



// This file is also used for the 32-bit version of the library. The define
// DOS32 is used to control the compile target. The functions in the first
// section (labeled "Target specific code") are the only ones that are
// target specific.
// Note: Because the Borland and Watcom compilers use different notations
// when accessing the word registers in a REGS strcuture, all assignments
// to those registers are splitted in two byte assignments. If you change
// this, the module will no longer compatible with all supported compilers.



#include <string.h>
#include <dos.h>
#if defined (__BORLANDC__)
#include <dir.h>
#endif
#if defined (__WATCOMC__)
#include <direct.h>
#endif
#if defined (__GO32__)
#include <dir.h>
#include <osfcn.h>
#endif

#include "check.h"
#include "str.h"
#include "filepath.h"
#include "filesys.h"



/*****************************************************************************/
/*		     File system & OS dependent constants		     */
/*****************************************************************************/



extern const char FileSysPathSep	= '\\'; // Path separator
extern const char FileSysListSep	= ';';	// Path list separator
extern const FileSysMaxPath		=  79;	// Maximum path length
extern const FileSysMaxDir		=  65;	// Maximum directory length
extern const FileSysMaxName		=   8;	// Maximum file name length
extern const FileSysMaxExt		=   4;	// Maximum extension length (including the dot)



/*****************************************************************************/
/*			     Target specific code			     */
/*****************************************************************************/



static char* CurDir (char* Buf, int Drive)
// Store the current working directory of the given drive in Buf and return
// Buf on success. If Drive is invalid, return NULL. Buf must be at least 65
// bytes large.
{
    char PathBuf [65+2];
    char* B;

    // Get the current disk
    int CurDrive = FileSysGetDrive ();

    // Check if we have to change the disk
    if (Drive != 0 && Drive != CurDrive) {
	// Must change the drive
	if (FileSysSetDrive (Drive) != 0) {
	    // Cannot set drive
	    return NULL;
	}
	B = getcwd (PathBuf, sizeof (PathBuf));
	FileSysSetDrive (CurDrive);
    } else {
	// Drive is already ok or default drive
	B = getcwd (PathBuf, sizeof (PathBuf));
    }
    if (B == NULL) {
	// An error occured
	return NULL;
    }

    // As the buffer contains the drive specifier, we have to copy the
    // path only
    strcpy (Buf, PathBuf + 2);

#ifdef __GO32__
    // The C library of gjgpp returns '/' as a path separator - convert it
    B = Buf;
    while (*B != '\0') {
	if (*B == '/') {
	    *B = '\\';
	}
	B++;
    }
#endif

    // Return the result
    return Buf;
}



/*****************************************************************************/
/*				     Code				     */
/*****************************************************************************/



int FileSysGetDrive ()
// Return the current drive. 1 = A:, 2 = B: and so on. On error, -1 is returned.
// This function may be called only if the system supports drives!
{
    // Set up for calling DOS function 19h
    REGS Regs;
    Regs.h.ah = 0x19;

    // Call DOS
    intdos (&Regs, &Regs);

    // Re-code the drive an return it
    return Regs.h.al + 1;
}



int FileSysSetDrive (unsigned Drive)
// Set the current drive. 1 = A:, 2 = B: and so on. On error, -1 is returned,
// otherwise zero. This function may be called only if the system supports
// drives!
{
    // Check the parameter
    PRECONDITION (Drive != 0);

    // Set up for calling DOS function 0Eh
    REGS Regs;
    Regs.h.ah = 0x0E;
    Regs.h.dl = Drive - 1;

    // Call DOS
    intdos (&Regs, &Regs);

    // Unfortunately, there is no error code when using this function...
    return 0;
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
    // In DOS, there is no difference between file systems
    strcpy (Info.fsName, "FAT");
    Info.fsMaxPath	 = FileSysMaxPath;
    Info.fsMaxDir	 = FileSysMaxDir;
    Info.fsMaxName	 = FileSysMaxName;
    Info.fsMaxExt	 = FileSysMaxExt;
    Info.fsPreservesCase = 0;
    Info.fsIgnoresCase	 = 1;
    CurDir (Info.fsCurDir, Drive);
}



int FileSysPreservesCase (int /*Drive*/ )
// Return 1 if the file system on the given drive preserves the case in
// filenames
{
    // DOS does not preserve case in file names
    return 0;
}



int FileSysIgnoresCase (int /*Drive*/ )
// Return 1 if the file system on the given drive ignores the case in file
// names. Beware: This is not the same as FileSysPreservesCase! The latter
// will return true if the file system preserves case in file names given,
// but when searching for files, the case can be ignored (this is the case
// with OS/2's HPFS file systems).
{
    // DOS does ignore case in file names
    return 1;
}



int FileSysValidChar (const char C)
// Return 1 if the given char is a valid part of a directory or file name.
{
    // Invald chars in the fat file system
    static char InvalidChars [] = " <>|+=:;,\"/\\[]";

    // Characters below and including the space are invalid
    if (C <= ' ') {
	return 0;
    }

    // Check for other invalid chars
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

    // DOS file systems support drives, so check for IncludeDrive
    if (IncludeDrive) {
	// If the current drive should be included, get that
	if (Drive == 0) {
	    Drive = FileSysGetDrive ();
	}
	// FAT is not case sensitive, so use lower case
	Dir += 'a' + Drive - 1;
	Dir += ':';
    }

    // Now get the current working directory. Since FAT is not case sensitive,
    // convert the name to lower case.
    char Buf [80];
    if (CurDir (Buf, Drive) == NULL) {
	// OOPS - unknown drive
	Dir.Clear ();
    } else {
	Dir += Buf;
	AddPathSep (Dir);
	Dir.ToLower ();
    }

    // Return the result
    return Dir;
}



