/*****************************************************************************/
/*									     */
/*				   RESUTIL.CC				     */
/*									     */
/* (C) 1993-95	Ullrich von Bassewitz					     */
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



#include "resed.h"



/*****************************************************************************/
/*			       class ResEditApp				     */
/*****************************************************************************/



void ResEditApp::DeleteRes ()
{
    delete Res;
    Res		= 0;
    ResID	= 0;
    ResChanged	= 0;

    // Change menue entries
    MainMenue->GrayItem (miWindow);
    MainMenue->GrayItem (miItems);
    MainMenue->GrayItem (miEditActions);
    MainMenue->GrayItem (miPrint);
    MainMenue->GrayItem (miWrite);

}



void ResEditApp::AssignRes (Streamable *NewRes)
{
    GenericMenue* M;

    // Delete the old resource, assign the new one
    DeleteRes ();
    Res = NewRes;
    ResID = NewRes->StreamableID ();

    // Change menue entries according to the resource type
    switch (ResID) {

	case ID_TopMenueBar:
	    // Move the TopMenueBar into the visible area
	    ((TopMenueBar*)Res)->MoveAbs (Point (0, 1));
	case ID_Menue:
	    M = (GenericMenue*) Res;
	    MainMenue->SetToggleValue (miCanMove, M->CanMove ());
	    MainMenue->SetToggleValue (miCanResize, M->CanResize ());
	    MainMenue->SetToggleValue (miModal, M->IsModal ());
	    MainMenue->SetToggleValue (miLRLink, M->HasLRLink ());
	    MainMenue->SetToggleValue (miIgnoreAccept, M->IgnoreAccept ());
	    MainMenue->SetToggleValue (miVisible, M->SaveVisible ());
	    MainMenue->ActivateItem (miWindow);
	    MainMenue->ActivateItem (miItems);
	    MainMenue->ActivateItem (miPrint);
	    if (ResFile) {
		MainMenue->ActivateItem (miWrite);
	    }
	    M->Show ();
	    break;

	case ID_MsgCollection:
	    MainMenue->ActivateItem (miEditActions);
	    MainMenue->ActivateItem (miPrint);
	    if (ResFile) {
		MainMenue->ActivateItem (miWrite);
	    }
	    break;

	default:
	    DeleteRes ();
	    ErrorMsg ("Unknown resource type, deleting");
	    break;

    }

}




void ResEditApp::NamePrompt (const String &Name, String &Val, int &Abort)
{
    // Calculate the size of the window
    const Rect& Screen = Background->OuterBounds ();
    Rect Bounds;
    Bounds.A.X = 2;
    Bounds.A.Y = Screen.YSize () / 2;
    Bounds.B.X = Screen.XSize () - 2;
    Bounds.B.Y = Bounds.A.Y + 3;

    // Create a window with an edit item
    int Length = Name.Len ();
    EditLine* E = new EditLine (Name, 1, 255, Bounds.XSize () - Length - 6, NULL);
    ItemWindow* Win = new ItemWindow (Bounds, wfFramed, paGray, 0, E);

    // Set the default
    E->SetValue (Val);

    // Edit the string
    E->Edit (Abort);
    if (!Abort) {
	Val = E->GetValue ();
    }

    // Delete the window including the edit item
    delete Win;
}



void ResEditApp::FloatPrompt (const String& Text, double& Val,
			      double Min, double Max, int& Abort)
{
    // Create a new edit item
    FloatEdit *F = new FloatEdit (Text, 1, 10, 3, NULL);

    // Calculate the size of the window
    Rect ScreenSize (Background->OuterBounds ());
    Rect Bounds (0, 0, F->MinWidth () + 2, 3);
    Bounds.Center (ScreenSize, cfCenterAll);

    // Create a window and insert the edit item
    F->SetMinMax (Min, Max);
    ItemWindow *Win = new ItemWindow (Bounds, wfFramed, paGray, 0, F);

    // Set the default
    F->SetValue (Val);

    // Edit the string
    F->Edit (Abort);
    if (!Abort) {
	Val = F->GetValue ();
    }

    // Delete the window including the edit item
    delete Win;
}



void ResEditApp::LongPrompt (const String & Text, i32 &Val, i32 Min, i32 Max, int &Abort)
{
    // Create a new edit item
    LongEdit *L = new LongEdit (Text, 1, 8, NULL);

    // Calculate the size of the window
    Rect ScreenSize (Background->OuterBounds ());
    Rect Bounds (0, 0, L->MinWidth () + 2, 3);
    Bounds.Center (ScreenSize, cfCenterAll);

    // Create a window and insert the edit item
    L->SetMinMax (Min, Max);
    ItemWindow *Win = new ItemWindow (Bounds, wfFramed, paGray, 0, L);

    // Set the default
    L->SetValue (Val);

    // Edit the string
    L->Edit (Abort);
    if (!Abort) {
	Val = L->GetValue ();
    }

    // Delete the window including the edit item
    delete Win;
}



void ResEditApp::IDPrompt (i16& ID, int& Abort)
{
    // Use a 32 bit int for temp val
    i32 ID32 = ID;

    // Get the id
    LongPrompt (" ID", ID32, 1, 32000, Abort);
    ID = (i16) ID32;
}



void ResEditApp::EditIDPrompt (i16 &ID, int &Abort)
{
    // Use a 32 bit int for temp val
    i32 ID32 = ID;

    // Get the id
    LongPrompt (" Edit ID", ID32, 0, 32000, Abort);
    ID = (i16) ID32;
}



void ResEditApp::MinMaxPrompt (i32 &Min, i32 &Max, i32 MinVal, i32 MaxVal, int &Abort)
{
    if (Min < MinVal) {
	Min = MinVal;
    }
    LongPrompt (" Minimum value", Min, MinVal, MaxVal, Abort);
    if (Abort) {
	return;
    }
    if (Max < Min) {
	Max = Min;
    }
    if (Max > MaxVal) {
	Max = MaxVal;
    }
    LongPrompt (" Maximum value", Max, Min, MaxVal, Abort);
    if (Abort) {
	return;
    }
}



void ResEditApp::FMinMaxPrompt (double& Min, double& Max, double MinVal,
				double MaxVal, int &Abort)
{
    if (Min < MinVal) {
	Min = MinVal;
    }
    FloatPrompt (" Minimum value", Min, MinVal, MaxVal, Abort);
    if (Abort) {
	return;
    }
    if (Max < Min) {
	Max = Min;
    }
    if (Max > MaxVal) {
	Max = MaxVal;
    }
    FloatPrompt (" Maximum value", Max, Min, MaxVal, Abort);
    if (Abort) {
	return;
    }
}



void ResEditApp::TextIDPrompt (String &Text, i16 &ID, int &Abort)
{
    // Choose the next possible ID
    ID = NextID ();

    // Edit the ID
    IDPrompt (ID, Abort);
    if (Abort) {
	return;
    }

    // Edit the text
    NamePrompt (" Text", Text, Abort);
}



int ResEditApp::SaveResource ()
//
{
    if (Res && ResChanged) {

	switch (AskSaveChanges ()) {

	    case 0:
		//Abort
		return 0;

	    case 1:
		// No
		return 1;

	    case 2:
		// Yes
		if (ResFile == NULL) {
		    ErrorMsg ("^!!! No resource file !!!|^"
			      "Choose ~No~ the next time,^"
			      "or open a resource file first!|");
		    return 0;	// Abort;
		} else {
		    Write ();
		    // Return true only if the save has been successful
		    return (ResChanged == 0);
		}

	}

    }

    // No change, always successful
    return 1;
}



