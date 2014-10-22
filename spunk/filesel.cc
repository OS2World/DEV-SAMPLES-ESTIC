/*****************************************************************************/
/*                                                                           */
/*                                FILESEL.CC                                 */
/*                                                                           */
/* (C) 1995-96  Ullrich von Bassewitz                                        */
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



#include "msgid.h"
#include "chartype.h"
#include "palette.h"
#include "filesys.h"
#include "filepath.h"
#include "filecoll.h"
#include "listbox.h"
#include "strcvt.h"
#include "menuedit.h"
#include "stdmsg.h"
#include "progutil.h"
#include "thread.h"
#include "settings.h"
#include "filesel.h"



/*****************************************************************************/
/*                             Message constants                             */
/*****************************************************************************/



const u16 msBytes               = MSGBASE_FILESEL +  0;
const u16 msDirectory           = MSGBASE_FILESEL +  1;



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



// Name of the settings resource for the window position
static const String FSelPosName = "FileSelector.FileSelectorWindow.Position";



/*****************************************************************************/
/*                                                                           */
/*****************************************************************************/

//
//        |<-------- 24 -------->|  |<--------- 24 ------->|
//                |<----------------- 42 ----------------->|
//        |<--------------------- 50 --------------------->|
//     |<------------------------ 56 ------------------------>|
//     ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ Open file ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
//     ³                                                      ³
//  1  ³  Path    c:\usr\uz\c\lib\                            ³
//  2  ³  Name    °°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°  ³
//     ³                                                      ³
//  4  ³  Files                     Directories               ³
//  5  ³  °°°°°°°°°°°°°°°°°°°°°°°°  °°°°°°°°°°°°°°°°°°°°°°°°  ³  -
//     ³  °°°°°°°°°°°°°°°°°°°°°°°°  °°°°°°°°°°°°°°°°°°°°°°°°  ³  |
//     ³  °°°°°°°°°°°°°°°°°°°°°°°°  °°°°°°°°°°°°°°°°°°°°°°°°  ³  |
//     ³  °°°°°°°°°°°°°°°°°°°°°°°°  °°°°°°°°°°°°°°°°°°°°°°°°  ³  |
//     ³  °°°°°°°°°°°°°°°°°°°°°°°°  °°°°°°°°°°°°°°°°°°°°°°°°  ³  1
//     ³  °°°°°°°°°°°°°°°°°°°°°°°°  °°°°°°°°°°°°°°°°°°°°°°°°  ³  0
//     ³  °°°°°°°°°°°°°°°°°°°°°°°°  °°°°°°°°°°°°°°°°°°°°°°°°  ³  |
//     ³  °°°°°°°°°°°°°°°°°°°°°°°°  °°°°°°°°°°°°°°°°°°°°°°°°  ³  |
//     ³  °°°°°°°°°°°°°°°°°°°°°°°°  °°°°°°°°°°°°°°°°°°°°°°°°  ³  |
// 14  ³  °°°°°°°°°°°°°°°°°°°°°°°°  °°°°°°°°°°°°°°°°°°°°°°°°  ³  -
//     ³                                                      ³
// 16  ³  Info                                                ³
// 17  ³  °°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°  ³
//     ³  °°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°  ³
//     ³                                                      ³
//     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
//



/*****************************************************************************/
/*                      Explicit template instantiation                      */
/*****************************************************************************/



#ifdef EXPLICIT_TEMPLATES
template class ListBox<FileInfo>;
#endif



/*****************************************************************************/
/*                             class FileListBox                             */
/*****************************************************************************/



class FileListBox: public ListBox<FileInfo> {

protected:
    String      SearchText;
    int         Found;


    virtual void Print (int Index, int X, int Y, u16 Attr);
    // Display one of the listbox entries

public:
    FileListBox (i16 aID, const Point& Size, WindowItem* NextItem = NULL);
    // Construct a FileListBox

    virtual void HandleKey (Key& K);
    // Override ListBox::HandleKey and implement a search function for files.

};



