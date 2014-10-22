/*****************************************************************************/
/*									     */
/*				  FILECOLL.CC				     */
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



// This module produces warnings about unreachable code under DOS and OS/2.
// You can safely ignore this warnings.



#include <errno.h>
#ifdef __WATCOMC__
#  include <direct.h>
#else
#  include <sys/types.h>
#  include <dirent.h>
#endif

#include "streamid.h"
#include "filesys.h"
#include "filepath.h"
#include "filecoll.h"



// Register the classes
LINK (FileInfo, ID_FileInfo);
LINK (FileInfoColl, ID_FileInfoColl);



/*****************************************************************************/
/*			Explicit template instantiation			     */
/*****************************************************************************/



#ifdef EXPLICIT_TEMPLATES
template class Collection<FileInfo>;
template class SortedCollection<FileInfo, String>;
#endif



/*****************************************************************************/
/*				class FileInfo				     */
/*****************************************************************************/



void FileInfo::Init ()
// Initialize - is called from the constructors
{
    // Try to stat the file
    struct stat Buf;
    if (stat (Name.GetStr (), &Buf) != 0) {
	Error = errno;
	return;
    }

    // Reset the error code and transfer the information to the object
    Error	= 0;
    Dev		= Buf.st_dev;
    Inode	= Buf.st_ino;
    Mode	= Buf.st_mode;
    LinkCount	= Buf.st_nlink;
    UID		= Buf.st_uid;
    GID		= Buf.st_gid;
    RDev	= Buf.st_rdev;
    Size	= Buf.st_size;
    ATime	= Buf.st_atime;
    MTime	= Buf.st_mtime;
    CTime	= Buf.st_ctime;
}



FileInfo::FileInfo (const FileInfo& FI):
    Name (FI.Name),
    Error (FI.Error),
    Dev (FI.Dev),
    Inode (FI.Inode),
    Mode (FI.Mode),
    LinkCount (FI.LinkCount),
    UID (FI.UID),
    GID (FI.GID),
    RDev (FI.RDev),
    Size (FI.Size),
    ATime (FI.ATime),
    MTime (FI.MTime),
    CTime (FI.CTime)
{
}



FileInfo& FileInfo::operator = (const FileInfo& rhs)
// Assignment operator
{
    // Beware! Ignore FI = FI
    if (&rhs != this) {
	Name	  = rhs.Name;
	Error	  = rhs.Error;
	Dev	  = rhs.Dev;
	Inode	  = rhs.Inode;
	Mode	  = rhs.Mode;
	LinkCount = rhs.LinkCount;
	UID	  = rhs.UID;
	GID	  = rhs.GID;
	RDev	  = rhs.RDev;
	Size	  = rhs.Size;
	ATime	  = rhs.ATime;
	MTime	  = rhs.MTime;
	CTime	  = rhs.CTime;
    }
    return *this;
}



void FileInfo::Load (Stream& S)
{
    S >> Name >> Error >> Dev >> Inode >> Mode >> LinkCount >> UID >> GID
      >> RDev >> Size >> ATime >> MTime >> CTime;
}



void FileInfo::Store (Stream& S) const
{
    S << Name << Error << Dev << Inode << Mode << LinkCount << UID << GID
      << RDev << Size << ATime << MTime << CTime;
}



u16 FileInfo::StreamableID () const
{
    return ID_FileInfo;
}



Streamable* FileInfo::Build ()
{
    return new FileInfo (Empty);
}



