/*****************************************************************************/
/*                                                                           */
/*                                PASSWORD.CC                                */
/*                                                                           */
/* (C) 1994-95  Ullrich von Bassewitz                                        */
/*              Zwehrenbuehlstrasse 33                                       */
/*              D-72070 Tuebingen                                            */
/* EMail:       uz@ibb.schwaben.com                                          */
/*                                                                           */
/*****************************************************************************/



// $Id$
//
// $Log$
//
//



#include "password.h"
#include "streamid.h"
#include "msgid.h"
#include "listbox.h"
#include "menue.h"
#include "crcstrm.h"
#include "memstrm.h"
#include "progutil.h"
#include "stdmenue.h"
#include "stdmsg.h"
#include "strcvt.h"
#include "menuedit.h"



/*****************************************************************************/
/*                             Message constants                             */
/*****************************************************************************/



static const u16 msCannotOpenPasswordFile       = MSGBASE_PASSWORD + 0;
static const u16 msCannotReadPasswordFile       = MSGBASE_PASSWORD + 1;
static const u16 msCannotWritePasswordFile      = MSGBASE_PASSWORD + 2;
static const u16 msUserIDEmpty                  = MSGBASE_PASSWORD + 3;
static const u16 msUserNameEmpty                = MSGBASE_PASSWORD + 4;
static const u16 msPasswordEmpty                = MSGBASE_PASSWORD + 5;
static const u16 msUserIDExists                 = MSGBASE_PASSWORD + 6;
static const u16 msInvalidLogin                 = MSGBASE_PASSWORD + 7;



/*****************************************************************************/
/*                                Global data                                */
/*****************************************************************************/



static String _CUN;
extern const String& CUN = _CUN;
static String _CUID;
extern const String& CUID = _CUID;
static u32 _CPL;
extern const u32& CPL = _CPL;



/*****************************************************************************/
/*                      Explicit template instantiation                      */
/*****************************************************************************/



#ifdef EXPLICIT_TEMPLATES
template class Collection<class PasswordEntry>;
template class SortedCollection<class PasswordEntry, String>;
template class ListBox<class PasswordEntry>;
#endif



/*****************************************************************************/
/*                            class PasswordEntry                            */
/*****************************************************************************/



class PasswordEntry: public Streamable {

    friend class PasswordColl;
    friend class PasswordListBox;
    friend void Login (const String&, const String&);
    friend inline void EntryEditor (class PasswordEntry*, int&, int&);
    // EntryEditor is not inline but static, but this way gcc don't displays
    // a warning

protected:
    String      UserName;
    String      UserID;
    String      Password;
    u32         Level;

    void Crypt ();
    void Decrypt ();

    PasswordEntry (StreamableInit);

public:
    PasswordEntry ();
    PasswordEntry (const String& Name, const String& ID, const String& PW, u32 aLevel);

    // Derived from class Streamable
    virtual void Load (Stream&);
    virtual void Store (Stream&) const;
    virtual u16 StreamableID () const;
    static Streamable* Build ();

};



PasswordEntry::PasswordEntry ():
    Level (0)
{
}



inline PasswordEntry::PasswordEntry (StreamableInit):
    UserName (Empty),
    UserID (Empty),
    Password (Empty)
{
}



PasswordEntry::PasswordEntry (const String& Name, const String& ID,
                              const String& PW, u32 aLevel):
    UserName (Name),
    UserID (ID),
    Password (PW),
    Level (aLevel)
{
}



void PasswordEntry::Crypt ()
{
    UserName.Crypt ();
    UserID.Crypt ();
    Password.Crypt ();
    Level ^= Password.Len ();
}



void PasswordEntry::Decrypt ()
{
    UserName.Decrypt ();
    UserID.Decrypt ();
    Password.Decrypt ();
    Level ^= Password.Len ();
}



void PasswordEntry::Load (Stream& S)
{
    S >> UserName >> UserID >> Password >> Level;
    Decrypt ();
}



void PasswordEntry::Store (Stream& S) const
{
    // Because we crypt and later decrypt *this, we cast away constness...
    ((PasswordEntry*) this)->Crypt ();
    S << UserName << UserID << Password << Level;
    ((PasswordEntry*) this)->Decrypt ();
}



u16 PasswordEntry::StreamableID () const
{
    return ID_PasswordEntry;
}