FileListBox::FileListBox (i16 aID, const Point& Size, WindowItem* NextItem):
    ListBox<FileInfo> ("", aID, Size, atEditNormal, atEditBar, atEditHigh, NextItem),
    Found (1)
{
}



void FileListBox::Print (int Index, int X, int Y, u16 Attr)
// Display one of the listbox entries
{
    // Get the entry
    FileInfo* FI = Coll->At (Index);

    // Get the name and pad it to size
    String Line = ' ' + FI->Name;
    if (FI->IsDir ()) {
        Line += FileSysPathSep;
    }
    Line.Pad (String::Right, Size.X);

    // Print the line
    Owner->Write (X, Y, Line, Attr);

}



void FileListBox::HandleKey (Key& K)
// Override ListBox::HandleKey and implement a search function for files.
{
    if (IsPlainKey (K) && !IsCntrl (K)) {

        if (Found == 1 && SearchText.Len () < 255) {

            // This key is ours! Add it to the search string
            SearchText += NLSUpCase (K);

            // Search for a name, beginning with SearchText. We cannot do a
            // binary search here, because we ignore case. So do a linear
            // search, even if it is ineffective.
            unsigned Count = Coll->GetCount ();
            unsigned I = 0;
            while (I < Count) {
                String S = Coll->At (I)->Name;
                S.Trunc (SearchText.Len ());
                S.ToUpper ();
                if (S == SearchText) {
                    break;
                }
                I++;
            }
            if (I < Count) {
                // We found it! Set the selected item
                SetSelected (I);
            } else {
                // We did not find the string. Remember that
                Found = 0;
            }

        }

        // In any case, mark the key as handled
        K = kbNoKey;

    } else {

        // Call the derived function and reset the searcher
        ListBox<FileInfo>::HandleKey (K);
        SearchText.Clear ();
        Found = 1;

    }
}



/*****************************************************************************/
/*                          File selector constants                          */
/*****************************************************************************/



const i16 miName                = 1;
const i16 miFiles               = 3;
const i16 miDirectories         = 4;
const i16 miFileBox             = 5;
const i16 miDirBox              = 6;
const i16 miPathLine            = 10;
const i16 miInfoLine1           = 11;
const i16 miInfoLine2           = 12;



/*****************************************************************************/
/*                            class FileSelector                             */
/*****************************************************************************/



FileSelector::FileSelector (const String& Header, const String& aDefExt, u32 aOptions):
    DirColl (new FileInfoColl),
    FileColl (new FileInfoColl),
    DefExt (aDefExt),
    Options (aOptions),
    Path (GetCurrentDir ()),
    NameSpec ("*")
{
    // Insert a leading dot into DefExt if one is missing and add DefExt to
    // the default name spec if DefExt is not empty
    if (!DefExt.IsEmpty ()) {
        if (DefExt [0] != '.') {
            DefExt.Ins (0, '.');
        }
        NameSpec += DefExt;
    }

    // Load the window from the file
    Win = (Menue*) LoadResource ("FILESEL.FileSelectorWindow");

    // If there is a stored window position and the flag fsIgnoreStoredPos is
    // not set, move the window to that position
    if ((Options & fsIgnoreStoredPos) == 0) {
        Point Pos = StgGetPoint (FSelPosName, Win->OuterBounds ().A);
        Win->MoveAbs (Pos);
    }

    // Set the header
    Win->SetHeader (Header);

    // Get pointers to the text items
    PathLine  = (TextItem*) Win->ItemWithID (miPathLine);
    InfoLine1 = (TextItem*) Win->ItemWithID (miInfoLine1);
    InfoLine2 = (TextItem*) Win->ItemWithID (miInfoLine2);

    // Create and place the listboxes
    FileBox = new FileListBox (miFileBox, Point (24, 10), NULL);
    Win->AddItem (FileBox);
    FileBox->SetPos (2, 5);
    DirBox  = new FileListBox (miDirBox, Point (24, 10), NULL);
    Win->AddItem (DirBox);
    DirBox->SetPos (28, 5);

    // Set the listbox collections
    FileBox->SetColl (FileColl);
    DirBox->SetColl (DirColl);

    // Deactivate the list boxes because they are controlled by the label
    // items
    FileBox->Deactivate ();
    DirBox->Deactivate ();

    // Redraw the window interior
    Win->DrawInterior ();
}



