/*****************************************************************************/
/*                                                                           */
/*                                 ICDIAG.CC                                 */
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



#include <stdio.h>

#include "eventid.h"
#include "delay.h"
#include "strcvt.h"
#include "coll.h"
#include "datetime.h"
#include "menue.h"
#include "textitem.h"
#include "listbox.h"
#include "menuitem.h"
#include "progutil.h"
#include "settings.h"
#include "winmgr.h"

#include "icobjid.h"
#include "icevents.h"
#include "icintcon.h"
#include "icmsg.h"
#include "devstate.h"
#include "icdlog.h"
#include "icdiag.h"



// Register the class
LINK (MatrixWindow, ID_MatrixWindow);



/*****************************************************************************/
/*                             Message constants                             */
/*****************************************************************************/



const u16 msMatrixWindowTitle   = MSGBASE_ICDIAG +  0;
const u16 msMatrixWindowHeader  = MSGBASE_ICDIAG +  1;



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



// The name of the matrix window position in the settings file
static const String MatrixWindowBounds = "MatrixWindow.Bounds";

// Width of the matrix window
static const unsigned MatrixWindowWidth = 64;



/*****************************************************************************/
/*                      Explicit template instantiation                      */
/*****************************************************************************/



#ifdef EXPLICIT_TEMPLATES
template class Collection<class DevStateInfo>;
template class SortedCollection<class DevStateInfo, unsigned char>;
template class ListBox<class DevStateInfo>;
#endif



/*****************************************************************************/
/*                            class DevStateColl                             */
/*****************************************************************************/



class DevStateColl: public SortedCollection<DevStateInfo, unsigned char> {

protected:
    virtual int Compare (const unsigned char* Key1, const unsigned char* Key2);
    virtual const unsigned char* KeyOf (const DevStateInfo* Item);

public:
    DevStateColl ();
    // Create a DevStateColl

    void DeleteDev (unsigned char Num);
    // Delete the device with the given numer.

    DevStateInfo* NewDev (unsigned char Num);
    // Create and insert a device with the given numer.

    DevStateInfo* GetDevStateInfo (unsigned char Num);
    // Return a pointer to the entry. Calls FAIL if the entry does not exist

    void SetState (unsigned char Dev, unsigned NewState);
    void ClrState (unsigned char Dev, unsigned NewState);
    // Set/reset the device state bits

    DevStateInfo* At (int Index);
    // Return a pointer to the item at position Index.
    // OVERRIDE FOR DEBUGGING

    void SetSchleife (unsigned Dev, int State);
    void SetAmt (unsigned Dev, int State, unsigned Amt);
    void SetInt (unsigned Dev, int State, unsigned Int);
    void SetTon (unsigned Dev, int State);
    void SetWTon (unsigned Dev, int State);
    void SetTFE (unsigned Dev, int State);
    void SetRuf (unsigned Dev, int State);
    // Set specific matrix states

    void AddDigit (unsigned Dev, char Digit);
    // Add a digit to the phone number if the device is in a state where a digit
    // is accepted (dialed)
};



int DevStateColl::Compare (const unsigned char* Key1, const unsigned char* Key2)
{
    if (*Key1 < *Key2) {
        return -1;
    } else if (*Key1 > *Key2) {
        return 1;
    } else {
        return 0;
    }
}



const unsigned char* DevStateColl::KeyOf (const DevStateInfo* Item)
{
    return &Item->DevNum;
}



DevStateColl::DevStateColl ():
    SortedCollection <DevStateInfo, unsigned char> (10, 10, 1)
{
    // Insert 8 devices (more devices are added dynamically if they are
    // detected the first time)
    for (unsigned char Dev = 0; Dev < 8; Dev++) {
        NewDev (Dev);
    }
}



void DevStateColl::DeleteDev (unsigned char Num)
// Delete the device with the given numer.
{
    // Search for the device
    int Index;
    int Result = Search (&Num, Index);
    CHECK (Result != 0);

    // Delete the entry
    AtDelete (Index);
}



