/*****************************************************************************/
/*                                                                           */
/*                                RESWIN.CC                                  */
/*                                                                           */
/* (C) 1993-95  Ullrich von Bassewitz                                        */
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



#include "resed.h"



/*****************************************************************************/
/*                             class ResEditApp                              */
/*****************************************************************************/



void ResEditApp::Header ()
{
    // Create a pointer alias to the lowest object in the hierarchy that
    // has a header
    Window *W = (Window *) Res;

    int Abort;
    String Header (W->GetHeader ());
    NamePrompt (" New header", Header, Abort);
    if (!Abort) {
        W->SetHeader (Header);
        ResChanged = 1;
    }
}



void ResEditApp::Footer ()
{
    // Create a pointer alias to the lowest object in the hierarchy that
    // has a footer
    Window *W = (Window *) Res;

    int Abort;
    String Footer (W->GetFooter ());
    NamePrompt (" New footer ", Footer, Abort);
    if (!Abort) {
        W->SetFooter (Footer);
        ResChanged = 1;
    }
}



void ResEditApp::Size ()
{
    // Create a pointer alias to the lowest object in the hierarchy that
    // is sizeable
    Window *W = (Window*) Res;

    // Remember the old window flags
    u16 OldFlags = W->Flags;

    // Enable moving and resizing
    W->Flags |= wfCanMove | wfCanResize;

    // Go...
    W->MoveResize ();

    // Restore the old flags
    W->Flags = OldFlags;

}


void ResEditApp::Color ()
{
    // Cast the Resource to the lowest object in the hierarchy that is able
    // to change the palette
    Window *W = (Window *) Res;

    // Decide wich palette
    switch (SimpleMenue ("Choose palette", "@Blue\n@Gray\n@Cyan\n@Red\nBl@ack\n@Error")) {

        case 0:
            // Abort
            return;

        case 1:
            W->SetPalette (paBlue);
            break;

        case 2:
            W->SetPalette (paGray);
            break;

        case 3:
            W->SetPalette (paCyan);
            break;

        case 4:
            W->SetPalette (paRed);
            break;

        case 5:
            W->SetPalette (paBlack);
            break;

        case 6:
            W->SetPalette (paError);
            break;
    }

    // Resource is changed now
    ResChanged = 1;

}



void ResEditApp::BackgroundChar ()
{
}



void ResEditApp::Number ()
{
}



void ResEditApp::IgnoreAccept (int On)
{
    ItemWindow * Win = (ItemWindow *) Res;
    if (On) {
        Win->Flags |= wfIgnoreAccept;
    } else {
        Win->Flags &= ~wfIgnoreAccept;
    }
    ResChanged = 1;
}



void ResEditApp::Modal (int On)
{
    ItemWindow * Win = (ItemWindow *) Res;
    if (On) {
        Win->Flags |= wfModal;
    } else {
        Win->Flags &= ~wfModal;
    }
    ResChanged = 1;
}



void ResEditApp::LRLink (int On)
{
    ItemWindow * Win = (ItemWindow *) Res;
    if (On) {
        Win->Flags |= wfLRLink;
    } else {
        Win->Flags &= ~wfLRLink;
    }
    ResChanged = 1;
}



void ResEditApp::Visible (int On)
{
    ItemWindow* Win = (ItemWindow*) Res;
    if (On) {
        Win->Flags |= wfSaveVisible;
    } else {
        Win->Flags &= ~wfSaveVisible;
    }
    ResChanged = 1;
}



void ResEditApp::CenterX (int On)
{
    Window *Win = (Window *) Res;
    if (On) {
        Win->SetOption (cfCenterX);
    } else {
        Win->ResetOption (cfCenterX);
    }
    ResChanged = 1;
}



void ResEditApp::CenterY (int On)
{
    Window* Win = (Window*) Res;
    if (On) {
        Win->SetOption (cfCenterY);
    } else {
        Win->ResetOption (cfCenterY);
    }
    ResChanged = 1;
}



void ResEditApp::CanMove (int On)
{
    Window* Win = (Window*) Res;
    if (On) {
        Win->Flags |= wfCanMove;
    } else {
        Win->Flags &= ~wfCanMove;
    }
    ResChanged = 1;
}



void ResEditApp::CanResize (int On)
{
    Window* Win = (Window*) Res;
    if (On) {
        Win->Flags |= wfCanResize;
    } else {
        Win->Flags &= ~wfCanResize;
    }
    ResChanged = 1;
}



void ResEditApp::Test ()
{
    if (ResID == ID_Menue || ResID == ID_TopMenueBar) {

        // Some sort of Menue
        GenericMenue *M = (GenericMenue *) Res;

        // Set it active
        M->Activate ();

        // Show a status line
        StatusLine->Push (siEsc_Abort);

        // If there are items, work with the menue, else show it
        if (M->ItemCount > 0) {
            M->RegisterItemKeys ();
            while (M->GetChoice () != 0) ;
            M->UnregisterItemKeys ();
        } else {
            int Done = 0;
            while (!Done) {
                switch (KbdGet ()) {

                    case vkAbort:
                    case kbEsc:
                        Done = 1;

                }
            }
        }

        // Deactivate the window, pop the statusline
        M->Deactivate ();
        StatusLine->Pop ();

        // Resource has changed
        ResChanged = 1;

    } else {

        // Wrong type of resource
        ErrorMsg ("Cannot test this type of resource");

    }

}