FileSelector::~FileSelector ()
// Delete the file selector
{
    // Save the current window position
    StgPutPoint (Win->OuterBounds ().A, FSelPosName);

    // Delete the window. This will delete the listboxes including both
    // collections
    delete Win;
}



void FileSelector::SetDefExt (const String& NewExt)
// Set the default extension
{
    // Remember the new extension
    DefExt = NewExt;

    if (HasDefExt ()) {

        // Add a leading dot if one is missing
        if (DefExt [0] != '.') {
            DefExt.Ins (0, '.');
        }

        // If the current name spec is '*', assume that the constructor has
        // set this and change it
        if (NameSpec == "*") {
            NameSpec = '*' + DefExt;
        }
    }
}



void FileSelector::ReadFiles ()
// Read the files according to name spec
{
    String Dir, Name;

    // This can last some time
    Window* WaitWin = PleaseWaitWindow ();

    // Display the path in the window, from where the files are read
    PathLine->SetText (ShortPath (Path, 42));

    // Clear the old file and dir collections
    FileColl->DeleteAll ();
    DirColl->DeleteAll ();

    // Read in a complete list of all files.
    FileInfoColl* AllFiles = new FileInfoColl;
    int ErrorCode = AllFiles->ReadFiles (Path);
    if (ErrorCode != 0) {
        // An error occured but AllFiles is valid (but maybe empty). Display
        // an error message and continue.
        SysErrorMsg (ErrorCode);
    }

    // Now sort the files: If the file matches the file mask and the modes,
    // insert it into the file collection. If the file is a directory, insert
    // it into the directory collection. Drop all other files.
    // To avoid copying, set the SouldDelete member of AllFiles to 0, and
    // delete retrieve _all_ entries from AllFiles.
    AllFiles->ShouldDelete = 0;
    i32 Count = AllFiles->GetCount ();
    while (--Count >= 0) {

        // Get the next entry
        FileInfo* FI = AllFiles->At (Count);

        // Is it a directory?
        if (FI->IsDir ()) {

            // Drop the path part of the name
            FSplit (FI->Name, Dir, Name);
            FI->Name = Name;

            // Ignore the current (".") directory
            if (FI->Name != ".") {
                // Insert it into DirColl
                DirColl->Insert (FI);
            } else {
                // Current dir - trash it
                delete FI;
            }

        // Is it a regular file with matching modes?
        } else if (FI->IsReg () && ((FI->Mode ^ ModeXor) & ModeAnd) != 0) {

            // Regular file with matching mode, check name
            FSplit (FI->Name, Dir, Name);
            if (FMatch (Name, NameSpec)) {
                // Drop the path part of the name
                FI->Name = Name;
                FileColl->Insert (FI);
            } else {
                // We do not want this file, delete it
                delete FI;
            }

        } else {

            // Unknown stuff, delete it
            delete FI;

        }

    }

    // Delete the (now empty) list of all files
    delete AllFiles;

    // Reset and redraw the listboxes
    FileBox->Reset ();
    FileBox->Draw ();
    DirBox->Reset ();
    DirBox->Draw ();

    // Clear the info rectangle
    ClearInfo ();

    // Delete the "please wait" window
    delete WaitWin;
}



