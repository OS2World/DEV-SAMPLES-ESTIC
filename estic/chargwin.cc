/*****************************************************************************/
/*									     */
/*				    CHARGWIN.CC				     */
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



// Charge display window



#include "listbox.h"
#include "textitem.h"
#include "settings.h"
#include "progutil.h"

#include "icmsg.h"
#include "icobjid.h"
#include "icevents.h"
#include "iccom.h"
#include "chargwin.h"



// Register the classes
LINK (ChargeWindow, ID_ChargeWindow);



/*****************************************************************************/
/*			       Message constants			     */
/*****************************************************************************/



const u16 msChargeWindowTitle	= MSGBASE_CHARGWIN + 0;
const u16 msChargeWindowHeader	= MSGBASE_CHARGWIN + 1;



/*****************************************************************************/
/*				     Data				     */
/*****************************************************************************/



// Name that the window uses to store it's position/size in the settings file
static const String ChargeWindowBounds = "ChargeWindow.Bounds";

// Count of ChargeWindows
unsigned ChargeWindow::WindowCount = 0;



/*****************************************************************************/
/*			Explicit template instantiation			     */
/*****************************************************************************/



#ifdef EXPLICIT_TEMPLATES
template class Collection<unsigned>;
template class ListBox<unsigned>;
#endif



/*****************************************************************************/
/*			       class ChargeColl				     */
/*****************************************************************************/



class ChargeColl: public Collection<unsigned> {

public:
    ChargeColl (IstecCharges& Charges);
    // Create a ChargeColl

};



ChargeColl::ChargeColl (IstecCharges& Charges):
    Collection<unsigned> (IstecDevCount, 0, 0)
{
    for (unsigned I = 0; I < IstecDevCount; I++) {
	Insert (&Charges [I]);
    }
}



/*****************************************************************************/
/*			      class ChargeListbox			     */
/*****************************************************************************/



class ChargeListbox: public ListBox<unsigned> {

protected:
    virtual void Print (int Index, int X, int Y, u16 Attr);
    // Display one of the listbox entries

public:
    ChargeListbox (i16 aID, const Point& aSize);
    // Create a charge listbox

    virtual void Store (Stream&) const;
    // Store the object into a stream

    virtual void Load (Stream&);
    // Load the object from a stream

    ChargeListbox (StreamableInit);
    // Build constructor

    virtual u16 StreamableID () const;
    // Return the streamable ID

    static Streamable* Build ();
    // Build an empty object

};



ChargeListbox::ChargeListbox (i16 aID, const Point& aSize):
    ListBox<unsigned> ("", aID, aSize, atEditNormal, atEditBar, atEditNormal, NULL)
// Create a charge listbox
{
    // Create and assign the collection
    SetColl (new ChargeColl (Charges));
}



ChargeListbox::ChargeListbox (StreamableInit):
    ListBox<unsigned> (Empty)
// Build constructor
{
}



void ChargeListbox::Store (Stream& S) const
// Store the object into a stream
{
    // Cast away the object constness
    ChargeListbox* THIS = (ChargeListbox*) this;

    // Temporarily replace the collection by a NULL pointer
    Collection<unsigned>* C = THIS->Coll;
    THIS->Coll = NULL;

    // Now store the box
    ListBox<unsigned>::Store (S);

    // Reset the collection pointer
    THIS->Coll = C;
}



void ChargeListbox::Load (Stream& S)
// Load the object from a stream
{
    // Call the derived function
    ListBox<unsigned>::Load (S);

    // Create a new collection
    SetColl (new ChargeColl (Charges));
}



u16 ChargeListbox::StreamableID () const
// Return the streamable ID
{
    return ID_ChargeListbox;
}



Streamable* ChargeListbox::Build ()
// Build an empty object
{
    return new ChargeListbox (Empty);
}



void ChargeListbox::Print (int Index, int X, int Y, u16 Attr)
// Display one of the listbox entries
{
    // Get the line
    String S = FormatStr (" %2d      %2u", Index+21, *(Coll->At (Index)));

    // Pad the line to length
    S.Pad (String::Right, Size.X);

    // Write out the string
    Owner->Write (X, Y, S, Attr);
}



// Register the ChargeListbox
LINK (ChargeListbox, ID_ChargeListbox);



