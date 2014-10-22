/*****************************************************************************/
/*                                                                           */
/*                                ICSHORT.CC                                 */
/*                                                                           */
/* (C) 1996-97  Ullrich von Bassewitz                                        */
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



// Handle the short dial numbers



#include "listbox.h"
#include "crcstrm.h"
#include "memstrm.h"
#include "menue.h"
#include "stdmenue.h"
#include "settings.h"
#include "syserror.h"
#include "progutil.h"

#include "icmsg.h"
#include "icobjid.h"
#include "icac.h"
#include "iccti.h"
#include "icconfig.h"
#include "icalias.h"
#include "icshort.h"



// Register the classes
LINK (ShortNumberInfo, ID_ShortNumberInfo);



/*****************************************************************************/
/*                             Message constants                             */
/*****************************************************************************/



const u16 msSigStandard         = MSGBASE_ICSHORT +  0;
const u16 msSigSignal1          = MSGBASE_ICSHORT +  1;
const u16 msSigSignal2          = MSGBASE_ICSHORT +  2;
const u16 msSigSignal3          = MSGBASE_ICSHORT +  3;
const u16 msSigNone             = MSGBASE_ICSHORT +  4;
const u16 msEditHeader          = MSGBASE_ICSHORT +  5;
const u16 msOpenError           = MSGBASE_ICSHORT +  6;
const u16 msInvalidVersion      = MSGBASE_ICSHORT +  7;
const u16 msUnused              = MSGBASE_ICSHORT +  8;
const u16 msShortcut            = MSGBASE_ICSHORT +  9;
const u16 msAutoDial            = MSGBASE_ICSHORT + 10;
const u16 msLock                = MSGBASE_ICSHORT + 11;



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



// Name that the window uses to store it's position/size in the settings file
static const String ShortDialWindowBounds = "ShortDialWindow.Bounds";

// Version number of short numbers in a file
const u32 FileVersion = 100;



/*****************************************************************************/
/*                      Explicit template instantiation                      */
/*****************************************************************************/



#ifdef EXPLICIT_TEMPLATES
template class SortedCollection<ShortNumberInfo, u16>;
template class Collection<ShortNumberInfo>;
template class ListBox<ShortNumberInfo>;
#endif



/*****************************************************************************/
/*                           class ShortNumberInfo                           */
/*****************************************************************************/



ShortNumberInfo::ShortNumberInfo (unsigned aMemory):
    Memory (aMemory),
    Usage (usUnused),
    AutoDial (0),
    Devices (0),
    Signaling (siStandard),
    Locked (0)
// Create a ShortNumberInfo
{
}



void ShortNumberInfo::Load (Stream& S)
// Load the object from a stream
{
    S >> Memory >> Usage >> Number >> AutoDial >> Devices >> Signaling
      >> Locked;
}



void ShortNumberInfo::Store (Stream& S) const
// Store the object into a stream
{
    S << Memory << Usage << Number << AutoDial << Devices << Signaling
      << Locked;
}



u16 ShortNumberInfo::StreamableID () const
// Return the stream ID
{
    return ID_ShortNumberInfo;
}



Streamable* ShortNumberInfo::Build ()
// Return a new instance
{
    return new ShortNumberInfo (Empty);
}



ShortNumberInfo::ShortNumberInfo (const IstecMsg& Msg)
// Create a ShortNumberInfo from an istec message
{
    Unpack (Msg);
}



ShortNumberInfo::ShortNumberInfo (StreamableInit):
    Number (Empty)
// Build constructor
{
}