Key FileSelector::BrowseBox (FileListBox* Box)
// Allow browsing the a listbox. If an entry is selected, return kbNoKey.
// On abort or hotkey, return the key.
{
    // Select the box and make it active
    Win->SelectNewItem (Box);

    // Get the current selection of the box. If the selection is valid, print
    // information on this file/dir, else clear the info rectangle
    ShowInfo (Box->GetSelection ());

    // Remember the selected entry
    int LastSel = Box->GetSelected ();
    int NewSel;

    // Endless loop waiting for input...
    while (1) {

        // Get a key
        Key K = KbdGet ();

        // Handle listbox keys
        Box->HandleKey (K);

        // Look if something is left
        switch (K) {

            case kbNoKey:
                // Box has handled the key. Check if an update of the info
                // area is needed
                NewSel = Box->GetSelected ();
                if (NewSel != LastSel) {
                    LastSel = NewSel;
                    ShowInfo (Box->GetSelection ());
                }
                break;

            case vkAbort:
                return vkAbort;

            case vkResize:
                // Resize request
                Win->MoveResize ();
                break;

            case kbTab:
            case kbShiftTab:
                // Handle tab and reverse tab like reserved keys
                return K;

            case kbEnter:
            case vkAccept:
                // Got a selection
                return kbNoKey;

            default:
                if (K != Box->GetAccelKey () && KeyIsRegistered (K)) {
                    return K;
                }
                break;

        }
    }
}



Key FileSelector::BrowseFileBox ()
// Allow browsing the file listbox. If an entry is selected, return kbNoKey.
// On abort or hotkey, return the key.
{
    // Get a pointer to the corresponding label and select it
    WindowItem* Label = Win->ForcedItemWithID (miFiles);

    // Select the label
    Label->Select ();

    // Allow browsing
    Key K = BrowseBox (FileBox);

    // Deselect the label
    Label->Deselect ();

    // Return the result
    return K;
}



Key FileSelector::BrowseDirBox ()
// Allow browsing the dir listbox. If an entry is selected, return kbNoKey.
// On abort or hotkey, return the key.
{
    // Get a pointer to the corresponding label and select it
    WindowItem* Label = Win->ForcedItemWithID (miDirectories);

    // Select the label
    Label->Select ();

    // Allow browsing
    Key K = BrowseBox (DirBox);

    // Deselect the label
    Label->Deselect ();

    // Return the result
    return K;
}



String FileSelector::FileTimeStr (const FileInfo* FI)
// Return the date/time of the file as a string for display in the info box
{
    Time T (FI->MTime);
    String S = T.DateTimeStr ();
    S.Pad (String::Right, 19);
    return S;
}



void FileSelector::ClearInfo ()
// Clear the info rectangle
{
    // Lock the window
    Win->Lock ();

    // Clear the info lines
    InfoLine1->SetText (EmptyString);
    InfoLine2->SetText (EmptyString);

    // Unlock the window, allow screen output
    Win->Unlock ();
}



void FileSelector::ShowFileInfo (const FileInfo* FI)
// Show information about a file
{
    // Lock the window
    Win->Lock ();

    // Display the complete name in the first line, make shure, the string
    // is not longer than the info area
    String Line1 = ' ' + ShortPath (Path + FI->Name, InfoLine1->GetWidth () - 2);
    InfoLine1->SetText (Line1);

    // Build a string like "drwxrwxrwx  123456789 Bytes   12.12.1995 13:24:25"
    String Line2 (InfoLine2->GetWidth ());
    Line2 = ' ';
    Line2 += FI->PermStr ();
    String Size = U32Str (FI->Size);    // gcc 2.5.8 workaround
    Size.Pad (String::Left, 11);
    Line2 += Size;
    Line2 += LoadMsg (msBytes);
    Line2 += FileTimeStr (FI);

    // Write out the second line
    InfoLine2->SetText (Line2);

    // Unlock the window, allow screen output
    Win->Unlock ();
}



