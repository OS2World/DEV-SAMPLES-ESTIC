/*****************************************************************************/
/*                                                                           */
/*                                 FILESYS.CC                                */
/*                                                                           */
/*                      (C) 1996 MU Softwareentwicklung                      */
/*                                                                           */
/*      Ullrich von Bassewitz                          Michael Peschel       */
/*      Wacholderweg 14                                   Ledergasse 3       */
/*      D-70597 Stuttgart                            D-72555 Metzingen       */
/*      uz@ibb.schwaben.com                      mipe@ibb.schwaben.com       */
/*                                                                           */
/*****************************************************************************/



#include <string.h>
#include <dos.h>

#include <windows.h>

#include "check.h"
#include "str.h"
#include "filepath.h"
#include "filesys.h"



/*****************************************************************************/
/*                   File system & OS dependent constants                    */
/*****************************************************************************/



extern const char FileSysPathSep        = '\\'; // Path separator
extern const char FileSysListSep        = ';';  // Path list separator
extern const FileSysMaxPath             = 259;  // Maximum path length
extern const FileSysMaxDir              = 254;  // Maximum directory length
extern const FileSysMaxName             = 254;  // Maximum file name length
extern const FileSysMaxExt              = 254;  // Maximum extension length (including the dot)



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



static char* FSType (int Drive, char* Buf, unsigned BufSize, ULONG& Error)
{

    char    DriveSpec [5];
    char    VolumeNameBuffer [1024];
    DWORD   nVolumeNameSize = sizeof (VolumeNameBuffer);
    DWORD   MaximumComponentLength;
    DWORD   FileSystemFlags;
    char    FileSystemNameBuffer [10];
    DWORD   nFileSystemNameSize = sizeof (FileSystemNameBuffer);


    // reset Error
    Error = 0;

    // Set up drive
    if (Drive == 0) {
        // No Path or no path in drive
        Drive = FileSysGetDrive ();
    }
    DriveSpec [0] = Drive + 'A' - 1;
    DriveSpec [1] = ':';
    DriveSpec [2] = '\\';
    DriveSpec [3] = '\0';

    if (GetVolumeInformation (DriveSpec,
                              VolumeNameBuffer,
                              nVolumeNameSize,
                              NULL,
                              &MaximumComponentLength,
                              &FileSystemFlags,
                              FileSystemNameBuffer,
                              nFileSystemNameSize) == 0) {
        // We had an error
        Error = 1;
        return NULL;
    }

    // Copy the returned name into the target buffer and return it
    strncpy (Buf, FileSystemNameBuffer, BufSize - 1);
    Buf [BufSize-1] = '\0';
    return Buf;
}



int FileSysGetDrive ()
// Return the current drive. 1 = A:, 2 = B: and so on. On error, -1 is returned.
// This function may be called only if the system supports drives!
{
    char  Buf [FileSysMaxPath];
    DWORD BufSize = sizeof (Buf);

    if (GetCurrentDirectory (BufSize, Buf) == 0) {
        // An error occured
        return -1;
    } else {
        // Completed without errors
        int Drive = Buf [0];
        return (Drive >= 'a') ? Drive - 'a' + 1 : Drive - 'A' + 1;
    }
}



int FileSysSetDrive (unsigned Drive)
// Set the current drive. 1 = A:, 2 = B: and so on. On error, -1 is returned,
// otherwise zero. This function may be called only if the system supports
// drives!
{
    char Buf [4] = " :.";

    // set drive name
    Buf [0] = 'A' + Drive - 1;

    return SetCurrentDirectory (Buf) ? 0 : -1;
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

    if (Error == 0 && strcmp (Info.fsName, "NTFS") == 0) {
        // NTFS file system
        Info.fsMaxPath          = 259;
        Info.fsMaxDir           = 254;
        Info.fsMaxName          = 254;
        Info.fsMaxExt           = 254;
        Info.fsPreservesCase    = 1;
        Info.fsIgnoresCase      = 1;
    } else if (Error == 0 && strcmp (Info.fsName, "HPFS") == 0) {
        // HPFS file system
        Info.fsMaxPath          = 259;
        Info.fsMaxDir           = 254;
        Info.fsMaxName          = 254;
        Info.fsMaxExt           = 254;
        Info.fsPreservesCase    = 1;
        Info.fsIgnoresCase      = 1;
    } else if (Error == 0 && strcmp (Info.fsName, "NFS") == 0) {
        // NFS file system
        Info.fsMaxPath          = 259;
        Info.fsMaxDir           = 254;
        Info.fsMaxName          = 254;
        Info.fsMaxExt           = 254;
        Info.fsPreservesCase    = 1;
        Info.fsIgnoresCase      = 1;
    } else if (Error == 0 || Error == ERROR_NOT_READY) {
        // FAT or some other file system
        Info.fsMaxPath          = 79;
        Info.fsMaxDir           = 65;
        Info.fsMaxName          =  8;
        Info.fsMaxExt           =  4;
        Info.fsPreservesCase    = 0;
        Info.fsIgnoresCase      = 1;
    } else {
        FAIL ("FileSysGetInfo: Requesting info on a non existing drive");
    }

    // Get the current directory.
    if (Error == 0) {
        String S = FileSysCurrentDir (0, Drive);
        strcpy (Info.fsCurDir, S.GetStr ());
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

    // HPFS and NTFS is case preserving
    return (strcmp (Buf, "HPFS") == 0) || (strcmp (Buf, "NTFS") == 0);
}



int FileSysIgnoresCase (int /*Drive*/ )
// Return 1 if the file system on the given drive ignores the case in file
// names. Beware: This is not the same as FileSysPreservesCase! The latter
// will return true if the file system preserves case in file names given,
// but when searching for files, the case can be ignored (this is the case
// with NT's file systems).
{
    // NT does ignore case in file names
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
    int    CurrentDrive;
    char   Buf     [FileSysMaxPath];
    DWORD  BufLen = sizeof (Buf);

    // get current drive
    CurrentDrive = FileSysGetDrive ();

    // check for errors
    if (CurrentDrive < 0) {
        return Dir;
    }

    // If the current drive should be included, get that
    if (Drive == 0) {
        Drive = CurrentDrive;
    }

    // set drive
    if (FileSysSetDrive (Drive) < 0) {
        goto ExitPoint;
    }

    // get dir
    if (GetCurrentDirectory (BufLen, Buf) == 0) {
        goto ExitPoint;
    }

    // set Dir
    Dir += Buf;

    // check if this is the root directory,
    // if not append the trailing '\'
    if (Dir.Len () > 3) {
        Dir += '\\';
    }

    // delete drive if not included
    if (!IncludeDrive) {
        Dir.Del (0, 2);
    }

ExitPoint:
    // back to the actual drive
    FileSysSetDrive (CurrentDrive);

    // Return the result
    return Dir;
}



