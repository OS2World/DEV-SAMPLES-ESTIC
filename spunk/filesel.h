/*****************************************************************************/
/*									     */
/*				  FILESEL.CC				     */
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



#ifndef _FILESEL_H
#define _FILESEL_H



#include "str.h"
#include "filecoll.h"
#include "textitem.h"
#include "menue.h"



/*****************************************************************************/
/*				   Constants				     */
/*****************************************************************************/



const u32 fsFileMayNotExist	= 0x00000000;
const u32 fsFileMustExist	= 0x00000001;
const u32 fsIgnoreStoredPos	= 0x00010000;	// Ignore settings position



/*****************************************************************************/
/*			      class FileSelector			     */
/*****************************************************************************/



// Forwards
class FileInfoColl;
class FileListBox;



class FileSelector: public Streamable {

protected:
    String		HelpKey;	// Help keyword
    class Menue*	Win;		// The visible part of the selector

    class FileInfoColl* DirColl;	// Directories
    class FileInfoColl* FileColl;	// Files

    class FileListBox*	DirBox;		// Directory listbox
    class FileListBox*	FileBox;	// Filename listbox

    String		DefExt;		// Default extension
    u32			Options;	// File selector options

    String		Path;		// Absolute directory
    String		NameSpec;	// Last namespec used
    u16			ModeAnd;	// Files to show
    u16			ModeXor;	// Files not to show

    TextItem*		PathLine;	// path line
    TextItem*		InfoLine1;	// info line #1
    TextItem*		InfoLine2;	// info line #2



    int HasDefExt () const;
    // Return true if the selector has a default extension

    String FileTimeStr (const FileInfo* FI);
    // Return the date/time of the file as a string for display in the info box

    void ReadFiles ();
    // Read the files according to name spec

    Key BrowseBox (FileListBox* Box);
    // Allow browsing the a listbox. If an entry is selected, return kbNoKey.
    // On abort or hotkey, return the key.

    Key BrowseDirBox ();
    // Allow browsing the dir listbox. If an entry is selected, return kbNoKey.
    // On abort or hotkey, return the key.

    Key BrowseFileBox ();
    // Allow browsing the file listbox. If an entry is selected, return kbNoKey.
    // On abort or hotkey, return the key.

    int EditNameSpec ();
    // Allow editing the name spec and return:
    //
    //	    0	    on abort
    //	    1	    if the value has been accepted but not changed
    //	    2	    if NameSpec/Path has a new value now and this value contains
    //		    wildcard chars or is not a directory
    //	    3	    if the name exists as a file (selection)
    //

    void NewDir (String DirPath);
    // Add DirPath to the path

    void ClearInfo ();
    // Clear the info rectangle

    virtual void ShowFileInfo (const FileInfo* FI);
    // Show information about a file

    virtual void ShowDirInfo (const FileInfo* FI);
    // Show information about a directory

    virtual void ShowSpecialInfo (const FileInfo* FI);
    // Show information about a special file

    void ShowInfo (const FileInfo* FI);
    // Calls one of ShowFileInfo/ShowDirInfo/ShowSpecialInfo


public:
    FileSelector (const String& Header,
		  const String& aDefExt = "",
		  u32 aOptions = fsFileMustExist);
    // Create a file selector with the given header

    virtual ~FileSelector ();
    // Delete the file selector

    String GetChoice (const String& aFileSpec = "",
		      u16 aModeAnd = S_IFREG,
		      u16 aModeXor = 0);
    // Pop up the window and allow selecting a file. aModeAnd and aModeXor are
    // valid only for files, not for directories. The function will return the
    // file choosen or an empty string if the user aborted the dialog with esc.
    // If aFileSpec is empty, the last spec is used (this defaults to "*" and
    // the current directory after init.

    void SetHelpKey (const String& NewKey);
    // Set the help keyword

    const String& GetHelpKey () const;
    // Return the current help keyword

    int HasHelp () const;
    // Return true if the viewer has a valid help key

    void SetDefExt (const String& NewExt);
    // Set the default extension

    void SetOptions (u32 NewOption);
    // Set new file selector options

    void ResetOptions (u32 NewOption);
    // Reset some file selector options
};



inline int FileSelector::HasDefExt () const
// Return true if the selector has a default extension
{
    return DefExt.Len () > 0;
}



inline void FileSelector::SetHelpKey (const String& NewKey)
// Set the help keyword
{
    HelpKey = NewKey;
}



inline const String& FileSelector::GetHelpKey () const
// Return the current help keyword
{
    return HelpKey;
}



inline int FileSelector::HasHelp () const
// Return true if the selector has a valid help key
{
    return HelpKey.Len () > 0;
}



inline void FileSelector::SetOptions (u32 NewOption)
// Set new file selector options
{
    Options |= NewOption;
}



inline void FileSelector::ResetOptions (u32 NewOption)
// Reset some file selector options
{
    Options &= ~NewOption;
}



// End of FILESEL.H

#endif