void ShortNumberInfo::Unpack (const IstecMsg& Msg)
// Unpack the contents of a message into the struct
{
    // This *must* be a correct message
    PRECONDITION (Msg.IsCTIMsg () &&
                  (Msg.At (1) == CTI_QUERY || Msg.At (1) == CTI_ACK) &&
                  Msg.At (2) == CTI_LOAD_NUMBER);

    // Get the data from the message
    Memory       = Msg.At (3);
    unsigned Len = (Msg.At (4) + 1) / 2;        // Chars --> Bytes
    Number       = FromBCD (&Msg.Data [5], Len, 0);
    AutoDial     = Msg.At (5 + Len + 0);
    Devices      = Msg.At (5 + Len + 1);
    Signaling    = InSignal (Msg.At (5 + Len + 2));
    Locked       = Msg.At (5 + Len + 3);

    // Remove one leading dialprefix from the number, then beautify it
    if (Number.NotEmpty ()) {
        if (Number [0] == DialPrefix) {
            Number.Del (0, 1);
            if (Number.Len () > 2 &&
                Number [0] == DialPrefix &&
                Number [1] != DialPrefix) {
                // National number
                unsigned AreaCodeLen = IstecGetAreaCodeLen (Number);
                if (AreaCodeLen > 0) {
                    Number.Ins (AreaCodeLen, '/');
                }
            }
        }
    }

    // Ok, now determine the usage from the data
    if (Number.IsEmpty ()) {
        // Empty number, entry is unused
        Usage     = usUnused;
        AutoDial  = 0;
        Devices   = 0;
        Signaling = siStandard;
        Locked    = 0;
    } else if (AutoDial != 0) {
        // AutoDial is configured for some devices
        Usage = usAutoDial;
    } else if (Devices == 0) {
        // No devices configured, entry is unused
        Usage = usUnused;
    } else {
        // Valid devices, determine if shortcut or lock
        if (Locked) {
            Usage = usLock;
        } else {
            Usage = usShortcut;
        }
    }

    // End of message must be reached now
    CHECK (Msg.At (5 + Len + 4) == CTI_STOP);
}



void ShortNumberInfo::Pack (IstecMsg& Msg) const
// Pack the data into a message ready for download to the istec
{
    // Make a copy of the number with an dial prefix prepended and any non
    // digit chars removed.
    String Phone;
    if (Number.NotEmpty ()) {
        Phone = DialPrefix + Number;
        Phone.Remove ("+-/", rmAll);
    }

    // Stuff for converting the number to BCD
    unsigned char Buf [19];
    unsigned NumBytes = (Phone.Len () + 1) / 2;
    unsigned I = 0;

    // Fill in data that is independent of the usage
    Buf [I++] = CTI_START;
    Buf [I++] = CTI_CONF;
    Buf [I++] = CTI_STORE_NUMBER;
    Buf [I++] = Memory;
    Buf [I++] = Phone.Len ();
    ToBCD (Phone.GetStr (), &Buf [I], NumBytes, 0);
    I += NumBytes;

    // The rest of the data depends on the usage
    switch (GetUsage ()) {

        case usUnused:
            Buf [I++] = 0;                              // AutoDial devices
            Buf [I++] = 0;                              // Devices
            Buf [I++] = OutSignal (GetSignaling ());    // Signal type
            Buf [I++] = 0;                              // Lock
            break;

        case usShortcut:
            Buf [I++] = 0;                              // AutoDial devices
            Buf [I++] = Devices;                        // Devices
            Buf [I++] = OutSignal (GetSignaling ());    // Signal type
            Buf [I++] = 0;                              // Lock
            break;

        case usAutoDial:
            Buf [I++] = AutoDial;                       // AutoDial devices
            Buf [I++] = 0;                              // Devices
            Buf [I++] = OutSignal (GetSignaling ());    // Signal type
            Buf [I++] = 0;                              // Lock
            break;

        case usLock:
            Buf [I++] = 0;                              // AutoDial devices
            Buf [I++] = Devices;                        // Devices
            Buf [I++] = OutSignal (GetSignaling ());    // Signal type
            Buf [I++] = 0xFF;                           // Lock
            break;

        default:
            FAIL ("ShortNumberInfo::Pack: Invalid usage type");

    }

    // Fill in the rest of the data
    Buf [I++] = CTI_STOP;

    // Now put the stuff into the message
    Msg.NewData (I, Buf);
}



unsigned ShortNumberInfo::GetUsage () const
// Get the usage info
{
    if (Number.IsEmpty ()) {
        return usUnused;
    } else {
        return Usage;
    }
}



void ShortNumberInfo::SetNumber (const String& NewNumber)
// Set a new number
{
    Number = IstecBeautifyPhone (NewNumber);
}



