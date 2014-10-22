/*****************************************************************************/
/*									     */
/*				  ICMSGWIN.CC				     */
/*									     */
/* (C) 1996	Ullrich von Bassewitz					     */
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



// A window that holds a header and an area for messages



#include "coll.h"
#include "textitem.h"
#include "settings.h"
#include "progutil.h"

#include "icmsg.h"
#include "icobjid.h"
#include "icmsgwin.h"



/*****************************************************************************/
/*			      class StringMsgColl			     */
/*****************************************************************************/



class StringMsgColl: public Collection<String> {

public:
    StringMsgColl ();
    // Create a StringMsgColl

    StringMsgColl (StreamableInit);
    // Create an empty StringMsgColl

    virtual u16 StreamableID () const;
    // return the object id

    static Streamable* Build ();
    // Return a new (empty) object

    virtual void Insert (String* S);
    // Insert a new string

    void Insert (const String& S);
    // Insert a new string

};



// Register the class
LINK (StringMsgColl, ID_StringMsgColl);



StringMsgColl::StringMsgColl ():
    Collection<String> (30, 0, 1)
// Create a StringMsgColl
{
}



StringMsgColl::StringMsgColl (StreamableInit):
    Collection<String> (Empty)
// Create an empty StringMsgColl
{
}



u16 StringMsgColl::StreamableID () const
// return the object id
{
    return ID_StringMsgColl;
}



Streamable* StringMsgColl::Build ()
// Return a new (empty) object
{
    return new StringMsgColl (Empty);
}



void StringMsgColl::Insert (String* S)
// Insert a new string
{
    if (Count == Limit) {
	// Collection is full, delete the first element
	AtDelete (0);
    }
    Collection<String>::Insert (S);
}



void StringMsgColl::Insert (const String& S)
// Insert a new string
{
    Insert (new String (S));
}



/*****************************************************************************/
/*			     class IstecMsgWindow			     */
/*****************************************************************************/



IstecMsgWindow::IstecMsgWindow (unsigned msWindowTitle,
				unsigned msWindowHeader1,
				unsigned msWindowHeader2,
				const String& aSizeName):
    ItemWindow (Rect (0, 1, 80, 16),
		wfFramed | wfCanMove | wfCanResize | wfSaveVisible),
    Messages (new StringMsgColl),
    ZoomSize (OBounds),
    SavedSizeName (aSizeName)
// Construct an IstecMsgWindow
{
    // Lock window output
    Lock ();

    // If there is a stored window size in the settings file, resize the
    // window to the stored rectangle.
    if (!SavedSizeName.IsEmpty ()) {
	Rect StoredBounds = StgGetRect (SavedSizeName, OBounds);
	if (StoredBounds != OBounds) {
	    Resize (StoredBounds);
	}
    }

    // Set the header
    SetHeader (::LoadAppMsg (msWindowTitle));

    // Create and insert the items for both header lines
    const String& Header1 = ::LoadAppMsg (msWindowHeader1);
    const String& Header2 = ::LoadAppMsg (msWindowHeader2);
    TextItem* H1 = new TextItem (Header1, 100, atTextNormal, NULL);
    TextItem* H2 = new TextItem (Header2, 101, atTextNormal, NULL);
    AddItem (H1);
    AddItem (H2);
    H1->SetWidth (Header1.Len ());
    H2->SetWidth (Header2.Len ());
    H1->SetPos (0, 0);
    H2->SetPos (0, 1);

    // Make an update of the window.
    DrawItems ();

    // Unlock the output
    Unlock ();
}



IstecMsgWindow::~IstecMsgWindow ()
// Destruct an IstecMsgWindow
{
    // Store the current window position and size into the settings file
    if (!SavedSizeName.IsEmpty ()) {
	StgPutRect (OBounds, SavedSizeName);
    }

    // Delete the string collection
    delete Messages;
}



void IstecMsgWindow::Store (Stream& S) const
{
    // Call the inherited Store
    ItemWindow::Store (S);

    // Store the string collection
    S.Put (Messages);

    // Store the rest of the stuff
    S << ZoomSize << SavedSizeName;
}



void IstecMsgWindow::Load (Stream& S)
{
    // Call the inherited Load
    ItemWindow::Load (S);

    // Load the string collection
    Messages = (StringMsgColl*) S.Get ();

    // Load the rest of the stuff
    S >> ZoomSize >> SavedSizeName;
}



void IstecMsgWindow::DrawInterior ()
// Draw the window contents
{
    // Lock window output
    Lock ();

    // Call the inherited function to draw item stuff
    ItemWindow::DrawInterior ();

    // Draw window specifics
    unsigned YSize = IYSize () - 2;
    unsigned Count = Messages->GetCount ();
    unsigned Index = Count > YSize? Count - YSize : 0;

    // Write out the lines
    int Y = 2;
    while (Index < Count) {
	// Watcom compiler bug: WCC needs an override here
	ItemWindow::Write (0, Y++, *Messages->At (Index++));
    }

    // Set the cursor position to the next line
    SetCursorPos (Point (0, Y));

    // Unlock the window, allow output
    Unlock ();
}



unsigned IstecMsgWindow::MinXSize () const
// Return the minimum X size of the window. Override this to limit resizing.
{
    return 10;
}



unsigned IstecMsgWindow::MinYSize () const
// Return the minumim Y size of the window. Override this to limit resizing.
{
    return 2 + 3;	// One text line at least
}



void IstecMsgWindow::Zoom ()
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



void IstecMsgWindow::Write (const String& S)
// Write a line to the window, advance the cursor
{
    // Remember the new line
    Messages->Insert (S);

    // Calculate the line count and the position
    unsigned YSize = IYSize () - 2;
    unsigned XSize = IXSize ();
    unsigned Count = Messages->GetCount ();
    if (Count > YSize) {
	// Scroll - we have to rewrite all lines
	unsigned Index = Count - YSize;
	unsigned Y = 2;
	while (Index < Count) {
	    String Line (XSize);
	    Line = *Messages->At (Index++);
	    Line.Pad (String::Right, XSize);
	    ItemWindow::Write (0, Y++, Line);
	}
    } else {
	// Just one line - pad it to length
	String Line (XSize);
	Line = S;
	Line.Pad (String::Right, XSize);

	// Watcom compiler bug: WCC needs an override here
	ItemWindow::Write (0, 2 + Count - 1, Line);
    }
}



