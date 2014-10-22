/*****************************************************************************/
/*                                                                           */
/*                                  FILEPATH.CC                              */
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



#ifdef DOSLIKE_OS
#  include <stdlib.h>
#  include <io.h>
#  include <direct.h>
#  include <ctype.h>
#else
#  include <unistd.h>
#endif

#include "check.h"
#include "filesys.h"
#include "filecoll.h"
#include "filepath.h"
#include "environ.h"



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



void AddPathSep (String& Path)
// Add a trailing path separator if the given path does not end in one
{
    if (Path.Len () > 0 && Path [Path.Len () - 1] != FileSysPathSep) {
        Path += FileSysPathSep;
    }
}



void DelPathSep (String& Path)
// Delete a trailing path separator if the last char of path is one
{
    // Get the path length
    unsigned Len = Path.Len ();

    // Check if there is a trailing path sep
    if (Len > 0 && Path [Len - 1] == FileSysPathSep) {
        // Beware!
        // * A single path separator means the root directory
        // * A root dir may also contain a drive or volume spec
        if (Len == 1) {
            // Just a root dir spec
            return;
        }
#if defined(FILESYS_HAS_DRIVES)
        if (Len == 3 && Path [1] == ':') {
            // A root dir of a drive
            return;
        }
#endif
        // Ok, we have a real path with a trailing separator
        Path.Trunc (Len - 1);
    }
}



void AddDefaultExtension (String& PathName, const String& DefExt)
// Add the extension DefExt to PathName if PathName has no extension
{
    // Split up the name
    String Dir, Name, Ext;
    FSplit (PathName, Dir, Name, Ext);

    // Check if there is already an extension
    if (Ext.IsEmpty ()) {
        // No extension present - add the default extension
        PathName += DefExt;
    }
}



void ForceExtension (String& PathName, const String& Ext)
// Force the file name in PathName to have the extension Ext
{
    // Split the name into its components
    String Path;
    String Name;
    String OldExt;
    FSplit (PathName, Path, Name, OldExt);

    // Now reassemble the name using the new extension
    PathName = Path + Name + Ext;
}



void FSplit (const String& Pathname, String& Path, String& Name)
// Split a complete path in its components. Name and extension aren't separated
{
    // Beware: On systems that support disk drives, "d:lib" is a valid name
    // that should be splited into "d:" and "lib"

    // Get the position of the rightmost separator char
    int SepPos = Pathname.ScanL (FileSysPathSep);
#if defined(FILESYS_HAS_DRIVES)
    int ColonPos = Pathname.ScanL (':');
    SepPos = SepPos > ColonPos ? SepPos : ColonPos;
#endif

    // Check if we had a separator
    if (SepPos == -1) {

        // No separator, no path
        Path.Clear ();
        Name = Pathname;

    } else {

        // Extract path
        Path = Pathname.Cut (0, SepPos+1);
        Name = Pathname.Cut (SepPos+1, Pathname.Len () - SepPos - 1);

    }
}



void FSplit (const String& Pathname, String& Path, String& Name, String& Ext)
// Split a complete path in its components
{
    // Beware: On systems that support disk drives, "d:lalle.faz" is a valid
    // name that should be splited into "d:", "lalle" and ".faz"

    // Get the position of the rightmost separator char
    int SepPos = Pathname.ScanL (FileSysPathSep);
#if defined(FILESYS_HAS_DRIVES)
    int ColonPos = Pathname.ScanL (':');
    SepPos = SepPos > ColonPos ? SepPos : ColonPos;
#endif

    // Check if we had a separator
    if (SepPos == -1) {

        // No separator, no path
        Path.Clear ();

        // Extract extension
        int DotPos = Pathname.ScanL ('.');
        if (DotPos == -1) {
            // No extension
            Ext.Clear ();
            Name = Pathname;
        } else {
            Name = Pathname.Cut (0, DotPos);
            Ext = Pathname.Cut (DotPos, Pathname.Len () - DotPos);
        }

    } else {

        // Extract path
        Path = Pathname.Cut (0, SepPos+1);

        // Extract extension
        int DotPos = Pathname.ScanL ('.');
        if (DotPos == -1 || DotPos < SepPos) {
            // No extension
            Ext.Clear ();
            Name = Pathname.Cut (SepPos+1, Pathname.Len () - SepPos - 1);
        } else {
            // Extension exists
            Name = Pathname.Cut (SepPos+1, DotPos - SepPos - 1);
            Ext = Pathname.Cut (DotPos, Pathname.Len () - DotPos);
        }

    }
}