unsigned ShortNumberInfo::GetSignaling () const
// Return the signaling info
{
    if (Number.IsEmpty ()) {
        return siStandard;
    } else {
        return Signaling;
    }
}



String ShortNumberInfo::GetAlias () const
// Get the alias for the given number
{
    return ::GetAlias (Number);
}



unsigned ShortNumberInfo::GetDeviceMap () const
// Return a bitmap of devices that are valid for the current usage
{
    // Determine from the usage which bitmap is valid
    unsigned Bitmap = 0;
    switch (GetUsage ()) {
        case usUnused:          Bitmap = 0;             break;
        case usShortcut:        Bitmap = Devices;       break;
        case usAutoDial:        Bitmap = AutoDial;      break;
        case usLock:            Bitmap = Devices;       break;
        default:                FAIL ("ShortNumberInfo::GetDeviceMap: Unknown usage value");
    }
    return Bitmap;
}



void ShortNumberInfo::SetDeviceMap (unsigned NewMap)
// Set a new device map
{
    switch (GetUsage ()) {
        case usUnused:          /* Ignore it */         break;
        case usShortcut:        Devices = NewMap;       break;
        case usAutoDial:        AutoDial = NewMap;      break;
        case usLock:            Devices = NewMap;       break;
        default:                FAIL ("ShortNumberInfo::SetDeviceMap: Unknown usage value");
    }
}



String ShortNumberInfo::DeviceList ()
// Return a list of devices (e.g. "-2--567-")
{
    const unsigned char Mask [8] = {
        0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
    };
    const char Dev [8] = {
        '1', '2', '3', '4', '5', '6', '7', '8'
    };

    // Determine from the usage which bitmap is valid
    unsigned Bitmap = GetDeviceMap ();

    // Convert bitmap into a string
    String S (8);
    for (unsigned I = 0; I < 8; I++) {
        S += Bitmap & Mask [I]? Dev [I] : '-';
    }

    return S;
}



const String& ShortNumberInfo::SignalName ()
// Return the name for a specific signaling type
{
    unsigned MsgNum = 0;
    switch (Signaling) {
        case siStandard:        MsgNum = msSigStandard; break;
        case siSignal1:         MsgNum = msSigSignal1;  break;
        case siSignal2:         MsgNum = msSigSignal2;  break;
        case siSignal3:         MsgNum = msSigSignal3;  break;
        case siNone:            MsgNum = msSigNone;     break;
        default:                FAIL ("ShortNumberInfo::SignalName: Unknown signal type");
    }
    return LoadAppMsg (MsgNum);
}



const String& ShortNumberInfo::UsageName ()
// Return the name for a usage
{
    unsigned MsgNum = 0;
    switch (GetUsage ()) {
        case usUnused:          MsgNum = msUnused;      break;
        case usShortcut:        MsgNum = msShortcut;    break;
        case usAutoDial:        MsgNum = msAutoDial;    break;
        case usLock:            MsgNum = msLock;        break;
        default:                FAIL ("ShortNumberInfo::UsageName: Unknown usage value");
    }
    return LoadAppMsg (MsgNum);
}



/*****************************************************************************/
/*                           class ShortNumberColl                           */
/*****************************************************************************/



int ShortNumberColl::Compare (const u16* Key1, const u16* Key2)
{
    if (*Key1 < *Key2) {
        return -1;
    } else if (*Key1 > *Key2) {
        return 1;
    } else {
        return 0;
    }
}



const u16* ShortNumberColl::KeyOf (const ShortNumberInfo* Item)
// Helpers for managing sort order
{
    return &Item->Memory;
}



ShortNumberColl::ShortNumberColl ():
    SortedCollection<ShortNumberInfo, u16> (100, 50, 1)
{
}



ShortNumberColl::ShortNumberColl (StreamableInit):
    SortedCollection<ShortNumberInfo, u16> (Empty)
{
}



ShortNumberInfo& ShortNumberColl::NewShortNumber (u16 aMemory)
// Create and insert a new short number
{
    // Create a new entry
    ShortNumberInfo* Info = new ShortNumberInfo (aMemory);

    // Insert the new entry...
    Insert (Info);

    // ... and return it
    return *Info;
}