DevStateInfo* DevStateColl::NewDev (unsigned char Num)
// Create and insert a device with the given numer.
{
    // Create a new entry
    DevStateInfo* DI = new DevStateInfo (Num);

    // Insert the new entry
    Insert (DI);

    // And return it
    return DI;
}



DevStateInfo* DevStateColl::GetDevStateInfo (unsigned char Num)
// Return a pointer to the entry. Creates an entry if none exists.
{
    // Search for the entry
    int Index;
    if (Search (&Num, Index) == 0) {
        // No entry til now, create one
        return NewDev (Num);
    } else {
        // Found, return it
        return At (Index);
    }
}



void DevStateColl::SetState (unsigned char Dev, unsigned NewState)
// Set the device state bits
{
    GetDevStateInfo (Dev)->SetState (NewState);
}



void DevStateColl::ClrState (unsigned char Dev, unsigned NewState)
// Reset the device state bits
{
    GetDevStateInfo (Dev)->ClrState (NewState);
}



DevStateInfo* DevStateColl::At (int Index)
// Return a pointer to the item at position Index.
// OVERRIDE FOR DEBUGGING
{
    // Check range
    if (Index < 0 || Index >= Count) {
        FAIL ("DevStateColl::At: Index out of bounds");
        return NULL;
    }

    return SortedCollection<DevStateInfo, unsigned char>::At (Index);
}



void DevStateColl::SetSchleife (unsigned Dev, int State)
{
    GetDevStateInfo (Dev)->SetSchleife (State);
}



void DevStateColl::SetAmt (unsigned Dev, int State, unsigned Amt)
{
    GetDevStateInfo (Dev)->SetAmt (State, Amt);
}



void DevStateColl::SetInt (unsigned Dev, int State, unsigned Int)
{
    GetDevStateInfo (Dev)->SetInt (State, Int);
}



void DevStateColl::SetTon (unsigned Dev, int State)
{
    GetDevStateInfo (Dev)->SetTon (State);
}



void DevStateColl::SetWTon (unsigned Dev, int State)
{
    GetDevStateInfo (Dev)->SetWTon (State);
}



void DevStateColl::SetTFE (unsigned Dev, int State)
{
    GetDevStateInfo (Dev)->SetTFE (State);
}



void DevStateColl::SetRuf (unsigned Dev, int State)
{
    GetDevStateInfo (Dev)->SetRuf (State);
}



void DevStateColl::AddDigit (unsigned Dev, char Digit)
// Add a digit to the phone number if the device is in a state where a digit
// is accepted (dialed)
{
    GetDevStateInfo (Dev)->AddDigit (Digit);
}



static DevStateColl DevState;



/*****************************************************************************/
/*                            class MatrixListBox                            */
/*****************************************************************************/



class MatrixListBox: public ListBox<DevStateInfo> {

protected:
    virtual void Print (int Index, int X, int Y, u16 Attr);
    // Display one of the listbox entries

public:
    MatrixListBox (i16 aID, const Point& aSize);
    // Create a matrix listbox

    MatrixListBox (StreamableInit);
    // Create an empty matrix listbox

    virtual ~MatrixListBox ();
    // Destroy a MatrixListBox

    virtual u16 StreamableID () const;
    // Return the streamable ID

    static Streamable* Build ();
    // Build an empty object
};



// Register the class
LINK (MatrixListBox, ID_MatrixListBox);



MatrixListBox::MatrixListBox (i16 aID, const Point& aSize):
    ListBox <DevStateInfo> ("", aID, aSize, atEditNormal, atEditBar, atEditNormal, NULL)
{
    // Set the collection
    SetColl (&DevState);
}



inline MatrixListBox::MatrixListBox (StreamableInit):
    ListBox<DevStateInfo> (Empty)
// Create an empty matrix listbox
{
    // Set the collection
    SetColl (&DevState);
}



MatrixListBox::~MatrixListBox ()
// Destroy a MatrixListBox
{
    // Beware: Before deleting, reset the collection pointer of the listbox
    SetColl (NULL);
}