int FExists (const String& Filename, int AccessMode)
// Check if the file exists and confirms to the given access mode
{
    return (access (Filename.GetStr (), AccessMode) == 0);
}



String FSearch (String List, const String& File, int AccessMode)
// Searches a list of different directories in LIST for the file FILE.
// The directories are separated by ListSep (defined in machine.h). If
// FILE is found in one of those directories, the complete, absolute(!)
// path to the file is returned. Othwerwise, an empty string is returned.
{
    // Search loop
    while (!List.IsEmpty ()) {

        String Dir;

        // Extract the first directory from the list
        int P = List.Pos (FileSysListSep);
        if (P == -1) {
            // There is only one element left
            Dir = List;
            List.Clear ();      // List is empty now
        } else {
            // More than one element left
            Dir = List.Cut (0, P);
            List.Del (0, P+1);
        }

        // Check if we have got an empty directory (nothing to do in this case)
        if (Dir.IsEmpty ()) {
            continue;
        }

        // Make the path clean, add a path separator
#ifdef NETWARE
        AddPathSep (Dir);
#else
        Dir = CleanPath (Dir);
#endif
        // Check if the file exists
        if (FExists (Dir + File, AccessMode)) {
            // Found !
            return Dir;
        }

    }

    // not found
    return String ();
}



String ShortPath (String S, unsigned Len)
// Try to shorten the filepath in S by replacing one or more subdirectories
// by "...". This is done until the path fits into the given length Len.
// If such an abreviation is not possible, S is returned - so check the length
// of the result before using the returned string.
{
    static const char ReplaceStr []  = "...";
    static const unsigned ReplaceLen = 3;       // Length of ReplaceStr

    if (S.Len () <= Len) {
        // The length is already ok
        return S;
    }

    int I = S.Pos (FileSysPathSep);
    if (I == -1) {
        // There is no subdirectory to replace
        return S;
    }

    int J = I;
    I++;                // position after the path separator

    do {

        // Skip the path separator
        J++;

        // Search for the next one
        while (J < S.Len () && S [J] != FileSysPathSep) {
            J++;
        }
        if (J >= S.Len ()) {
            // No path separator found
            return S;
        }

    // Repeat until the new length is ok
    } while (S.Len () - unsigned (J - I) + ReplaceLen > Len);

    // Ok, delete the substring and replace by the dot sequence
    S.Del (I, J - I);
    S.Ins (I, ReplaceStr);

    // Return the result
    return S;
}



String TempName ()
// Return a name for a temporary file (this function uses tmpnam)
{
    return String (tmpnam (NULL));
}



char GetCurrentDrive ()
// Return the current drive as a letter
{
#if !defined(FILESYS_HAS_DRIVES)
    // Cannot be called if the file system does not support drives
    FAIL ("GetCurrentDrive called on a system without drives");
    return '\0';
#else
    int Drive = FileSysGetDrive ();
    if (FileSysIgnoresCase (Drive)) {
        return Drive - 1 + 'A';
    } else {
        return Drive - 1 + 'a';
    }
#endif
}



String GetCurrentDir ()
// Return the current directory including the current drive if the file system
// supports drives. The returned string includes a trailing path separator!
{
    // Use the file system function
    return FileSysCurrentDir ();
}



int FHasWildcards (const String& Pathname)
// Return 1 if the given path contains one of the wildcard characters '*', '?',
// '[]' or '{}', return zero otherwise.
{
    return Pathname.Pos ('*') >= 0 || Pathname.Pos ('?') >= 0 ||
           Pathname.Pos ('[') >= 0 || Pathname.Pos (']') >= 0 ||
           Pathname.Pos ('{') >= 0 || Pathname.Pos ('}') >= 0;
}



int FIsAbsolute (const String& Path)
// Return true if the given path is an absolute path
{
    // An empty path cannot be absolute
    if (Path.IsEmpty ()) {
        return 0;
    }

    // If the file system supports drives, there may be a leading
    // drive specifier
#if defined(FILESYS_HAS_DRIVES)
    if (Path.Len () > 2 && Path [1] == ':') {
        // look behind the drive specifier
        return Path [2] == FileSysPathSep;
    } else {
        // Look at the first character of the path name
        return Path [0] == FileSysPathSep;
    }
#else
    // If the file system does not support drives, allow the
    // tilde as a representation for the home directory
    if (Path [0] == '~') {
        // Assume that the home directory is absolute, no further checking
        return 1;
    } else {
        // Look at the first character of the path name
        return Path [0] == FileSysPathSep;
    }
#endif
}



