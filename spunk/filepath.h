/*****************************************************************************/
/*                                                                           */
/*                                 FILEPATH.H                                */
/*                                                                           */
/* (C) 1993-96  Ullrich von Bassewitz                                        */
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



#ifndef _FILEPATH_H
#define _FILEPATH_H



#if defined (DOS) || defined (DOS32) || defined (OS2)
#  include <io.h>
#else
#  include <unistd.h>
#endif

#include "statdef.h"
#include "str.h"



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



void AddPathSep (String& Path);
// Add a trailing path separator if the given path does not end in one

void DelPathSep (String& Path);
// Delete a trailing path separator if the last char of path is one

void AddDefaultExtension (String& PathName, const String& DefExt);
// Add the extension DefExt to PathName if PathName has no extension

void ForceExtension (String& PathName, const String& Ext);
// Force the file name in PathName to have the extension Ext

void FSplit (const String& Pathname, String& Path, String& Name);
// Split a complete path in its components. Name and extension aren't separated

void FSplit (const String& Pathname, String& Path, String& Name, String& Ext);
// Split a complete path in its components

int FExists (const String& Filename, int AccessMode = F_OK);
// Check if the file exists and confirms to the given access mode

String FSearch (String List, const String& File, int AccessMode = F_OK);
// Searches a list of different directories in LIST for the file FILE.
// The directories are separated by ListSep (defined in machine.h). If
// FILE is found in one of those directories, the complete, absolute(!)
// path to the file is returned. Othwerwise, an empty string is returned.

String ShortPath (String S, unsigned Len);
// Try to shorten the filepath in S by replacing one or more subdirectories
// by "...". This is done until the path fits into the given length Len.
// If such an abreviation is not possible, S is returned - so check the length
// of the result before using the returned string.

String TempName ();
// Return a name for a temporary file (this function uses tmpnam)

char GetCurrentDrive ();
// Return the current drive as a letter

String GetCurrentDir ();
// Return the current directory including the current drive if the file system
// supports drives. The returned string includes a trailing path separator!

int FHasWildcards (const String& Pathname);
// Return 1 if the given path contains one of the wildcard characters '*', '?'
// or '[]', return zero otherwise.

int FMatch (const String& Source, const String& Pattern);
// Match the string in Source against Pattern. Pattern may contain the
// wildcards '*', '?', '[abcd]' '[ab-d]', '[!abcd]', '[!ab-d]'
// The function returns a value of zero if Source does not match Pattern,
// otherwise a non zero value is returned.
// If Pattern contains an invalid wildcard pattern (e.g. 'A[x'), the function
// returns zero.

int FIsAbsolute (const String& Path);
// Return true if the given path is an absolute path

int FHasDriveSpec (const String& Path);
// Return 1 if the given path has a drive specification, return 0 otherwise

int CreatePath (const String& Dir);
// Dir is assumed to be a complete directory without a file name. A trailing
// path separator in Dir is ignored. The function tries to create the given
// directory (including all parent directories) if they don't exist.
// The return value is a system error code, it is zero if no error occured.

String CleanPath (String Path);
// Assume Path is a directory (no file!). Make the path absolute including
// a drive if needed and delete "." and ".." parts and add a trailing
// path separator.

String MakeAbsolute (const String& Pathname);
// Assume Pathname is the name of a file with or without a relative or
// absolute path. Make Pathname contain the absolute name of the file.

u32 FSize (const String& Name);
// Return the size of the given file. On error -1 converted to u32 is returned

int FMatch (String Source, String Pattern);
// Use String::Match to check if Source is matched by Pattern. This function
// is different from String::Match in that it ignores case on systems where
// case is ignored in file names. So use this function for matching file
// names instead of the function from module str.



// End of FILEPATH.H

#endif