Streamable* PasswordEntry::Build ()
{
    return new PasswordEntry (Empty);
}



/*****************************************************************************/
/*                             class PWLogEntry                              */
/*****************************************************************************/



class PWLogEntry: public Streamable {

public:
    enum _What { Login, Logout, Fail };

protected:
    String      UserName;
    String      UserID;
    u32         Level;
    Time        Now;
    _What       What;

    void Crypt ();
    void Decrypt ();

    PWLogEntry (StreamableInit);
    // Build constructor

public:
    PWLogEntry (const String& Name, const String& ID, u32 aLevel, _What Action);

    // Derived from class Streamable
    virtual void Load (Stream&);
    virtual void Store (Stream&) const;
    virtual u16 StreamableID () const;
    static Streamable* Build ();

};



PWLogEntry::PWLogEntry (const String& Name, const String& ID,
                        u32 aLevel, _What Action):
    UserName (Name),
    UserID (ID),
    Level (aLevel),
    What (Action)
{
}



inline PWLogEntry::PWLogEntry (StreamableInit):
    UserName (Empty),
    UserID (Empty),
    Now (Empty)
{
}



void PWLogEntry::Crypt ()
{
    UserName.Crypt ();
    UserID.Crypt ();
    Level ^= UserID.Len ();
}



void PWLogEntry::Decrypt ()
{
    UserName.Decrypt ();
    UserID.Decrypt ();
    Level ^= UserID.Len ();
}



void PWLogEntry::Load (Stream& S)
{
    u32 Tmp;
    S >> UserName >> UserID >> Level >> Now >> Tmp;
    What = (_What) Tmp;
    Decrypt ();
}



void PWLogEntry::Store (Stream& S) const
{
    // Because we crypt and later decrypt *this, we cast away constness...
    ((PWLogEntry*) this)->Crypt ();
    u32 Tmp = What;
    S << UserName << UserID << Level << Now << Tmp;
    ((PWLogEntry*) this)->Decrypt ();
}



u16 PWLogEntry::StreamableID () const
{
    return ID_PWLogEntry;
}



Streamable* PWLogEntry::Build ()
{
    return new PWLogEntry (Empty);
}



/*****************************************************************************/
/*                            class PasswordColl                             */
/*****************************************************************************/



class PasswordColl: public SortedCollection<PasswordEntry, String> {

protected:
    // Derived from class Collection
    virtual void* GetItem (Stream& S);
    virtual void PutItem (Stream& S, void* Item) const;

    // Derived from class SortedCollection
    virtual int Compare (const String* Key1, const String* Key2);
    virtual const String* KeyOf (const PasswordEntry* Item);

    PasswordColl (StreamableInit);
    // Build constructor


public:
    PasswordColl ();

    // Derived from class Streamable
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    u32 GetLevel (const String& ID, const String& PW);
    // Search for ID in the collection and compare the password. Return 0 if
    // the ID is not found or the password does not match, return the user
    // level otherwise

    int UserIDExists (PasswordEntry* Entry);
    // Check if the user id in Entry exists already in the collection

    PasswordEntry* Extract (int Index);
    // Get the entry from the collection and return it. Delete it from the
    // collection
};



PasswordColl::PasswordColl ():
    SortedCollection<PasswordEntry, String> (50, 10)
{
    ShouldDelete = 1;
}



PasswordColl::PasswordColl (StreamableInit):
    SortedCollection<PasswordEntry, String> (Empty)
// Build constructor
{
}



int PasswordColl::Compare (const String* Key1, const String* Key2)
{
    return ::Compare (*Key1, *Key2);
}



const String* PasswordColl::KeyOf (const PasswordEntry* Item)
{
    return &Item->UserID;
}



u16 PasswordColl::StreamableID () const
{
    return ID_PasswordColl;
}



Streamable* PasswordColl::Build ()
{
    return new PasswordColl (Empty);
}



void* PasswordColl::GetItem (Stream& S)
{
    return (void*) S.Get ();
}



void PasswordColl::PutItem (Stream& S, void* Item) const
{
    S.Put ((PasswordEntry*) Item);
}



u32 PasswordColl::GetLevel (const String& ID, const String& PW)
{
    int Index;
    if (Search (&ID, Index) == 0) {
        // Not found, level is zero
        return 0;
    }
    PasswordEntry* Entry = At (Index);
    return (Entry->Password == PW) ? Entry->Level : 0;
}