void FileSelector::ShowDirInfo (const FileInfo* FI)
// Show information about a directory
{
    // Lock the window
    Win->Lock ();

    // Display the complete name in the first line, make shure, the string
    // is not longer than the info area
    String Line1 = ' ' + ShortPath (Path + FI->Name, InfoLine1->GetWidth () - 2);
    InfoLine1->SetText (Line1);

    // Build a string like "drwxrwxrwx     Directory      12.12.1995 13:24:25"
    String Line2 (InfoLine2->GetWidth ());
    Line2 = ' ';
    Line2 += FI->PermStr ();
    Line2 += LoadMsg (msDirectory);
    Line2 += FileTimeStr (FI);

    // Write out the second line
    InfoLine2->SetText (Line2);

    // Unlock the window, allow screen output
    Win->Unlock ();
}



void FileSelector::ShowSpecialInfo (const FileInfo* FI)
// Show information about a special file
{
    // Lock the window
    Win->Lock ();

    // Display the complete name in the first line, make shure, the string
    // is not longer than the info area
    String Line1 = ' ' + ShortPath (Path + FI->Name, InfoLine1->GetWidth () - 2);
    InfoLine1->SetText (Line1);

    // Build a string like "drwxrwxrwx  123456789 Bytes   12.12.1995 13:24:25"
    String Line2 (InfoLine2->GetWidth ());
    Line2 = ' ';
    Line2 += FI->PermStr ();
    String Size = U32Str (FI->Size);    // gcc 2.5.8 workaround
    Size.Pad (String::Left, 11);
    Line2 += Size;
    Line2 += LoadMsg (msBytes);
    Line2 += FileTimeStr (FI);

    // Write[Aut the second line
    InfoLine2->SetText (Line2);

    // Unlock the window, allow screen output
    Win->Unlock ();
}



void FileSelector::ShowInfo (const FileInfo* FI)
// Calls one of ShowFileInfo/ShowDirInfo/ShowSpecialInfo
{
    // Distribute to one of the virtual functions
    if (FI == NULL) {
        // Invalid file info, clear the info area
        ClearInfo ();
    } else if (FI->IsReg ()) {
        // Regular file
        ShowFileInfo (FI);
    } else if (FI->IsDir ()) {
        // Directory
        ShowDirInfo (FI);
    } else {
        // Special file
        ShowSpecialInfo (FI);
    }
}