void MatrixListBox::Print (int Index, int X, int Y, u16 Attr)
{
    // Get the line
    String S = Coll->At (Index)->MatrixLine;

    // Pad the line to length
    S.Pad (String::Right, Size.X);

    // Write out the string
    Owner->Write (X, Y, S, Attr);
}



u16 MatrixListBox::StreamableID () const
// Return the streamable ID
{
    return ID_MatrixListBox;
}



Streamable* MatrixListBox::Build ()
// Build an empty object
{
    return new MatrixListBox (Empty);
}



/*****************************************************************************/
/*                            class MatrixWindow                             */
/*****************************************************************************/



void MatrixWindow::HandleKey (Key& K)
// Key dispatcher used in Browse
{
    // First call the derived function.
    ItemWindow::HandleKey (K);

    // Maybe the listbox has some work
    MatrixBox->HandleKey (K);
}



MatrixWindow::MatrixWindow (const Point& Pos, unsigned aDevCount):
    ItemWindow (Rect (Pos.X, Pos.Y, Pos.X+MatrixWindowWidth+2, Pos.Y+11),
                wfFramed | wfCanMove | wfCanResize | wfSaveVisible),
    MatrixBox (NULL),
    DevCount (aDevCount),
    ZoomSize (OBounds)
{
    // Count of windows must be zero, otherwise this is an error
    CHECK (WindowCount == 0);

    // Lock window output
    Lock ();

    // If there is a stored window size in the settings file, resize the
    // window to the stored rectangle.
    Rect StoredBounds = StgGetRect (MatrixWindowBounds, OBounds);
    if (StoredBounds != OBounds) {
        Resize (StoredBounds);
    }

    // Set the window title
    SetHeader (LoadAppMsg (msMatrixWindowTitle));

    // Create and insert the header line
    TextItem* HdrItem = new TextItem (LoadAppMsg (msMatrixWindowHeader),
                                      100, atTextNormal, NULL);
    AddItem (HdrItem);
    HdrItem->SetWidth (IXSize ());
    HdrItem->SetPos (0, 0);

    // Create a listbox inside the window
    Point Size (IXSize (), IYSize () - 1);
    MatrixBox = new MatrixListBox (1, Size);
    AddItem (MatrixBox);
    MatrixBox->SetPos (0, 1);
    MatrixBox->Draw ();

    // Redraw the window contents
    DrawInterior ();

    // Unlock the window, allowing output
    Unlock ();

    // Ok, we have the window now
    WindowCount++;

    // Tell the application that the window count has changed
    PostEvent (evMatrixWinChange, WindowCount);
}



inline MatrixWindow::MatrixWindow (StreamableInit):
    ItemWindow (Empty)
{
    // One window more
    WindowCount++;

    // Tell the application that the window count has changed
    PostEvent (evMatrixWinChange, WindowCount);
}



MatrixWindow::~MatrixWindow ()
{
    // Store the current window position and size into the settings file
    StgPutRect (OBounds, MatrixWindowBounds);

    // Decrease the window count and invalidate the global pointer
    WindowCount--;

    // Tell the application that the window count has changed
    PostEvent (evMatrixWinChange, WindowCount);
}



void MatrixWindow::Store (Stream& S) const
// Store the object into a stream
{
    // Before storing, be shure to reset the pointer for the device state
    // collection
    MatrixBox->SetColl (NULL);

    // Now use the inherited store
    ItemWindow::Store (S);

    // Reset the collection pointer
    MatrixBox->SetColl (&DevState);

    // Store additional data
    S << DevCount << ZoomSize;
}



void MatrixWindow::Load (Stream& S)
// Load the object from a stream
{
    // Load the derived data
    ItemWindow::Load (S);

    // Load the device count
    S >> DevCount >> ZoomSize;

    // Get a pointer to the listbox
    MatrixBox = (MatrixListBox*) ForcedItemWithID (1);

    // Assign a pointer to the global collection
    MatrixBox->SetColl (&DevState);

    // The inherited Load function did not draw the contents of the listbox
    // since the listbox had no valid data collection at that time. Do the
    // redraw now.
    DrawMatrix ();
}