int PasswordColl::UserIDExists (PasswordEntry* Entry)
// Check if the user id in Entry exists already in the collection
{
    int Index;
    return (Search (&Entry->UserID, Index));
}



PasswordEntry* PasswordColl::Extract (int Index)
// Get the entry from the collection and return it. Delete it from the
// collection
{
    // Remember old ShouldDelete value and reset it
    int OldShouldDelete = ShouldDelete;
    ShouldDelete = 0;

    // Get the entry
    PasswordEntry* Entry = At (Index);

    // Delete it from the collection
    AtDelete (Index);

    // Reset ShouldDelete value
    ShouldDelete = OldShouldDelete;

    // Return the extracted PasswordEntry
    return Entry;
}



/*****************************************************************************/
/*                           class PasswordListBox                           */
/*****************************************************************************/



class PasswordListBox: public ListBox<PasswordEntry> {

protected:
    virtual void Print (int Index, int X, int Y, u16 Attr);
    // Display one of the listbox entries

public:
    PasswordListBox (const String& aItemText, i16 aID, const Point& aSize,
                     WindowItem* NextItem);
};



inline PasswordListBox::PasswordListBox (const String& aItemText,
                                         i16 aID,
                                         const Point& aSize,
                                         WindowItem* NextItem):
    ListBox<PasswordEntry> (aItemText, aID, aSize, atEditNormal,
                            atEditBar, atEditHigh, NextItem)
{
}



void PasswordListBox::Print (int Index, int X, int Y, u16 Attr)
// Display one of the listbox entries
{
    // Zeiger auf den Eintrag holen
    PasswordEntry* E = Coll->At (Index);

    // Strings bauen
    String Password = E->Password;
    String UserID   = E->UserID;
    String UserName = E->UserName;

    String Line (" ");
    Line += UserID.Trunc (14).Pad (String::Right, 17);
    Line += UserName.Trunc (30).Pad (String::Right, 34);
    Line += Password.Pad (String::Right, 18);
    Line += U32Str (E->Level).Pad (String::Left, 4);

    Owner->Write (X, Y, Line.Pad (String::Right, Size.X), Attr);
}



/*****************************************************************************/
/*                          Registering the classes                          */
/*****************************************************************************/



LINK (PasswordEntry, ID_PasswordEntry);
LINK (PWLogEntry, ID_PWLogEntry);
LINK (PasswordColl, ID_PasswordColl);



/*****************************************************************************/
/*                              PasswordEditor                               */
/*****************************************************************************/



static void EntryEditor (PasswordEntry* E, int& Abort, int& Changed)
// Allow editing of one password entry
{
    // ID's of the menue items
    const miUserID      = 1;
    const miUserName    = 2;
    const miPassword    = 3;
    const miLevel       = 4;

     // Remember the crc of the entry
    u32 OldCRC = GetCRC (E);

    // Store the old data in a memory stream
    MemoryStream MS (256);
    MS << *E;

    // Load the editor window and register all accel keys
    Menue* M = (Menue*) LoadResource ("PASSWORD.EntryEditwindow");
    M->RegisterItemKeys ();

    // Set the menue entries
    M->SetStringValue (miUserID, E->UserID);
    M->SetStringValue (miUserName, E->UserName);
    M->SetStringValue (miPassword, E->Password);
    M->SetLongValue (miLevel, E->Level);

    // Set a new status line and activate the window
    PushStatusLine (siAbort | siAccept | siUpDnCR_Select);
    M->Activate ();

    // Allow editing
    int Done = 0;
    Changed = 0;
    while (!Done) {

        switch (M->GetChoice ()) {

            case 0:
                // Accept or abort
                if (M->GetAbortKey () == vkAbort) {
                    // Editing aborted
                    if (GetCRC (E) != OldCRC) {
                        // Entry has been changed
                        if (AskDiscardChanges () == 2) {
                            MS >> *E;
                            Changed = 0;
                            Abort   = 1;
                            Done    = 1;
                        }
                    } else {
                        Changed = 0;
                        Abort   = 1;
                        Done    = 1;
                    }
                } else {
                    // Accept, check if all entries are valid
                    if (E->UserID.IsEmpty ()) {
                        ErrorMsg (msUserIDEmpty);
                    } else if (E->UserName.IsEmpty ()) {
                        ErrorMsg (msUserNameEmpty);
                    } else if (E->Password.IsEmpty ()) {
                        ErrorMsg (msPasswordEmpty);
                    } else {
                        // Entry is valid
                        Changed = (GetCRC (E) != OldCRC);
                        Abort   = 0;
                        Done    = 1;
                    }
                }
                break;

            case miUserID:
                E->UserID   = M->GetStringValue (miUserID);
                break;

            case miUserName:
                E->UserName = M->GetStringValue (miUserName);
                break;

            case miPassword:
                E->Password = M->GetStringValue (miPassword);
                break;

            case miLevel:
                E->Level    = M->GetLongValue (miLevel);
                break;


        }

    }

    // Delete window and status line
    M->UnregisterItemKeys ();
    delete M;
    PopStatusLine ();

}



