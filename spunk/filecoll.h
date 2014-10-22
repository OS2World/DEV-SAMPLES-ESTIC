/*****************************************************************************/
/*									     */
/*				  FILECOLL.H				     */
/*									     */
/* (C) 1995	Ullrich von Bassewitz					     */
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



#ifndef __FILECOLL_H
#define __FILECOLL_H



#include "statdef.h"
#include "coll.h"
#include "strmable.h"
#include "str.h"



/*****************************************************************************/
/*				class FileInfo				     */
/*****************************************************************************/



class FileInfo: public Streamable {

private:
    void Init ();
    // Initialize - is called from the constructors

public:
    String	Name;			// Name of the file
    u16		Error;			// Error code after init
    u16		Dev;			// Device the file resides on
    u16		Inode;
    u16		Mode;			// File mode
    u16		LinkCount;
    u16		UID;			// user id of owner
    u16		GID;			// group id of owner
    u16		RDev;
    u32		Size;			// file size in bytes
    u32		ATime;			// last access time
    u32		MTime;			// last modify time
    u32		CTime;			// creation time

public:
    FileInfo (const char* Pathname);
    FileInfo (const String& Pathname);
    FileInfo (const FileInfo& FI);
    FileInfo (StreamableInit);
    // Construct a FileInfo object

    // Member functions derived from class Streamable
    virtual void Load (Stream&);
    virtual void Store (Stream&) const;
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    String PermStr () const;
    // Create a file permission string from the mode

    FileInfo& operator = (const FileInfo& rhs);
    // Assignment operator

    int IsFifo () const;
    // Return true if the file is a FIFO special file

    int IsChr () const;
    // Return true if the file is a character special file

    int IsBlk () const;
    // Return true if the file is a block special file

    int IsDir () const;
    // Return true if the file is a directory

    int IsReg () const;
    // Return true if the file is a regular file

};



inline FileInfo::FileInfo (const String& Pathname):
    Name (Pathname)
{
    Init ();
}



inline FileInfo::FileInfo (const char* Pathname):
    Name (Pathname)
{
    Init ();
}



inline FileInfo::FileInfo (StreamableInit):
    Name (Empty)
{
}



inline int FileInfo::IsFifo () const
// Return true if the file is a FIFO special file
{
    return (Error == 0 && S_ISFIFO (Mode));
}



inline int FileInfo::IsChr () const
// Return true if the file is a character special file
{
    return (Error == 0 && S_ISCHR (Mode));
}



inline int FileInfo::IsBlk () const
// Return true if the file is a block special file
{
    return (Error == 0 && S_ISBLK (Mode));
}



inline int FileInfo::IsDir () const
// Return true if the file is a directory
{
    return (Error == 0 && S_ISDIR (Mode));
}



inline int FileInfo::IsReg () const
// Return true if the file is a regular file
{
    return (Error == 0 && S_ISREG (Mode));
}



/*****************************************************************************/
/*			      class FileInfoColl			     */
/*****************************************************************************/



class FileInfoColl: public SortedCollection<FileInfo, String> {

protected:
    virtual int Compare (const String* Key1, const String* Key2);
    virtual const String* KeyOf (const FileInfo* Item);

public:
    FileInfoColl (int aLimit = 100, int aDelta = 50);
    // Constructor, create an empty collection

    FileInfoColl (StreamableInit);
    // Build constructor

    // Derived from class Streamable
    virtual u16 StreamableID () const;

    int ReadFiles (String Dir, const String& FileSpec,
		   unsigned ModeAnd, unsigned ModeXor);
    // Create a collection of files in the given directory. Files not matching
    // FileSpec are not inserted. The mode of all files found are xor'ed with
    // ModeXor, than and'ed with ModeAnd. If the result is zero, the file is
    // not inserted into the collection. This is a convienient way to choose
    // files by mode.

    int ReadFiles (String Dir);
    // Create a collection of files in the given directory inserting all of the
    // files in this directory regardless of mode and filetype.

};



inline FileInfoColl::FileInfoColl (int aLimit, int aDelta):
    SortedCollection<FileInfo, String> (aLimit, aDelta, 1)
{
}



inline FileInfoColl::FileInfoColl (StreamableInit):
    SortedCollection<FileInfo, String> (Empty)
{
}



// End of FILECOLL.H

#endif