int FileSelector::EditNameSpec ()
// Allow editing the name spec and return:
//
//      0       on abort
//      1       if the value has been accepted but not changed
//      2       if NameSpec/Path has a new value now and this value contains
//              wildcard chars or is not a directory
//      3       if the name exists as a file (selection)
//
{
    // Get a pointer to the edit field
    FileEdit* FE = (FileEdit*) Win->ForcedItemWithID (miName);

    // Select the edit field
    Win->SelectNewItem (FE);

    // Allow editing the filespec
    int Abort;
    FE->Edit (Abort);

    // Set FileSpec and return the correct result
    if (Abort) {
        return 0;
    } else {
        String NewSpec = FE->GetValue ();
        if (NameSpec == NewSpec) {
            // Nothing has changed
            return 1;
        }

        // There are some possibilities now:
        //   * If the name part of the new spec contains wildcard
        //     characters, the dir part must be a directory.
        //   * If the name part does not contain wildcards, assume it is
        //     a directory, if it ends in a path separator char.
        //   * If the name does not contain wildcards, it may also be a
        //     directory. Try to stat the file to check that.
        //   * If the user input ends in a path separator, the FSplit
        //     function returns an empty name part, so the complete
        //     input is automatically handled as a directory.
        //   * If the name part does not contain wildcards is not a
        //     directory and does not exist as a file, but a default
        //     extension exists, check if the name with the default
        //     extension added exists as a file. If yes, assume this as
        //     the selected file.

        // Split the input into a directory and a name part.
        String Dir, Name;
        FSplit (NewSpec, Dir, Name);

        // If the name part is empty, use the last spec used
        if (Name.IsEmpty ()) {
            Name = NameSpec;
        }

        // Assume that the name is non existant or contains wildcards
        int RetVal = 2;

        // If the name part contains wildcards, the dir part must be a
        // directory
        if (FHasWildcards (Name)) {
            // Use the given Dir if it's not empty
            if (!Dir.IsEmpty ()) {
                // value entered contains a directory
                NewDir (Dir);
            }
        } else {

            // Name does not contain wildcards. Add the dir from the new spec
            // to the current path.
            if (!Dir.IsEmpty ()) {
                NewDir (Dir);
            }

            // Stat the file
            FileInfo FI (Path + Name);
            if (FI.Error != 0) {
                // File/Dir does not exist. If the fsFileMustExist flag is
                // not set, add an eventual default extension and accept
                // the input as new selection.
                if ((Options & fsFileMustExist) == 0) {

                    AddDefaultExtension (Name, DefExt);
                    RetVal = 3;

                } else {

                    // The file must exist, but does not. If the name has no
                    // extension but a default extension exists, check if the
                    // name with the default extension added exists as a file.
                    // If yes return this as the selected file.
                    if (HasDefExt ()) {
                        String D, N, E;
                        FSplit (Path + Name, D, N, E);
                        if (E.IsEmpty ()) {
                            // Has no extension! Check if there is a file with
                            // this name
                            FileInfo FO (Path + Name + DefExt);
                            if (FO.Error != 0) {
                                // Not found
                                SysErrorMsg (FO.Error);
                            } else {
                                // Beware: The resulting name may not be the
                                // name of a file!
                                if (FO.IsReg ()) {
                                    // Set up the new name
                                    Name += DefExt;
                                    // Found a selection
                                    RetVal = 3;
                                } else {
                                    // Print the original error message from
                                    // above
                                    SysErrorMsg (FI.Error);
                                }
                            }
                        } else {
                            SysErrorMsg (FI.Error);
                        }
                    } else {
                        SysErrorMsg (FI.Error);
                    }
                }

            } else {

                // The file exists, but it may be a directory. Check that.
                if (FI.IsDir ()) {
                    // The given name is a directory. Handle that.
                    NewDir (Name);
                    Name = NameSpec;        // Last spec used
                } else if (FI.IsReg ()) {
                    // The given name exists as a file.
                    RetVal = 3;
                }
            }
        }

        // Remember the new name spec
        NameSpec = Name;

        // Set the new value for the edit field
        FE->SetValue (NameSpec);

        // Return the result
        return RetVal;
    }
}



void FileSelector::NewDir (String DirPath)
// Add DirPath to the path
{
    if (FIsAbsolute (DirPath)) {

        // Absolute path, replace Path
        Path = CleanPath (DirPath);

    } else {

        // The path is relative but maybe it has a drive spec...
        if (FHasDriveSpec (DirPath)) {
            // DirPath contains a drive spec - make it absolute
            Path = CleanPath (DirPath);
        } else {
            // Relative path without drive spec, add DirPath to the current path
            Path = CleanPath (Path + DirPath);
        }
    }
}



String FileSelector::GetChoice (const String& aFileSpec, u16 aModeAnd, u16 aModeXor)
// Pop up the window and allow selecting a file. aModeAnd and aModeXor are
// valid only for files, not for directories. The function will return the
// file choosen or an empty string if the user aborted the dialog with esc.