static void PasswordEditor (PasswordColl* PC, int& Abort, int& Changed)
// Allow editing of the given password collection
{
    // Create a memory stream and store the old password data
    MemoryStream MS;
    MS << *PC;

    // Store the crc of the collection to determine if any data has changed
    u32 OldCRC = GetCRC (PC);

    // Load the editor window and adjust its size to the screen size
    Menue* M = (Menue*) LoadResource ("PASSWORD.Editorwindow");
    Rect WindowSize = M->OuterBounds ();
    WindowSize.A.Y = 2;
    WindowSize.B.Y = Background->IYSize () - 2;
    M->Resize (WindowSize);

    // Create a listbox and place it in the window
    PasswordListBox* L = new PasswordListBox ("", 2,
                                              Point (WindowSize.XSize (), WindowSize.YSize () - 1),
                                              NULL);
    L->SetColl (PC);
    L->SetPos (0, 1);
    M->AddItem (L);
    L->Select ();

    // Push a new statusline and activate (show) the window
    PushStatusLine (siAbort | siAccept | siInsert | siDelete | siChange);
    M->Activate ();

    // User loop
    int Done = 0;
    Abort = 0;
    while (!Done) {

        // Get a key from the user
        Key K = KbdGet ();

        // Feed the key to the listbox
        L->HandleKey (K);

        // Get the current listbox entry
        int Current = L->GetSelected ();
        PasswordEntry* Entry;
        PasswordEntry* NewEntry;
        int EAbort;
        int EChanged;

        // Look if there's anything left
        switch (K) {

            case vkAbort:
                // Ask if anything has changed
                if (GetCRC (PC) != OldCRC) {
                    if (AskDiscardChanges () == 2) {
                        // Discard changes, end editing
                        PC->DeleteAll ();
                        MS >> *PC;
                        Changed = 0;
                        Abort   = 1;
                        Done    = 1;
                    }
                } else {
                    Changed = 0;
                    Abort   = 1;
                    Done    = 1;
                }
                break;

            case vkAccept:
                Changed = (GetCRC (PC) != OldCRC);
                Abort = 0;
                Done = 1;
                break;

            case kbEnter:
                // Change an entry
                if (Current != -1) {

                    // Get the current entry and delete it from the collection
                    Entry = PC->Extract (Current);

                    // Create a duplicate of the entry
                    NewEntry = Duplicate (Entry);

                    EntryEditor (NewEntry, EAbort, EChanged);
                    if (!EAbort && EChanged) {
                        // Check if the user id already exists
                        if (PC->UserIDExists (NewEntry)) {
                            // The id exists, delete the entry
                            ErrorMsg (msUserIDExists);
                            delete NewEntry;
                            PC->Insert (Entry);
                        } else {
                            delete Entry;
                            L->Insert (NewEntry);
                            L->Reset ();
                        }
                    } else {
                        delete NewEntry;
                        PC->Insert (Entry);
                    }
                }
                break;

            case vkIns:
                // Insert a new entry
                Entry = new PasswordEntry;
                EntryEditor (Entry, EAbort, EChanged);
                if (!EAbort && EChanged) {
                    // Check if the user id already exists
                    if (PC->UserIDExists (Entry)) {
                        // The id exists, delete the entry
                        ErrorMsg (msUserIDExists);
                        delete Entry;
                    } else {
                        // User id is unique, insert it
                        L->Insert (Entry);
                    }
                } else {
                    // Editing was aborted, delete the entry
                    delete Entry;
                }
                break;

            case vkDel:
                // Delete the current entry
                if (Current != -1 && AskAreYouShure () == 2) {
                    L->Delete (Current);
                }
                break;

        }

    }

    // The listbox will delete the owned collection if we don't set it to NULL
    // before deleting the window (including the listbox)
    L->SetColl (NULL);
    delete M;

    // Restore the old status line
    PopStatusLine ();

}