ShortNumberInfo& ShortNumberColl::NewShortNumber (const IstecMsg& Msg)
// Create and insert a new short number
{
    // Create a new entry
    ShortNumberInfo* Info = new ShortNumberInfo (Msg);

    // Insert the new entry...
    Insert (Info);

    // ... and return it
    return *Info;
}



ShortNumberInfo& ShortNumberColl::GetShortNumber (u16 aMemory)
// Get the entry in the specified memory. If it does not exist, it is created.
{
    // Search for the index
    int Index;
    if (Search (&aMemory, Index) == 0) {
        // No entry til now, create one
        return NewShortNumber (aMemory);
    } else {
        // Found an entry, return it
        return *At (Index);
    }
}



/*****************************************************************************/
/*                         class ShortNumberListBox                          */
/*****************************************************************************/



class ShortNumberListBox: public ListBox<ShortNumberInfo> {


protected:
    virtual void Print (int Index, int X, int Y, u16 Attr);
    // Display one of the listbox entries

public:
    ShortNumberListBox (i16 aID, const Point& aSize, WindowItem* NextItem = NULL);

};



ShortNumberListBox::ShortNumberListBox (i16 aID, const Point& aSize,
                                        WindowItem* NextItem):
    ListBox <ShortNumberInfo> ("", aID, aSize, atEditNormal, atEditBar, atEditHigh, NextItem)
{
}



void ShortNumberListBox::Print (int Index, int X, int Y, u16 Attr)
{
    // Get the entry
    ShortNumberInfo* Info = Coll->At (Index);

    // Create a string with the correct length
    String Line (Size.X);

    // Build the line.
    // Nr.  Number______________  Alias___________  Verwendung_  Ger„te__  Signal__
    Line = FormatStr (" %3u  ", Info->GetMemory () + 300);
    Line.Add (Info->GetNumber (), 20).Add ("  ");
    Line.Add (Info->GetAlias (), 16).Add ("  ");
    Line.Add (Info->UsageName (), 11).Add ("  ");
    Line.Add (Info->DeviceList (), 8).Add ("  ");
    Line.Add (Info->SignalName (), 8).Add ("  ");

    // Pad the line to fit into the window
    Line.Pad (String::Right, Size.X - 1);
    Line += ' ';

    // Print the name
    Owner->Write (X, Y, Line, Attr);
}



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



// Item numbers for the short number edit menu
const miNumber      =  10;
const miUsage       =  20;
const miSignaling   =  30;
const miDevice1     = 110;
const miDevice2     = 120;
const miDevice3     = 130;
const miDevice4     = 140;
const miDevice5     = 150;
const miDevice6     = 160;
const miDevice7     = 170;
const miDevice8     = 180;



static void SetValues (Menue* M, const ShortNumberInfo& Data)
// Update the values in the menue from Data
{
    const unsigned char DeviceItems [8] = {
        miDevice1, miDevice2, miDevice3, miDevice4,
        miDevice5, miDevice6, miDevice7, miDevice8
    };
    const unsigned char Mask [8] = {
        0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
    };

    // Get the number. If it is empty, disable "Usage" and "Signaling"
    String Number = Data.GetNumber ();

    // Get the usage type. If it is "Unused", the device items must be
    // disabled
    unsigned Usage = Data.GetUsage ();

    // Set the data
    M->SetStringValue (miNumber, Number);
    M->SetToggleValue (miUsage, Usage);
    M->SetToggleValue (miSignaling, Data.GetSignaling ());

    // Disable items if necessary
    if (Number.IsEmpty ()) {
        M->GrayItem (miUsage);
        M->GrayItem (miSignaling);
    } else {
        M->ActivateItem (miUsage);
        M->ActivateItem (miSignaling);
    }

    // Set the device map
    unsigned DeviceMap = Data.GetDeviceMap ();
    for (unsigned I = 0; I < sizeof (Mask) / sizeof (Mask [0]); I++) {
        M->SetToggleValue (DeviceItems [I], (DeviceMap & Mask [I]) != 0);
        if (Usage == usUnused) {
            M->GrayItem (DeviceItems [I]);
        } else {
            M->ActivateItem (DeviceItems [I]);
        }
    }
}