u16 MatrixWindow::StreamableID () const
// Return the streamable ID
{
    return ID_MatrixWindow;
}



Streamable* MatrixWindow::Build ()
// Build an empty object
{
    return new MatrixWindow (Empty);
}



unsigned MatrixWindow::MinXSize () const
// Return the minimum X size of the window. Override this to limit resizing.
{
    return 7;           // Device number + online flag
}



unsigned MatrixWindow::MinYSize () const
// Return the minumim Y size of the window. Override this to limit resizing.
{
    return 4;           // Header and one device
}



void MatrixWindow::Resize (const Rect& NewBounds)
// Resize the window to the new bounds (this can also be used to move the
// window but Move is faster if the window should not be resized).
{
    // If we have already a matrix listbox, resize it to fit into the new
    // window
    if (MatrixBox) {
        MatrixBox->SetWidth (NewBounds.XSize () - 2);
        MatrixBox->SetHeight (NewBounds.YSize () - 3);
    }

    // Now do the actual resize
    ItemWindow::Resize (NewBounds);
}



void MatrixWindow::Zoom ()
// Zoom the window
{
    // Get the desktop bounds
    Rect Desktop = Background->GetDesktop ();

    // Check if we must zoom in or out
    if (OBounds != Desktop) {
        // Remember the old size, then zoom out
        ZoomSize = OBounds;
        Resize (Desktop);
    } else {
        // Zoom in
        Resize (ZoomSize);
    }
}



void MatrixWindow::DrawMatrix ()
// Redraw the listbox after changes
{
    MatrixBox->Draw ();
}



void MatrixWindow::HandleEvent (Event& E)
// Handle incoming events
{
    // Call the derived function
    ItemWindow::HandleEvent (E);
    if (E.Handled) {
        return;
    }

    // Switch on the type of event
    switch (E.What) {

        case evMatrixChange:
            // Redraw the complete matrix for now
            DrawMatrix ();
            break;

    }
}



/*****************************************************************************/
/*                  Static variables of class MatrixWindow                   */
/*****************************************************************************/



unsigned MatrixWindow::WindowCount = 0;



/*****************************************************************************/
/*                             Helper functions                              */
/*****************************************************************************/



static char MapDigit (unsigned Digit)
// Map a digit code into the corresponding character
{
    // Remap the digit code to a real digit
    switch (Digit) {
        case  1:        return '1';
        case  2:        return '2';
        case  3:        return '3';
        case  4:        return '4';
        case  5:        return '5';
        case  6:        return '6';
        case  7:        return '7';
        case  8:        return '8';
        case  9:        return '9';
        case 10:        return '0';
        case 16:        return 'R';
        default:        return '?';
    }
}



/*****************************************************************************/
/*                          Matrix update functions                          */
/*****************************************************************************/



static void RedrawMatrixCol (unsigned Dev)
{
    // Post an event
    PostEvent (evMatrixChange, (void*)DevState.GetDevStateInfo (Dev));
}



static void WriteDialStatus (unsigned Dev, unsigned Digit)
{
    // Remember the digit, but only if we don't have already a connection
    DevState.AddDigit (Dev, MapDigit (Digit));

    // Update the matrix window
    RedrawMatrixCol (Dev);
}



static void SetMatrix (unsigned Col, unsigned Dev, int State)
{
    // Check which matrix column to set
    switch (Col) {

        case 0:
            DevState.SetAmt (Dev, State, 1);
            break;

        case 1:
            DevState.SetAmt (Dev, State, 2);
            break;

        case 2:
            DevState.SetInt (Dev, State, 1);
            break;

        case 3:
            DevState.SetInt (Dev, State, 2);
            break;

        case 4:
            DevState.SetInt (Dev, State, 3);
            break;

        case 5:
            DevState.SetTon (Dev, State);
            break;

        case 6:
            DevState.SetWTon (Dev, State);
            break;

        case 7:
            DevState.SetTFE (Dev, State);
            break;

        default:
            WriteDebugLog (FormatStr ("Unknown matrix col: %d", Col));
            break;

    }

    // Update the matrix column
    RedrawMatrixCol (Dev);
}



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