#if defined(FILESYS_HAS_DRIVES)
int FHasDriveSpec (const String& Path)
// Return 1 if the given path has a drive specification, return 0 otherwise
{
    return Path.Len () >= 2 && Path [1] == ':';
}
#else
int FHasDriveSpec (const String& /* Path */)
// Return 1 if the given path has a drive specification, return 0 otherwise
{
    return 0;
}
#endif



int CreatePath (const String& /*Dir*/)
// Dir is assumed to be a complete directory without a file name. A trailing
// path separator in Dir is ignored. The function tries to create the given
// directory (including all parent directories) if they don't exist.
// The return value is a system error code, it is zero if no error occured.
{
    return 1;           // ##
}



u32 FSize (const String& Name)
// Return the size of the given file. On error -1 converted to u32 is returned
{
    // Stat the file
    FileInfo FI (Name);

    if (FI.Error != 0 || FI.IsReg () == 0) {
        // Error or file is not a regular file
        return (u32) -1;
    } else {
        return FI.Size;
    }
}



String CleanPath (String Path)
// Assume Path is a directory (no file!). Make the path absolute including
// a drive if needed and delete "." and ".." parts and add a trailing
// path separator.
{
    // Check if path is empty
    if (Path.IsEmpty ()) {
        // This one is easy...
        return GetCurrentDir ();
    }

    // Make the path absolute (remember: Path is not empty!)
#if defined(FILESYS_HAS_DRIVES)

    // Add a drivespec if needed
    if (!FHasDriveSpec (Path)) {
        // Drive is missing
        char Drive [3];
        Drive [0] = GetCurrentDrive ();
        Drive [1] = ':';
        Drive [2] = '\0';
        Path.Ins (0, Drive);
    }

    // Make the path absolute if needed (remember: The drive is already added)
    if (!FIsAbsolute (Path)) {
        // Path is not absolute
        int Drive = Path [0] >= 'a' ? Path [0] - 'a' + 1 : Path [0] - 'A' + 1;
        if (Path.Len () == 2) {
            // Just a drive. Add the working directory for this drive
            Path += FileSysCurrentDir (0, Drive);
        } else {
            Path = Path.Cut (0, 2) + FileSysCurrentDir (0, Drive) +
                   Path.Cut (2, Path.Len () - 2);
        }
    }
#else
    if (Path [0] != FileSysPathSep) {
        // Support a tilde as a replacement for the home directory of the
        // user. This is a unix convention, it is implicitly assumed that
        // operating systems that do not support drives are unix alikes.
        if (Path [0] == '~') {
            // Expand the tilde
            Path.Del (0, 1);
            Path.Ins (0, GetEnvVar ("HOME"));
        } else {
            // No tilde, the path is relativ to the current directory
            Path = GetCurrentDir () + Path;
        }
    }
#endif

    // Add a path separator
    AddPathSep (Path);

    // Path is now absolute, remove "." and ".." sequences
    char F [5];

    // Check for "/./"
    F [0] = FileSysPathSep;
    F [1] = '.';
    F [2] = FileSysPathSep;
    F [3] = '\0';
    int Pos = Path.Pos (F);
    while (Pos != -1) {
        Path.Del (Pos, 2);
        Pos = Path.Pos (F);
    }

    // Check for "/../"
    F [2] = '.';
    F [3] = FileSysPathSep;
    F [4] = '\0';
    Pos = Path.Pos (F);
    while (Pos != -1) {

        // Find the previous path element
        int I = Pos - 1;
        while (I >= 0 && Path [I] != FileSysPathSep) {
            I--;
        }

        // Delete it
        if (I < 0) {
            // Absolute path begins with "/../", just delete it
            Path.Del (Pos, 3);
        } else {
            // Delete "/../" and preceeding dir
            Path.Del (I, Pos - I + 3);
        }

        // Check for next occurance
        Pos = Path.Pos (F);

    }

    // Ok, path is clean
    return Path;
}



String MakeAbsolute (const String& Pathname)
// Assume Pathname is the name of a file with or without a relative or
// absolute path. Make Pathname contain the absolute name of the file.
{
    // Split the given name into path and filename
    String Path, Name;
    FSplit (Pathname, Path, Name);

    // Now clean up the path and reassemble Pathname
    return CleanPath (Path) + Name;
}



int FMatch (String Source, String Pattern)
// Use String::Match to check if Source is matched by Pattern. This function
// is different from String::Match in that it ignores case on systems where
// case is ignored in file names. So use this function for matching file
// names instead of the function from module str.
{
    if (FileSysIgnoresCase ()) {
        // File system ignores case. Use the upper case equivalent of both
        // strings for the match.
        Source.ToUpper ();
        Pattern.ToUpper ();
    }

    return Match (Source, Pattern);
}