static void EditShortNumber (ShortNumberInfo& Data)
// Edit short number data.
{
    // Settings name for the window position
    static const String StgPosName = "EditShortNumber.EditMenue.Position";

    // Save the current config into a memory stream in case we want to cancel
    // changes. Calculate the CRC.
    u32 OldCRC = GetCRC (Data);
    MemoryStream SaveStream;
    SaveStream << Data;
    SaveStream.Seek (0);

    // Load the window
    Menue* M = (Menue*) LoadResource ("@ICSHORT.ShortNumberEdit");

    // Set the window header
    M->SetHeader (FormatStr (LoadAppMsg (msEditHeader).GetStr (), Data.GetMemory () + 300));

    // If there is a stored window position, move the window to that position
    Point Pos = StgGetPoint (StgPosName, M->OuterBounds ().A);
    M->MoveAbs (Pos);

    // Transfer the data into the menu
    SetValues (M, Data);

    // Activate the menue
    M->Activate ();

    // New status line
    PushStatusLine (siAbort | siSelectKeys | siChange | siAccept);

    // Allow editing
    int Done = 0;
    String Num;
    while (!Done) {

        // Get a choice from the user
        int Choice = M->GetChoice ();

        // Look what he wants...
        switch (Choice) {

            case miNumber:
                // Set the new number
                Data.SetNumber (M->GetStringValue (miNumber));
                // Other data in the menue needs an update
                SetValues (M, Data);
                break;

            case miUsage + usUnused:
            case miUsage + usShortcut:
            case miUsage + usAutoDial:
            case miUsage + usLock:
                // Set the new usage
                Data.SetUsage (M->GetToggleValue (miUsage));
                // Other data in the menue needs an update
                SetValues (M, Data);
                break;

            case miSignaling + siStandard:
            case miSignaling + siSignal1:
            case miSignaling + siSignal2:
            case miSignaling + siSignal3:
            case miSignaling + siNone:
                Data.SetSignaling (M->GetToggleValue (miSignaling));
                break;

            case miDevice1:
                Data.SetDeviceMap (Data.GetDeviceMap () & ~0x01);
                break;

            case miDevice1+1:
                Data.SetDeviceMap (Data.GetDeviceMap () | 0x01);
                break;

            case miDevice2:
                Data.SetDeviceMap (Data.GetDeviceMap () & ~0x02);
                break;

            case miDevice2+1:
                Data.SetDeviceMap (Data.GetDeviceMap () | 0x02);
                break;

            case miDevice3:
                Data.SetDeviceMap (Data.GetDeviceMap () & ~0x04);
                break;

            case miDevice3+1:
                Data.SetDeviceMap (Data.GetDeviceMap () | 0x04);
                break;

            case miDevice4:
                Data.SetDeviceMap (Data.GetDeviceMap () & ~0x08);
                break;

            case miDevice4+1:
                Data.SetDeviceMap (Data.GetDeviceMap () | 0x08);
                break;

            case miDevice5:
                Data.SetDeviceMap (Data.GetDeviceMap () & ~0x10);
                break;

            case miDevice5+1:
                Data.SetDeviceMap (Data.GetDeviceMap () | 0x10);
                break;

            case miDevice6:
                Data.SetDeviceMap (Data.GetDeviceMap () & ~0x20);
                break;

            case miDevice6+1:
                Data.SetDeviceMap (Data.GetDeviceMap () | 0x20);
                break;

            case miDevice7:
                Data.SetDeviceMap (Data.GetDeviceMap () & ~0x40);
                break;

            case miDevice7+1:
                Data.SetDeviceMap (Data.GetDeviceMap () | 0x40);
                break;

            case miDevice8:
                Data.SetDeviceMap (Data.GetDeviceMap () & ~0x80);
                break;

            case miDevice8+1:
                Data.SetDeviceMap (Data.GetDeviceMap () | 0x80);
                break;

            case 0:
                if (M->GetAbortKey () == vkAbort) {
                    // Abort - ask if we have changes
                    if (GetCRC (Data) != OldCRC) {
                        // We have changes
                        if (AskDiscardChanges () == 2) {
                            // Discard changes, reload from stream
                            SaveStream >> Data;
                            Done = 1;
                        }
                    } else {
                        // No changes
                        Done = 1;
                    }
                } else if (M->GetAbortKey () == vkAccept) {
                    // Accept the changes
                    Done = 1;
                }
                break;

        }

    }

    // Save the current window position
    StgPutPoint (M->OuterBounds ().A, StgPosName);

    // Pop the status line, delete the menue
    PopStatusLine ();
    delete M;
}