/*****************************************************************************/
/*			       class ChargeWindow			     */
/*****************************************************************************/



ChargeWindow::ChargeWindow ():
    ItemWindow (Rect (32, 10, 32+18, 10+11),
		wfFramed | wfCanMove | wfCanResize | wfSaveVisible),
    ZoomSize (OBounds),
    Box (NULL)
// Construct an ChargeWindow
{
    // Lock window output
    Lock ();

    // If there is a stored window size in the settings file, resize the
    // window to the stored rectangle.
    Rect StoredBounds = StgGetRect (ChargeWindowBounds, OBounds);
    if (StoredBounds != OBounds) {
	Resize (StoredBounds);
    }

    // Set the window title
    SetHeader (LoadAppMsg (msChargeWindowTitle));

    // Create and insert the header line
    TextItem* HdrItem = new TextItem (LoadAppMsg (msChargeWindowHeader),
				      100, atTextNormal, NULL);
    AddItem (HdrItem);
    HdrItem->SetWidth (IXSize ());
    HdrItem->SetPos (0, 0);

    // Create a listbox inside the window
    Point Size (IXSize (), IYSize () - 1);
    Box = new ChargeListbox (1, Size);
    AddItem (Box);
    Box->SetPos (0, 1);
    Box->Draw ();

    // Redraw the window contents
    DrawInterior ();

    // Unlock the window, allowing output
    Unlock ();

    // Ok, we have the window now
    WindowCount++;

    // Tell the application that the window count has changed
    PostEvent (evChargeWinChange, WindowCount);
}



ChargeWindow::ChargeWindow (StreamableInit):
    ItemWindow (Empty)
{
    // One window more
    WindowCount++;

    // Tell the application that the window count has changed
    PostEvent (evChargeWinChange, WindowCount);
}



ChargeWindow::~ChargeWindow ()
// Destruct an ChargeWindow
{
    // Write the current position to the settings file
    StgPutRect (OBounds, ChargeWindowBounds);

    // One window less
    WindowCount--;

    // Tell the application that the window count has changed
    PostEvent (evChargeWinChange, WindowCount);
}



void ChargeWindow::Store (Stream &S) const
// Store the window in a stream
{
    // Store the data from ItemWindow
    ItemWindow::Store (S);

    // Store additional data
    S << ZoomSize;
}



void ChargeWindow::Load (Stream& S)
// Load the window from a stream
{
    // Load the parental data
    ItemWindow::Load (S);

    // Get a pointer to the listbox
    Box = (ChargeListbox*) ForcedItemWithID (1);

    // Load additional data
    S >> ZoomSize;
}



u16 ChargeWindow::StreamableID () const
// Return the stream ID
{
    return ID_ChargeWindow;
}



Streamable* ChargeWindow::Build ()
// Create an empty ChargeWindow
{
    return new ChargeWindow (Empty);
}



unsigned ChargeWindow::MinXSize () const
// Return the minimum X size of the window. Override this to limit resizing.
{
    return 18;
}



unsigned ChargeWindow::MinYSize () const
// Return the minumim Y size of the window. Override this to limit resizing.
{
    return 4;		// One line at least
}



void ChargeWindow::Resize (const Rect& NewBounds)
// Resize the window to the new bounds (this can also be used to move the
// window but Move is faster if the window should not be resized).
{
    // If we have already a matrix listbox, resize it to fit into the new
    // window
    if (Box) {
	Box->SetWidth (NewBounds.XSize () - 2);
	Box->SetHeight (NewBounds.YSize () - 3);
    }

    // Now do the actual resize
    ItemWindow::Resize (NewBounds);
}



void ChargeWindow::Zoom ()
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



void ChargeWindow::HandleKey (Key& K)
// Key dispatcher used in Browse
{
    // First call the derived function.
    ItemWindow::HandleKey (K);

    // Maybe the listbox has some work
    Box->HandleKey (K);
}



void ChargeWindow::HandleEvent (Event& E)
// Handle incoming events.
{
    // Call the derived function
    ItemWindow::HandleEvent (E);
    if (E.Handled) {
	return;
    }

    // Check which event
    switch (E.What) {

	case evChargeUpdate:
	    // Update the listbox
	    Box->Draw ();
	    break;

    }
}