String FileInfo::PermStr () const
// Create a file permission string from the mode
{
    char S [11];

    // Calling this function with errors is invalid
    PRECONDITION (Error == 0);

    // Set file type
    if (S_ISREG (Mode)) {
	S [0] = '-';
    } else if (S_ISDIR (Mode)) {
	S [0] = 'd';
#ifdef UNIXLIKE_OS
    } else if (S_ISLNK (Mode)) {
	S [0] = 'l';
    } else if (S_ISFIFO (Mode)) {
	S [0] = 'f';
    } else if (S_ISSOCK (Mode)) {
	S [0] = 's';
    } else if (S_ISBLK (Mode)) {
	S [0] = 'b';
#endif
    } else if (S_ISCHR (Mode)) {
	S [0] = 'c';
    }

    S [1] = (Mode & S_IRUSR) ? 'r' : '-';
    S [2] = (Mode & S_IWUSR) ? 'w' : '-';
    S [3] = (Mode & S_ISUID) ? 's' : (Mode & S_IXUSR) ? 'x' : '-';
    S [4] = (Mode & S_IRGRP) ? 'r' : '-';
    S [5] = (Mode & S_IWGRP) ? 'w' : '-';
    S [6] = (Mode & S_ISGID) ? 's' : (Mode & S_IXGRP) ? 'x' : '-';
    S [7] = (Mode & S_IROTH) ? 'r' : '-';
    S [8] = (Mode & S_IWOTH) ? 'w' : '-';
    S [9] = (Mode & S_ISVTX) ? 't' : (Mode & S_IXOTH) ? 'x' : '-';
    S [10] = '\0';

    // return the result
    return String (S);
}



/*****************************************************************************/
/*			      class FileInfoColl			     */
/*****************************************************************************/



int FileInfoColl::ReadFiles (String Dir,
			     const String& FileSpec,
			     unsigned ModeAnd,
			     unsigned ModeXor)