{
    // If there is a new filespec given, split and store it
    if (!aFileSpec.IsEmpty ()) {
        // Filespec given split it in name and directory
        FSplit (aFileSpec, Path, NameSpec);
        Path = CleanPath (Path);
    }

    // Store the modes and read the files
    ModeAnd  = aModeAnd;
    ModeXor  = aModeXor;
    ReadFiles ();

    // Get a pointer to the edit line
    FileEdit* NameEdit = (FileEdit*) Win->ForcedItemWithID (miName);

    // Set the flags for the file editline
    NameEdit->SetFileFlags (ffPathOk | ffWildcardsOk | ffExtensionOk);

    // Set the current value for the file edit line
    NameEdit->SetValue (NameSpec);

    // Register the window keys
    Win->RegisterItemKeys ();

    // Remember the old state and make the window active
    unsigned OldState = Win->GetState ();
    Win->Activate ();

    // Show a new status line
    PushStatusLine (HasHelp () ? siHelp | siAbort : siAbort);

    // Select the name as active item
    Win->SelectNewItem (miName);
    WindowItem* ActiveItem = NameEdit;

    String Result;
    int Sel;
    Key K;
    int Done = 0;
    while (!Done) {

        switch (ActiveItem->GetID ()) {

            case miName:
                switch (EditNameSpec ()) {

                    case 0:
                        // Abort
                        Done = 1;
                        break;

                    case 1:
                        // No change. If there are files, edit the filebox,
                        // else edit the dirbox
                        if (FileBox->GetCount () > 0) {
                            ActiveItem = FileBox;
                        } else if (DirBox->GetCount () > 0) {
                            ActiveItem = DirBox;
                        }
                        break;

                    case 2:
                        // New file spec, read new files
                        ReadFiles ();
                        // If there are files, edit the filebox, else edit the
                        // dirbox
                        if (FileBox->GetCount () > 0) {
                            ActiveItem = FileBox;
                        } else if (DirBox->GetCount () > 0) {
                            ActiveItem = DirBox;
                        }
                        break;

                    case 3:
                        // Selection
                        Result = Path + NameSpec;
                        Done = 1;
                        break;
                }
                break;

            case miFileBox:
                K = BrowseFileBox ();
                switch (K) {

                    case kbNoKey:
                        // Got a selection
                        Sel = FileBox->GetSelected ();
                        if (Sel != -1) {
                            Result = Path + FileColl->At (Sel) -> Name;
                            Done = 1;
                        }
                        break;

                    case vkAbort:
                        Done = 1;
                        break;

                    default:
                        KbdPut (K);
                        break;

                }
                break;

            case miDirBox:
                K = BrowseDirBox ();
                switch (K) {

                    case kbNoKey:
                        // Got a selection
                        Sel = DirBox->GetSelected ();
                        if (Sel != -1) {
                            // Change the directory
                            NewDir (DirColl->At (Sel) -> Name);

                            // Read in the files
                            ReadFiles ();

                            // If the dir box is empty but the file box is not,
                            // change the active item to the file box
                            if (DirBox->GetCount () == 0) {
                                if (FileBox->GetCount () > 0) {
                                    ActiveItem = FileBox;
                                } else {
                                    ActiveItem = NameEdit;
                                }
                            }
                        }
                        break;

                    case vkAbort:
                        Done = 1;
                        break;

                    default:
                        KbdPut (K);
                        break;

                }
                break;

        }

        // Check for any keys
        if (CurThread () -> KbdKeyAvail ()) {

            // There is a key - get it
            Key K = KbdGet ();

            // Handle the key, drop it if unknown
            if (K == Win->GetAccelKey (miName)) {
                ActiveItem = NameEdit;
            } else if (K == Win->GetAccelKey (miFiles)) {
                ActiveItem = FileBox;
            } else if (K == Win->GetAccelKey (miDirectories)) {
                ActiveItem = DirBox;
            } else if (K == kbTab) {
                if (ActiveItem == DirBox) {
                    ActiveItem = FileBox;
                } else if (ActiveItem == FileBox) {
                    ActiveItem = DirBox;
                }
            } else if (K == kbShiftTab) {
                if (ActiveItem == DirBox) {
                    ActiveItem = FileBox;
                } else if (ActiveItem == FileBox) {
                    ActiveItem = DirBox;
                }
            } else if (KeyIsRegistered (K)) {
                KbdPut (K);
                Done = 1;
            }
        }

    }

    // Restore the old status line
    PopStatusLine ();

    // Unregister the menue keys
    Win->UnregisterItemKeys ();

    // Restore the window state
    Win->SetState (OldState);

    // Free the list of files/directories
    FileColl->DeleteAll ();
    DirColl->DeleteAll ();

    // Return the selected file
    return Result;
}