static String MsgDesc (const char* S, unsigned Dev, unsigned State)
{
    return FormatStr ("Device %d: %s %s", Dev+21, S, State? "on" : "off");
}



static String DialDesc (const char* S, unsigned Dev, unsigned Digit)
{
    return FormatStr ("Device %d: %s '%c'", Dev+21, S, MapDigit (Digit));
}



static String MatrixDesc (unsigned Dev, unsigned Code, unsigned State)
{
    // Map the code into a real action
    char* Action;
    switch (Code) {
        case 0:         Action = "Line 1";              break;
        case 1:         Action = "Line 2";              break;
        case 2:         Action = "Int 1";               break;
        case 3:         Action = "Int 2";               break;
        case 4:         Action = "Int 3";               break;
        case 5:         Action = "Tone";                break;
        case 6:         Action = "Dial tone";           break;
        case 7:         Action = "TFE";                 break;
        default:        Action = "Unknown matrix col";  break;
    }

    return FormatStr ("Device %d: %s %s", Dev+21, Action, State? "on" : "off");
}



String DiagMsgDesc (const unsigned char* Msg)
// Return a textual description of the given diagnostic message
{
    switch (Msg [1]) {

        case 0x00:
            return "Enable diag mode";

        case 0x01:
            return "Disable diag mode";

        case 0x02:
            // connection matrix
            return MatrixDesc (Msg [3], Msg [2], Msg [4]);

        case 0x03:
            // call
            return MsgDesc ("Call", Msg [2], Msg [4]);

        case 0x04:
            // Schleifenzustand
            return MsgDesc ("Loop", Msg [2], Msg [4]);

        case 0x05:
            // Pulse dial
            return DialDesc ("Pulse dial", Msg [2], Msg [4]);

        case 0x06:
            // Tone dial
            return DialDesc ("Tone Dial", IntCon [Msg [2]], Msg [4]);

        case 0x07:
            // LED state
            return FormatStr ("LED state %s", Msg [4]? "on" : "off");

        case 0x08:
            // Charges
            return "";

        case 0x09:
            // TFE amplifier
            return "";

        case 0x0A:
            // Door opener
            return "";

        case 0x0D:
            // Switch
            return "";

        default:
            // Unknown debug message!
            return "Unknown diagnostic message!";

    }
}



void HandleDiagMsg (const unsigned char* Msg)
// Handle a diagnostic message from the istec
{
    // Check the event type
    String S;
    switch (Msg [1]) {

        case 0x02:
            // connection matrix
            SetMatrix (Msg [2], Msg [3], Msg [4]);
            break;

        case 0x03:
            // call
            DevState.SetRuf (Msg [2], Msg [4]);
            RedrawMatrixCol (Msg [2]);
            break;

        case 0x04:
            // Schleifenzustand
            DevState.SetSchleife (Msg [2], Msg [4]);
            RedrawMatrixCol (Msg [2]);
            break;

        case 0x05:
            // Pulse dial
            WriteDialStatus (Msg [2], Msg [4]);
            break;

        case 0x06:
            // Tone dial
            WriteDialStatus (IntCon [Msg [2]], Msg [4]);
            break;

        case 0x07:
            // LED state
            break;

        case 0x08:
            // Charges
            break;

        case 0x09:
            // TFE amplifier
            break;

        case 0x0A:
            // Door opener
            break;

        case 0x0D:
            // Switch
            break;

        default:
            // Unknown debug message!
            S = FormatStr ("%02x %02x %02x %02x %02x",
                           Msg [1], Msg [2], Msg [3], Msg [4], Msg [5]);
            WriteDebugLog ("Warning: Got unknown diagnostic message: " + S);
            break;

    }
}