void ShortNumberList (ShortNumberColl& ShortNumbers, int& Changed)
// List all short numbers. Allow editing.
{
    // Save the current config into a memory stream in case we want to cancel
    // changes. Calculate the CRC.
    u32 OldCRC = GetCRC (ShortNumbers);
    MemoryStream SaveStream;
    SaveStream << ShortNumbers;
    SaveStream.Seek (0);

    // Load the window
    Menue* Win = (Menue*) LoadResource ("@ICSHORT.ShortNumberWindow");

    // Make the window cover the whole desktop
    Win->Resize (Background->GetDesktop ());

    // Create a listbox inside the window
    Point Size;
    Size.X = Win->IXSize ();
    Size.Y = Win->IYSize () - 2;
    ShortNumberListBox* Box = new ShortNumberListBox (1, Size);
    Box->SetColl (&ShortNumbers);
    Win->AddItem (Box);
    Box->SetPos (0, 2);
    Box->Select ();
    Box->Draw ();
    Win->Activate ();

    // New status line
    PushStatusLine (siAbort | siSelectKeys | siChange | siAccept);

    // Allow choosing an entry
    int Done = 0;
    while (!Done) {

        // Get keyboard input
        Key K = ::KbdGet ();

        // Let the box look for a useful key
        Box->HandleKey (K);

        // Look what's left
        int Selected;
        switch (K) {

            case kbEnter:
                Selected = Box->GetSelected ();
                if (Selected != -1) {
                    EditShortNumber (*ShortNumbers [Selected]);
                    Box->Draw ();
                }
                break;

            case vkAccept:
                // If we had changes, tell the caller
                if (GetCRC (ShortNumbers) != OldCRC) {
                    Changed = 1;
                }
                Done = 1;
                break;

            case vkResize:
                Win->MoveResize ();
                break;

            case vkAbort:
                if (GetCRC (ShortNumbers) != OldCRC) {
                    // We have changes - ask if we should discard them
                    if (AskDiscardChanges () == 2) {
                        // Discard the changes. To do that, reload the old
                        // config data from the memory stream.
                        SaveStream >> ShortNumbers;
                        Done = 1;
                    }
                } else {
                    Done = 1;
                }
                break;

        }

    }

    // Restore the status line
    PopStatusLine ();

    // Set a new collection for the listbox (otherwise the box would try to
    // delete the collection)
    Box->SetColl (NULL);

    // Delete the window
    delete Win;

}



String FileSaveShort (const String& Filename, const ShortNumberColl& Numbers)
// Save short numbers to a file. The function returns an error message or
// the empty string if all is well.
{
    // Open the stream
    FileStream S (Filename, "wb");
    if (S.GetStatus () != stOk) {
        return LoadAppMsg (msOpenError) + GetSysErrorMsg (S.GetErrorInfo ());
    }

    // Put version and config data into the file
    S << FileVersion << Numbers;

    // Success
    return "";
}



String FileLoadShort (const String& Filename, ShortNumberColl& Numbers)
// Load a configuration from a file. The function returns an error message or
// the empty string if all is well.
{
    // Open the stream
    FileStream S (Filename, "rb");
    if (S.GetStatus () != stOk) {
        return LoadAppMsg (msOpenError) + GetSysErrorMsg (S.GetErrorInfo ());
    }

    // Read the version from the file
    u32 Version;
    S >> Version;

    if (Version != FileVersion) {
        return LoadAppMsg (msInvalidVersion);
    }

    // Load the config data
    S >> Numbers;

    // Success
    return "";
}