static PasswordColl* ReadPasswordFile (const String& Filename)
// Read the password file and return the password collection. If there is no
// password file, return an empty collection
{
    PasswordColl* PC;

    // Try to open the file
    FileStream S (Filename, "rb");
    if (S.GetStatus () != stOk) {
        // new file, create a collection
        PC = new PasswordColl;
    } else {
        // Load a password collection from the stream and check for errors
        PC = (PasswordColl*) S.Get ();
        if (!PC || S.GetStatus () != stOk) {
            ErrorMsg (msCannotReadPasswordFile, Filename.GetStr ());
        }
    }

    return PC;
}



void PasswordEditor (const String& Filename)
// Loads a password collection from the given file and allows editing users/
// passwords
{
    // Load the password collection from the file
    PasswordColl* PC = ReadPasswordFile (Filename);

    // Allow editing
    int Abort;
    int Changed;
    PasswordEditor (PC, Abort, Changed);

    // If the collection has been changed, store it
    if (!Abort && Changed) {
        FileStream S (Filename, "wb");
        if (S.GetStatus () != stOk) {
            ErrorMsg (msCannotOpenPasswordFile, Filename.GetStr ());
            return;
        }
        S.Truncate ();
        S.Put (PC);
        if (S.GetStatus () != stOk) {
            ErrorMsg (msCannotWritePasswordFile, Filename.GetStr ());
            return;
        }
    }

}



/*****************************************************************************/
/*                               Login/Logout                                */
/*****************************************************************************/



static void ResetLoginData ()
// Reset the login data
{
    _CUN  = "";
    _CUID = "";
    _CPL  = 0;
}



static void Log (const String& Logname, const String& Name, const String& ID,
                 u32 Level, PWLogEntry::_What What)
{
    if (Logname.IsEmpty ()) {
        return;
    }
    FileStream S (Logname);
    S.SeekToEnd ();
    if (S.GetStatus () == stOk) {
        PWLogEntry LE (Name, ID, Level, What);
        S.Put (LE);
    }
}



void Login (const String& PWName, const String& Logname)
// Ask for user id and password and set the variables CUN CUID and CPL
// according to the users password entry.
// If Logname is not empty, a binary log of all login attempts is stored there.
{
    int Abort;

    // Read in the user id
    Menue* M = (Menue*) LoadResource ("PASSWORD.UserID-Window");
    TextEdit* TE = (TextEdit*) M->ForcedItemWithID (1);
    TE->Edit (Abort);
    String UserID = TE->GetValue ();
    delete M;
    if (Abort) {
        return;
    }

    // Read in the password
    M = (Menue*) LoadResource ("PASSWORD.Password-Window");
    PasswordEdit* PE = (PasswordEdit*) M->ForcedItemWithID (1);
    PE->Edit (Abort);
    String Password = PE->GetValue ();
    delete M;
    if (Abort) {
        return;
    }

    // Load the password collection from the file
    PasswordColl* PC = ReadPasswordFile (PWName);

    // Search for the user id
    int Index;
    if (PC->Search (&UserID, Index) == 0 || PC->At (Index)->Password != Password) {
        // Not found, pop up an error message, reset data
        ErrorMsg (msInvalidLogin);
        Log (Logname, "", UserID, 0, PWLogEntry::Fail);
        ResetLoginData ();
        return;
    } else {
        // Found, set the login data, log the login
        PasswordEntry* PE = PC->At (Index);
        _CUN  = PE->UserName;
        _CUID = PE->UserID;
        _CPL  = PE->Level;
        Log (Logname, CUN, CUID, CPL, PWLogEntry::Login);
    }
}



void Logout (const String& Logname)
// Reset the user data. If Logname is not empty, a binary log of the
// login/logout sequences is kept there.
{
    // First, check if a logout is necessary
    if (CPL == 0) {
        return;
    }

    // Log the logout
    if (!Logname.IsEmpty ()) {
        PWLogEntry LE (CUN, CUID, CPL, PWLogEntry::Logout);
        FileStream S (Logname);
        S.Put (LE);
    }

    // Reset the data
    ResetLoginData ();
}