// Create a collection of files in the given directory. Files not matching
// FileSpec are not inserted. The mode of all files found are xor'ed with
// ModeXor, than and'ed with ModeAnd. If the result is zero, the file is
// not inserted into the collection. This is a convienient way to choose
// files by mode.
{
    // Delete the old contents of the collection
    DeleteAll ();

    // If the directory is empty, use the current dir
    if (Dir.IsEmpty ()) {
	Dir = GetCurrentDir ();
    }

    // Delete a trailing path separator
    DelPathSep (Dir);

    // Remember if the file system, the directory resides on, preserves the
    // case of the names
    int PreservesCase = FileSysPreservesCase (Dir);

    // Open the directory stream
    DIR* D;
#if (defined (DOS) && (__BCPLUSPLUS__==0x310)) || (defined (DOS32) && defined (__GO32__))
    // BC++ 3.1 for DOS and djgpp both have a buggy declaration for opendir():
    // the parameter is not declared as const. I'll assume that the function
    // will not change the name and cast away the constness (*sigh*)
    if ((D = opendir ((char*) Dir.GetStr ())) == NULL) {
#else
    if ((D = opendir (Dir.GetStr ())) == NULL) {
#endif
	// Invalid directory
	return errno;
    }

    // Ok, work with Dir is done, add the path separator again
    AddPathSep (Dir);

    // Read one name after the other
    int ErrorCode = 0;
    dirent* Entry;
    while ((Entry = readdir (D)) != NULL) {
	if (Match (Entry->d_name, FileSpec) != 0) {
	    // File name matches the given filespec
	    FileInfo* FI = new FileInfo (Dir + Entry->d_name);
	    if (FI->Error != 0) {
		// An error occured
		ErrorCode = FI->Error;
		delete FI;
	    } else if (((FI->Mode ^ ModeXor) & ModeAnd) == 0) {
		// We are not interested in this file
		delete FI;
	    } else {
		// Ok, we want that file. If the file system does not preserve
		// the case of names, convert the file name to lower case
		if (!PreservesCase) {
		    FI->Name.ToLower ();
		}
		Insert (FI);
	    }
	}
    }

    // Close the directory stream
    closedir (D);

    // return the error code
    return ErrorCode;
}



int FileInfoColl::ReadFiles (String Dir)
// Create a collection of files in the given directory inserting all of the
// files in this directory regardless of mode and filetype.
{
    // Delete the old contents of the collection
    DeleteAll ();

    // If the directory is empty, use the current dir
    if (Dir.IsEmpty ()) {
	Dir = GetCurrentDir ();
    }

    // Make the given directory clean and absolute
    Dir = CleanPath (Dir);

    // Delete a trailing path separator
    DelPathSep (Dir);

    // Remember if the file system, the directory resides on, preserves the
    // case of the names
    int PreservesCase = FileSysPreservesCase (Dir);

    // Open the directory stream
    DIR* D;
#if (defined (DOS) && (__BCPLUSPLUS__==0x310)) || (defined (DOS32) && defined (__GO32__))
    // BC++ 3.1 for DOS and djgpp both have a buggy declaration for opendir():
    // the parameter is not declared as const. I'll assume that the function
    // will not change the name and cast away the constness (*sigh*)
    if ((D = opendir ((char*) Dir.GetStr ())) == NULL) {
#else
    if ((D = opendir (Dir.GetStr ())) == NULL) {
#endif
	// Invalid directory
	return errno;
    }

#if defined (DOS) || defined (DOS32) || defined (OS2)
    // Check and remember if we are in the root and first level dirs. This
    // will be needed for tweaking DOS & OS/2 later.
    int RootLevel = Dir.Len () == 3;	// Since Dir is clean, this is ok
    String Root, N;
    FSplit (Dir, Root, N);
    int SecondLevel = Root.Len () == 3 && N.Len () > 0;
#endif

    // Ok, work with Dir is done, add the path separator again
    AddPathSep (Dir);

    // Read one name after the other
    int ErrorCode = 0;
    dirent* Entry;
    while ((Entry = readdir (D)) != NULL) {

#if defined (DOS) || defined (DOS32) || defined (OS2)
	// On OS/2 systems, the root directory of HPFS drives contains a ".."
	// directory that points back to the root directory. Calling stat
	// with such a name fails. So check for this before creating a new
	// FileInfo entry.
	// Another problem under DOS & OS/2 are the entries ".." in second
	// level directories. Calling stat with an argument of "C:\X\.."
	// will fail, but "C:\" (which is the correct name) is ok, since it
	// is handled in a special way by the stat function (so one can say
	// that this is a stat bug, stat should do also a special handling
	// on names like C:\X\..). To enable the special handling by stat,
	// use the name of the root directory in this case, but do not
	// forget to set the correct name.
	FileInfo* FI;
	if (strcmp (Entry->d_name, "..") == 0) {
	    if (RootLevel) {
		// This is really the root dir, ignore the ".." entry
		continue;
	    }
	    if (SecondLevel) {
		FI = new FileInfo (Root);
		FI->Name = "..";	// Use the correct name!
	    } else {
		// Not a special directory
		FI = new FileInfo (Dir + Entry->d_name);
	    }
	} else {
	    // Normal file, stat it
	    FI = new FileInfo (Dir + Entry->d_name);
	}
#else
	// This is the way to do it under a _real_ operating system
	FileInfo* FI = new FileInfo (Dir + Entry->d_name);
#endif

	// If the file has no error, insert it
	if (FI->Error != 0) {
	    // An error occured
	    ErrorCode = FI->Error;
	    delete FI;
	} else {
	    // Ok, we want that file. If the file system does not preserve
	    // the case of names, convert the file name to lower case
	    if (!PreservesCase) {
		FI->Name.ToLower ();
	    }
	    Insert (FI);
	}
    }

    // Close the directory stream
    closedir (D);

    // return the error code
    return ErrorCode;
}



int FileInfoColl::Compare (const String* Key1, const String* Key2)
{
    return ::Compare (*Key1, *Key2);
}



const String* FileInfoColl::KeyOf (const FileInfo* Item)
{
    return &Item->Name;
}



u16 FileInfoColl::StreamableID () const
{
    return ID_FileInfoColl;
}



