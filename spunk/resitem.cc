/*****************************************************************************/
/*                                                                           */
/*                                 RESITEM.CC                                */
/*                                                                           */
/* (C) 1993-96  Ullrich von Bassewitz                                        */
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



#include "itemlbl.h"
#include "textitem.h"
#include "stdmsg.h"
#include "resed.h"



/*****************************************************************************/
/*                             class ResEditApp                              */
/*****************************************************************************/



WindowItem* ResEditApp::ChooseItem ()
{
    u16 Flags [100];

    // Cast the resource to something that is able to hold items and choose
    // from those items
    GenericMenue* M = (GenericMenue*) Res;

    // Check if there are any items to choose
    if (M->ItemCount == 0) {
        ErrorMsg ("No items");
        return NULL;
    }

    // Check if we have enough room to store the flags
    if (M->ItemCount > (sizeof (Flags) / sizeof (u16))) {
        ErrorMsg ("Sorry, too many items (change ResItem source)");
        return NULL;
    }

    // Save the item attributes into the save area, activate all items,
    // set the ifNoSub flag
    WindowItem *I = M->FirstItem;
    int F = 0;
    do {
        Flags [F] = I->Flags & ~ifSelected;
        I->Activate ();
        I->Flags |= ifNoSub;                    // No harm done on other items
        F++;
        I = I->INode.Next () -> Contents ();
    } while (I != M->FirstItem);

    // Now choose an item
    StatusLine->Push (" Choose an item");            // #####
    int Result = M->GetChoice ();
    StatusLine->Pop ();                              // #####

    // Restore the flags and redraw the items
    I = M->FirstItem;
    F = 0;
    do {
        I->Flags = Flags [F];
        I->Draw ();
        F++;
        I = I->INode.Next ()->Contents ();
    } while (I != M->FirstItem);

    // Return the result
    return Result ? M->ForcedItemWithID (Result) : (WindowItem*) NULL;
}



i16 ResEditApp::NextID (i16 StartID)
// Choose a new ID
{
    // Cast the resource pointer
    ItemWindow *Win = (ItemWindow *) Res;

    // Search for a free ID
    while (Win->ItemWithID (StartID) != NULL) {
        StartID++;
    }
    return StartID;
}



i16 ResEditApp::SearchItemYPos ()
// Search the first free Y position in the Menue
{
    // Cast to a ItemWindow
    ItemWindow * Win = (ItemWindow *) Res;

    // If the window is empty, stop here
    if (Win->ItemCount == 0) {
        return 0;
    }

    // Search for a free Y position
    int Y = 0;
    WindowItem *Item;
    int Found;
    while (Y <= Win->MaxY ()) {

        // Search for this Y pos
        Found = 0;
        Item = Win->FirstItem;
        do {
            if (Item->ItemY == Y) {
                // Already occupied
                Found = 1;
                break;
            }
            Item = Item->INode.Next () -> Contents ();
        } while (Item != Win->FirstItem);

        if (!Found) {
            // This Y pos is free
            return (i16) Y;
        }

        Y++;
    }

    // Not found
    return 0;
}



i16 ResEditApp::SearchItemXPos ()
// Search the first free X position in the Menue
{
    // Cast to a ItemWindow
    ItemWindow * Win = (ItemWindow *) Res;

    // If the window is empty, stop here
    if (Win->ItemCount == 0) {
        return 1;
    }

    // Search for a free X position
    i16 X = 1;
    WindowItem *Item = Win->FirstItem;
    do {
        i16 MaxX = Item->ItemX + Item->GetWidth ();
        if (MaxX > X) {
            X = MaxX;
        }
        Item = Item->INode.Next () -> Contents ();
    } while (Item != Win->FirstItem);

    if (X >= Win->MaxX ()) {
        X = 0;
    }

    return X;
}



void ResEditApp::AddItem (WindowItem *Item)
//
{
    // Cast the resource to something that is able to hold a WindowItem
    ItemWindow *Win = (ItemWindow *) Res;

    // Search for a free Y position
    i16 YPos = SearchItemYPos ();

    // Add the item
    Win->AddItem (Item);
    Item->SetPos (0, YPos);
    i16 Width = Win->InnerBounds ().XSize ();
    i16 MinWidth = Item->MinWidth ();
    Item->SetWidth (Width >= MinWidth ? Width : MinWidth);
    Win->ValidateSelectedItem ();
    Win->DrawInterior ();
    ResChanged = 1;
}



void ResEditApp::AddWindowItem ()
{
    int Abort;

    // Ask for item id and item text
    i16 ID;
    String ItemText;
    TextIDPrompt (ItemText, ID, Abort);
    if (Abort) {
        return;
    }

    // Create and add the item
    AddItem (new WindowItem (ItemText, ID, NULL));
}



void ResEditApp::AddTextItem ()
{
    int Abort;

    // Ask for item id and item text
    i16 ID;
    String ItemText;
    TextIDPrompt (ItemText, ID, Abort);
    if (Abort) {
        return;
    }

    // Ask for the attribute of the text
    unsigned Attr = SimpleMenue (" Attribute ",
                                 "atFrameInactive\n"
                                 "atFrameActive\n"
                                 "atFrameResizing\n"
                                 "atTextNormal\n"
                                 "atTextInvers\n"
                                 "atTextSelected\n"
                                 "atTextHigh\n"
                                 "atTextHighInvers\n"
                                 "atTextGrayed\n"
                                 "atTextGrayedInvers\n"
                                 "atEditNormal\n"
                                 "atEditHigh\n"
                                 "atEditBar");
    if (Attr == 0) {
        // User abort
        return;
    }
    Attr--;

    // Create and add the item
    AddItem (new TextItem (ItemText, ID, Attr, NULL));
}



void ResEditApp::AddItemLabel ()
{
    int Abort;

    // Ask for item id and item text
    i16 ID;
    String ItemText;
    TextIDPrompt (ItemText, ID, Abort);
    if (Abort) {
        return;
    }

    // Ask for the id of the controlled item
    i32 CtrlID = 0;
    LongPrompt (" Ctrl-ID", CtrlID, 1, 32000, Abort);
    if (Abort) {
        return;
    }

    // Create and add the item
    AddItem (new ItemLabel (ItemText, ID, CtrlID, NULL));
}



void ResEditApp::AddMenueItem ()
{
    int Abort;

    // Ask for item id and item text
    i16 ID;
    String ItemText;
    TextIDPrompt (ItemText, ID, Abort);
    if (Abort) {
        return;
    }

    // Create and add the item
    AddItem (new MenueItem (ItemText, ID, NULL));
}



void ResEditApp::AddSubMenueItem ()
{
    int Abort;

    // Ask for item id and item text
    i16 ID;
    String ItemText;
    TextIDPrompt (ItemText, ID, Abort);
    if (Abort) {
        return;
    }

    // Try to load the submenue for the item
    String MenueName = ChooseRes ();
    if (MenueName.IsEmpty ()) {
        return;
    }
    Streamable* M = LoadRes (MenueName);
    if (M == 0) {
        return;
    }

    // Check if we really have a menue
    if (M->StreamableID () != ID_Menue) {
        ErrorMsg ("Must have a menue object for assignment");
        delete M;
        return;
    }

    // Create the item
    SubMenueItem* S = new SubMenueItem (ItemText, ID, 0, 0);

    // Add the item
    AddItem (S);

    // Add the submenue
    S->SetSubMenue ((Menue*) M);
}



void ResEditApp::AddMenueBarItem ()
{
    int Abort;

    // Ask for item id and item text
    i16 ID;
    String ItemText;
    TextIDPrompt (ItemText, ID, Abort);
    if (Abort) {
        return;
    }

    // Try to load the submenue for the item
    String MenueName = ChooseRes ();
    if (MenueName.IsEmpty ()) {
        return;
    }
    Streamable* M = LoadRes (MenueName);
    if (M == 0) {
        return;
    }

    // Check if we really have a menue
    if (M->StreamableID () != ID_Menue) {
        ErrorMsg ("Must have a menue object for assignment");
        delete M;
        return;
    }

    // Create the item
    MenueBarItem* S = new MenueBarItem (ItemText, ID, 0, 0);

    // Add the item. This must not be done with AddItem since AddItem will
    // align items verticaly, which is wrong in this case.
    ItemWindow *Win = (ItemWindow *) Res;

    // Search for a free X position
    i16 XPos = SearchItemXPos ();

    // Add the item
    Win->AddItem (S);
    S->SetPos (XPos, 0);
    S->SetWidth (S->MinWidth ());
    Win->ValidateSelectedItem ();
    Win->DrawInterior ();
    ResChanged = 1;

    // Add the submenue
    S->SetSubMenue ((Menue*) M);
}



void ResEditApp::AddFloatItem ()
{
    int Abort;
    double Min, Max;

    // Ask for item id and item text
    i16 ID;
    String ItemText;
    TextIDPrompt (ItemText, ID, Abort);
    if (Abort) {
        return;
    }

    // Ask for leading digit count
    i32 LD = 4;
    LongPrompt (" Leading digits", LD, 1, 10, Abort);
    if (Abort) {
        return;
    }

    // Ask for trailing digit count
    i32 TD = 4;
    LongPrompt (" Trailing digits", TD, 0, 10, Abort);
    if (Abort) {
        return;
    }

    // Ask for Edit ID
    i16 EditID = 0;
    EditIDPrompt (EditID, Abort);
    if (Abort) {
        return;
    }

    // Initialize Min/Max
    Min = Max = 0;

    if (EditID != 0) {

        // We have an edit item, choose Min/Max
        FMinMaxPrompt (Min, Max, -1000000000, 1000000000, Abort);
        if (Abort) {
            return;
        }

    }

    // Create and add the item
    AddItem (new FloatItem (ItemText, ID, LD, TD, EditID, Min, Max, NULL));
}



void ResEditApp::AddExpFloatItem ()
{
    ErrorMsg ("Not implemented");
}



void ResEditApp::AddLongItem ()
{
    int Abort;
    i32 Min, Max;

    // Ask for item id and item text
    i16 ID;
    String ItemText;
    TextIDPrompt (ItemText, ID, Abort);
    if (Abort) {
        return;
    }

    // Ask for digit count
    i32 Digits = 4;
    LongPrompt (" Digit count", Digits, 1, 10, Abort);
    if (Abort) {
        return;
    }

    // Ask for Edit ID
    i16 EditID = 0;
    EditIDPrompt (EditID, Abort);
    if (Abort) {
        return;
    }

    // Initialize Min/Max
    Min = Max = 0;

    if (EditID != 0) {

        // We have an edit item, choose Min/Max
        MinMaxPrompt (Min, Max, (i32) 0x80000000, 0x7FFFFFFF, Abort);
        if (Abort) {
            return;
        }

    }

    // Create and add the item
    AddItem (new LongItem (ItemText, ID, Digits, EditID, Min, Max, NULL));
}



void ResEditApp::AddHexItem ()
{
    int Abort;
    i32 Min, Max;

    // Ask for item id and item text
    i16 ID;
    String ItemText;
    TextIDPrompt (ItemText, ID, Abort);
    if (Abort) {
        return;
    }

    // Ask for digit count
    i32 Digits = 4;
    LongPrompt (" Digit count", Digits, 2, 8, Abort);
    if (Abort) {
        return;
    }

    // Ask for Edit ID
    i16 EditID = 0;
    EditIDPrompt (EditID, Abort);
    if (Abort) {
        return;
    }

    // Initialize Min/Max
    Min = Max = 0;

    if (EditID != 0) {

        MinMaxPrompt (Min, Max, (i32) 0x80000000, 0x7FFFFFFF, Abort);
        if (Abort) {
            return;
        }

    }

    // Create and add the item
    AddItem (new HexItem (ItemText, ID, Digits, EditID, Min, Max, NULL));
}



void ResEditApp::AddStringItem ()
{
    int Abort;

    // Ask for item id and item text
    i16 ID;
    String ItemText;
    TextIDPrompt (ItemText, ID, Abort);
    if (Abort) {
        return;
    }

    // Ask for Edit ID
    i16 EditID = 0;
    EditIDPrompt (EditID, Abort);
    if (Abort) {
        return;
    }

    // Create and add the item
    AddItem (new StringItem (ItemText, ID, EditID, NULL));
}



void ResEditApp::AddRStringItem ()
{
    int Abort;

    // Ask for item id and item text
    i16 ID;
    String ItemText;
    TextIDPrompt (ItemText, ID, Abort);
    if (Abort) {
        return;
    }

    // Ask for Edit ID
    i16 EditID = 0;
    EditIDPrompt (EditID, Abort);
    if (Abort) {
        return;
    }

    i32 InputLength = 0;
    LongPrompt (" Input length:", InputLength, 1, 255, Abort);
    if (Abort) {
        return;
    }

    // Create and add the item
    AddItem (new RStringItem (ItemText, ID, EditID, InputLength, NULL));
}



void ResEditApp::AddToggleItem ()
{
    int Abort;

    // Ask for item id and item text
    i16 ID;
    String ItemText;
    TextIDPrompt (ItemText, ID, Abort);
    if (Abort) {
        return;
    }

    // Ask for the toggle count
    i32 ToggleCount = 2;
    LongPrompt (" Toggle count", ToggleCount, 2, 25, Abort);
    if (Abort) {
        return;
    }

    // Ask for the toggle string
    String ToggleText;
    NamePrompt (" Toggle values", ToggleText, Abort);
    if (Abort || ToggleText.IsEmpty ()) {
        return;
    }

    // Validate the toggle text
    if (ToggleText.Len () % ToggleCount != 0) {
        ErrorMsg ("Invalid toggle count / toggle values");
        return;
    }

    // Create and add the item
    AddItem (new ToggleItem (ItemText, ID, ToggleText, ToggleCount, NULL));
}



void ResEditApp::AddOffOnItem ()
{
    int Abort;

    // Ask for item id and item text
    i16 ID;
    String ItemText;
    TextIDPrompt (ItemText, ID, Abort);
    if (Abort) {
        return;
    }

    // Create and add the item
    AddItem (new OffOnItem (ItemText, ID, NULL));
}



void ResEditApp::AddNoYesItem ()
{
    int Abort;

    // Ask for item id and item text
    i16 ID;
    String ItemText;
    TextIDPrompt (ItemText, ID, Abort);
    if (Abort) {
        return;
    }

    // Create and add the item
    AddItem (new NoYesItem (ItemText, ID, NULL));
}



void ResEditApp::AddDateItem ()
{
    int Abort;

    // Ask for item id and item text
    i16 ID;
    String ItemText;
    TextIDPrompt (ItemText, ID, Abort);
    if (Abort) {
        return;
    }

    // Ask for Edit ID
    i16 EditID = 0;
    EditIDPrompt (EditID, Abort);
    if (Abort) {
        return;
    }

    // Create and add the item
    AddItem (new DateItem (ItemText, ID, EditID, NULL));
}



void ResEditApp::AddTimeItem ()
{

    int Abort;

    // Ask for item id and item text
    i16 ID;
    String ItemText;
    TextIDPrompt (ItemText, ID, Abort);
    if (Abort) {
        return;
    }

    // Ask for Edit ID
    i16 EditID = 0;
    EditIDPrompt (EditID, Abort);
    if (Abort) {
        return;
    }

    // Create and add the item
    AddItem (new TimeItem (ItemText, ID, EditID, NULL));
}



void ResEditApp::AddEditLine ()
{
    int Abort;

    // Ask for item id and item text
    i16 ID;
    String ItemText;
    TextIDPrompt (ItemText, ID, Abort);
    if (Abort) {
        return;
    }

    // Ask for the field length
    i32 FieldLen = 4;
    LongPrompt (" Field length", FieldLen, 2, 255, Abort);
    if (Abort) {
        return;
    }

    // Ask for the maximum input length
    i32 MaxLen = FieldLen - 1;
    LongPrompt (" Input length", MaxLen, FieldLen - 1, 254, Abort);
    if (Abort) {
        return;
    }

    // Create and add the item
    AddItem (new EditLine (ItemText, ID, MaxLen, FieldLen, NULL));
}



void ResEditApp::AddFloatEdit ()
{
    int Abort;
    double Min, Max;

    // Ask for item id and item text
    i16 ID;
    String ItemText;
    TextIDPrompt (ItemText, ID, Abort);
    if (Abort) {
        return;
    }

    // Ask for leading digit count
    i32 LD = 4;
    LongPrompt (" Leading digits", LD, 1, 10, Abort);
    if (Abort) {
        return;
    }

    // Ask for trailing digit count
    i32 TD = 4;
    LongPrompt (" Trailing digits", TD, 0, 10, Abort);
    if (Abort) {
        return;
    }

    // Initialize Min/Max
    Min = Max = 0;

    // choose Min/Max
    FMinMaxPrompt (Min, Max, -1000000000, 1000000000, Abort);
    if (Abort) {
        return;
    }

    // Create and add the item
    AddItem (new FloatEdit (ItemText, ID, LD, TD, Min, Max, NULL));
}



void ResEditApp::AddExpFloatEdit ()
{
    ErrorMsg ("Not implemented");
}



void ResEditApp::AddLongEdit ()
{
    int Abort;

    // Ask for item id and item text
    i16 ID;
    String ItemText;
    TextIDPrompt (ItemText, ID, Abort);
    if (Abort) {
        return;
    }

    // Ask for digit count
    i32 Digits = 4;
    LongPrompt (" Digit count", Digits, 1, 10, Abort);
    if (Abort) {
        return;
    }

    // choose Min/Max
    i32 Min = 0;
    i32 Max = 0;
    MinMaxPrompt (Min, Max, (i32) 0x80000000, 0x7FFFFFFF, Abort);
    if (Abort) {
        return;
    }

    // Create and add the item
    AddItem (new LongEdit (ItemText, ID, Digits, Min, Max, NULL));
}



void ResEditApp::AddHexEdit ()
{
    int Abort;

    // Ask for item id and item text
    i16 ID;
    String ItemText;
    TextIDPrompt (ItemText, ID, Abort);
    if (Abort) {
        return;
    }

    // Ask for digit count
    i32 Digits = 4;
    LongPrompt (" Digit count", Digits, 2, 8, Abort);
    if (Abort) {
        return;
    }

    // choose Min/Max
    i32 Min = 0;
    i32 Max = 0;
    MinMaxPrompt (Min, Max, (i32) 0x80000000, 0x7FFFFFFF, Abort);
    if (Abort) {
        return;
    }

    // Create and add the item
    AddItem (new HexEdit (ItemText, ID, Digits, Min, Max, NULL));
}



void ResEditApp::AddPasswordEdit ()
{
    int Abort;

    // Ask for item id and item text
    i16 ID;
    String ItemText;
    TextIDPrompt (ItemText, ID, Abort);
    if (Abort) {
        return;
    }

    // Ask for the field length
    i32 FieldLen = 13;
    LongPrompt (" Field length", FieldLen, 2, 255, Abort);
    if (Abort) {
        return;
    }

    // Ask for the maximum input length
    i32 MaxLen = FieldLen - 1;
    LongPrompt (" Input length", MaxLen, FieldLen - 1, 254, Abort);
    if (Abort) {
        return;
    }

    // Create and add the item
    AddItem (new PasswordEdit (ItemText, ID, MaxLen, FieldLen, NULL));
}



void ResEditApp::AddFileEdit ()
{
    int Abort;

    // Ask for item id and item text
    i16 ID;
    String ItemText;
    TextIDPrompt (ItemText, ID, Abort);
    if (Abort) {
        return;
    }

    // Ask for the field length
    i32 FieldLen = 13;
    LongPrompt (" Field length", FieldLen, 2, 255, Abort);
    if (Abort) {
        return;
    }

    // Ask for the maximum input length
    i32 MaxLen = FieldLen - 1;
    LongPrompt (" Input length", MaxLen, FieldLen - 1, 254, Abort);
    if (Abort) {
        return;
    }

    // Create and add the item
    AddItem (new FileEdit (ItemText, ID, MaxLen, FieldLen, 0, NULL));
}



void ResEditApp::AddDateEdit ()
{
    int Abort;

    // Ask for item id and item text
    i16 ID;
    String ItemText;
    TextIDPrompt (ItemText, ID, Abort);
    if (Abort) {
        return;
    }

    // Create and add the item
    AddItem (new DateEdit (ItemText, ID, NULL));
}



void ResEditApp::AddTimeEdit ()
{
    int Abort;

    // Ask for item id and item text
    i16 ID;
    String ItemText;
    TextIDPrompt (ItemText, ID, Abort);
    if (Abort) {
        return;
    }

    // Create and add the item
    AddItem (new TimeEdit (ItemText, ID, NULL));
}



void ResEditApp::AddTextEdit ()
{
    int Abort;

    // Ask for item id and item text
    i16 ID;
    String ItemText;
    TextIDPrompt (ItemText, ID, Abort);
    if (Abort) {
        return;
    }

    // Ask for the field length
    i32 FieldLen = 4;
    LongPrompt (" Field length", FieldLen, 2, 255, Abort);
    if (Abort) {
        return;
    }

    // Ask for the maximum input length
    i32 MaxLen = FieldLen - 1;
    LongPrompt (" Input length", MaxLen, FieldLen - 1, 254, Abort);
    if (Abort) {
        return;
    }

    // Create and add the item
    AddItem (new TextEdit (ItemText, ID, MaxLen, FieldLen, NULL));
}



void ResEditApp::AddMenueLine ()
{
    i16 ID = NextID (30000);
    int Abort;

    // Ask for item id
    IDPrompt (ID, Abort);
    if (Abort) {
        return;
    }

    AddItem (new MenueLine (ID, NULL));
}



void ResEditApp::Delete ()
{
    // Choose the item to delete
    WindowItem *Item = ChooseItem ();
    if (Item == NULL) {
        // abort
        return;
    }

    //
    if (AskAreYouShure () != 2) {
        // Aborted!
        return;
    }

    ItemWindow *Win = (ItemWindow *) Res;
    Win->DeleteItem (Item);
    Win->DrawInterior ();
    ResChanged = 1;
}



void ResEditApp::Copy ()
{
    ErrorMsg ("Not implemented");
}



void ResEditApp::MoveResizeInside (Window* W, GenericMenue* M, int Resize, int& Abort)
// Move/Resize a window inside a menue
{

    // Allow moving and sizing the window
    // New status line according to the resize flag
    if (Resize) {
        // Resizing allowed
        StatusLine->Push (" ~Esc~ Abort  ~Enter~ Accept  "
                          "~Cursor keys~ Move  "
                          "~Ctrl-Cursor keys~ Size");
    } else {
        StatusLine->Push (" ~Esc~ Abort  ~Enter~ Accept  "
                          "~Cursor keys~ Move");
    }

    // Now let's size/move...
    Rect Bounds;
    int Done = 0;
    Abort = 0;
    while (!Done) {

        switch (KbdGet ()) {

            case vkAbort:
            case kbEsc:
                // Abort the operation
                Done = 1;
                Abort = 1;
                break;

            case vkAccept:
            case kbEnter:
                Done = 1;
                break;

            case vkUp:
                if (W->OuterBounds().A.Y > M->InnerBounds().A.Y) {
                    W->MoveRel (Point (0, -1));
                }
                break;

            case vkDown:
                if (W->OuterBounds().B.Y < M->InnerBounds().B.Y) {
                    W->MoveRel (Point (0, 1));
                }
                break;

            case vkLeft:
                if (W->OuterBounds().A.X > M->InnerBounds().A.X) {
                    W->MoveRel (Point (-1, 0));
                }
                break;

            case vkRight:
                if (W->OuterBounds().B.X < M->InnerBounds().B.X) {
                    W->MoveRel (Point (1, 0));
                }
                break;

            case vkCtrlUp:
                if (Resize) {
                    Bounds = W->OuterBounds ();
                    if (Bounds.YSize () > 1) {
                        Bounds.B.Y--;
                        W->Resize (Bounds);
                    }
                }
                break;

            case vkCtrlLeft:
                if (Resize) {
                    Bounds = W->OuterBounds ();
                    if (Bounds.XSize () > 2) {
                        Bounds.B.X--;
                        W->Resize (Bounds);
                    }
                }
                break;

            case vkCtrlDown:
                if (Resize) {
                    Bounds = W->OuterBounds ();
                    if (Bounds.B.Y < M->InnerBounds().B.Y) {
                        Bounds.B.Y++;
                        W->Resize (Bounds);
                    }
                }
                break;

            case vkCtrlRight:
                if (Resize) {
                    Bounds = W->OuterBounds ();
                    if (Bounds.B.X < M->InnerBounds().B.X) {
                        Bounds.B.X++;
                        W->Resize (Bounds);
                    }
                }
                break;

        }

    }

    // Restore old statusline
    StatusLine->Pop ();

}



void ResEditApp::MoveArea ()
{
    // Cast the resource to a menue
    GenericMenue* M = (GenericMenue*) Res;

    // Create a window in the upper left corner of the resource window
    Rect WBounds (M->InnerBounds().A, Point (1, 1));
    Window* W = new Window (WBounds, 0, paBlack);

    // Put the window on top of the menue and activate it
    W->PutOnTop ();
    W->Activate ();

    // Now let's size/move...
    int Abort;
    MoveResizeInside (W, M, 1, Abort);
    if (Abort) {
        delete W;
        return;
    }

    // Remember the area to move
    Rect Bounds (W->OuterBounds ());

    // Move the window to the target position
    MoveResizeInside (W, M, 0, Abort);
    if (Abort) {
        delete W;
        return;
    }

    // Remember the move vector
    Point Vec (Bounds.A.X - W->OuterBounds().A.X,
               Bounds.A.Y - W->OuterBounds().A.Y);

    // delete the upper window
    delete W;

    // Now move all items inside the source area by the move vector
    WindowItem* I = M->FirstItem;
    if (I && Vec != Point (0, 0)) {
        do {
            Point P (I->Pos ());
            M->Absolute (P);
            if (Bounds.Contains (P)) {
                // The item is inside the source area
                P -= Vec;
                M->Relative (P);
                I->SetPos (P);
                ResChanged = 1;
            }
            I = I->INode.Next () -> Contents ();
        } while (I != M->FirstItem);

        // Redraw the items inside the window
        M->DrawInterior ();
    }

}



void ResEditApp::Edit ()
{
    // Choose the item to move
    WindowItem* Item = ChooseItem ();
    if (Item == NULL) {
        return;
    }

    // Get a pointer to the owner window
    ItemWindow* Win = (ItemWindow*) Res;

    // Create new status line
    StatusLine->Push (siEnd | siMoveKeys);

    Win->ValidateSelectedItem ();
    Win->Activate ();

    int Done = 0;
    while (!Done) {

        switch (KbdGet ()) {

            case vkAccept:
            case vkAbort:
                Done = 1;
                ResChanged = 1;
                break;

            case vkUp:
                if (Item->YPos () > 0) {
                    Item->Clear ();
                    Item->SetPos (Item->XPos (), Item->YPos () - 1);
                    Win->DrawInterior ();
                }
                break;

            case vkDown:
                if (Item->YPos () < Win->MaxY ()) {
                    Item->Clear ();
                    Item->SetPos (Item->XPos (), Item->YPos () + 1);
                    Win->DrawInterior ();
                }
                break;

            case vkLeft:
                if (Item->XPos () > 0) {
                    Item->Clear ();
                    Item->SetPos (Item->XPos () - 1, Item->YPos ());
                    Win->DrawInterior ();
                }
                break;

            case vkRight:
                if (Item->XPos () < Win->MaxX ()) {
                    Item->Clear ();
                    Item->SetPos (Item->XPos () + 1, Item->YPos ());
                    Win->DrawInterior ();
                }
                break;

            case vkCtrlLeft:
                Item->Clear ();
                Item->SetWidth (Item->GetWidth () - 1);
                Item->Draw ();
                break;

            case vkCtrlRight:
                Item->Clear ();
                Item->SetWidth (Item->GetWidth () + 1);
                Item->Draw ();
                break;

            case kbEnter:
                ItemMenue (Item);
                break;

        }

    }

    //
    Win->Deactivate ();

    // Restore the old status line
    PopStatusLine ();
}



String ResEditApp::GetItemText (WindowItem* Item)
// Extract and return the text of a window item.
{
    // Get the text
    String Text = Item->ItemText;

    // Insert a '@' if a hotkey exists
    if (Item->HotPos != -1) {
        Text.Ins (Item->HotPos, '@');
    }

    // Return the result
    return Text;
}



void ResEditApp::SetItemText (WindowItem* Item, String Text)
// Set the new item text, handle @ as the hotkey position.
{
    // Clear the old text before setting the new one
    Item->ClearItemText ();

    // Check for a hotkey
    int Pos = Text.Pos ('@');
    Item->HotPos = Pos;
    if (Pos != -1) {
        Text.Del (Pos, 1);
        Item->HotKey = NLSUpCase (Text [Pos]);
    } else {
        Item->HotKey = kbNoKey;
    }
    Item->ItemText = Text;
    Item->SetWidth (Item->GetWidth ());
    Item->DrawItemText ();

    // Resource has changed
    ResChanged = 1;
}



unsigned ResEditApp::GetItemState (WindowItem* Item)
// Extract and return the item state 0..2
{
    if (Item->IsActive ()) {
        return 0;
    } else if (Item->IsGrayed ()) {
        return 2;
    } else {
        // Inactive
        return 1;
    }
}



void ResEditApp::SetItemState (WindowItem* Item, unsigned State)
// Set the state according to the state given (0..2)
{
    switch (State) {

        case 0:
            Item->Activate ();
            break;

        case 1:
            Item->Deactivate ();
            break;

        case 2:
            Item->Gray ();
            break;

        default:
            FAIL ("ResEditApp::SetItemState: Invalid item state");

    }
}



String ResEditApp::GetCharSetString (WindowItem* Item)
// Get a character set from a window item as a string
{
    CharSet CS = GetCharSet (Item);

    // Convert to string
    String S (256);
    for (unsigned I = 1; I < 256; I++) {
        if (CS [(char)I] != 0) {
            S += (char)I;
        }
    }
    S.ShowControls ();

    // Return the string
    return S;
}



void ResEditApp::SetCharSetString (WindowItem* I, String S)
// Set a character set of a window item from a string
{
    // Make a character set from a string
    S.HideControls ();
    CharSet CS = S;

    // Set the new characters
    switch (I->StreamableID ()) {

        case ID_RStringItem:
            ((RStringItem*) I)->SetAllowedChars (CS);
            break;

        case ID_TextEdit:
            ((TextEdit*) I)->SetAllowedChars (CS);
            break;

        default:
            // Error
            FAIL ("GetCharset: Invalid item type");
    }
}



const CharSet& ResEditApp::GetCharSet (WindowItem* I)
// Get a character set from an item
{
    if (I->StreamableID () == ID_RStringItem) {
        return ((RStringItem*) I)->GetAllowedChars ();
    } else if (I->StreamableID () == ID_TextEdit) {
        return ((TextEdit*) I)->GetAllowedChars ();
    } else {
        // Error
        FAIL ("GetCharset: Invalid item type");
        return * (CharSet*) NULL;
    }
}



void ResEditApp::SetCharSet (WindowItem* I, const CharSet& CS)
// Set a character set
{
    if (I->StreamableID () == ID_RStringItem) {
        ((RStringItem*) I)->SetAllowedChars (CS);
    } else if (I->StreamableID () == ID_TextEdit) {
        ((TextEdit*) I)->SetAllowedChars (CS);
    } else {
        // Error
        FAIL ("GetCharset: Invalid item type");
    }
}



void ResEditApp::ItemMenue (WindowItem* Item)
// Data edit menue
{
    // Menue IDs
    const miID          = 10;
    const miText        = 20;
    const miState       = 30;
    const miAccelKey    = 40;
    const miHelpKey     = 50;
    const miCharset     = 60;
    const miLimits      = 70;
    const miSubMenue    = 80;
    const miToggleText  = 90;
    const miToggleCount = 100;

    // Load the menue from the resource
    Menue* M = (Menue*) LoadResource ("@RESITEM.ItemDataMenue");

    // Place the menue near the item to edit
    M->PlaceNear (Item);

    // Transfer generic item data to the menue
    M->SetLongValue (miID, Item->GetID ());
    M->SetStringValue (miText, GetItemText (Item));
    M->SetToggleValue (miState, GetItemState (Item));
    M->SetStringValue (miAccelKey, GetKeyName (Item->AccelKey));
    M->SetStringValue (miHelpKey, Item->HelpKey);

    // Some menue items have special data:
    int IsToggle = 0;
    switch (Item->StreamableID ()) {

        case ID_RStringItem:
        case ID_TextEdit:
            M->SetStringValue (miCharset, GetCharSetString (Item));
            M->ActivateItem (miCharset);
            break;

        case ID_HexEdit:
        case ID_LongEdit:
        case ID_FloatEdit:
        case ID_HexItem:
        case ID_LongItem:
        case ID_FloatItem:
            M->ActivateItem (miLimits);
            break;

        case ID_MenueBarItem:
        case ID_SubMenueItem:
            M->ActivateItem (miSubMenue);
            break;

        case ID_ToggleItem:
        case ID_NoYesItem:
        case ID_OffOnItem:
            IsToggle = 1;
            M->SetStringValue (miToggleText, ((ToggleItem*)Item)->TList);
            M->SetLongValue (miToggleCount, ((ToggleItem*)Item)->TCount);
            M->ActivateItem (miToggleText);
            M->ActivateItem (miToggleCount);
            break;

    }

    // New statusline
    StatusLine->Push (siAccept | siSelectChooseKeys);

    // Now let the user decide what to do
    int Done = 0;
    M->Activate ();
    while (!Done) {

        // Variables needed in the loop
        Window* W;
        int ID;
        Key K;

        // Get a choice
        int Choice = M->GetChoice ();

        // Actions...
        switch (Choice) {

            case miID:
                // Check if the ID is not already in use
                ID = M->GetLongValue (miID);
                if (((GenericMenue*)Res)->ItemWithID (ID) != NULL) {
                    ErrorMsg ("ID is already in use");
                } else {
                    // Assign the new ID
                    Item->ID = ID;

                    // Resource has been changed
                    ResChanged = 1;
                }
                break;

            case miText:
                // Set the new text
                SetItemText (Item, M->GetStringValue (miText));
                ResChanged = 1;
                break;

            case miState:
            case miState+1:
            case miState+2:
                SetItemState (Item, Choice - miState);
                ResChanged = 1;
                break;

            case miAccelKey:
                // Choose the accel key to assign
                W = MsgWindow (" Press accelerator key or Esc to abort", "", paCyan);
                K = KbdGet ();
                if (K != vkAbort && K != kbEsc) {
                    // Set the accel key
                    Item->SetAccelKey (K);

                    // Redraw the item, rebuild the item string before
                    Item->SetWidth (Item->GetWidth ());
                    Item->Draw ();

                    // Display the accel key in the menue
                    M->SetStringValue (miAccelKey, GetKeyName (K));

                    // Resource has changed
                    ResChanged = 1;
                }
                // Delete the window
                delete W;
                break;

            case miHelpKey:
                Item->SetHelpKey (M->GetStringValue (miHelpKey));
                ResChanged = 1;
                break;

            case miCharset:
                ChangeCharset (Item);
                M->SetStringValue (miCharset, GetCharSetString (Item));
                break;

            case miLimits:
                ChangeLimits (Item);
                break;

            case miSubMenue:
                ChangeSubMenue (Item);
                break;

            case miToggleText:
                ((ToggleItem*)Item)->TList = M->GetStringValue (miToggleText);
                ResChanged = 1;
                break;

            case miToggleCount:
                ((ToggleItem*)Item)->TCount = M->GetLongValue (miToggleCount);
                ResChanged = 1;
                break;

            case 0:
                // Check some stuff before end
                if (IsToggle) {
                    ToggleItem* TI = (ToggleItem*) Item;
                    if (TI->TList.Len () % TI->TCount != 0) {
                        ErrorMsg ("Invalid toggle count/length");
                    } else {
                        TI->TLen = TI->TList.Len () / TI->TCount;
                        TI->Draw ();
                        Done = 1;
                    }
                } else {
                    Done = 1;
                }
                break;
        }

    }

    // Restore the statusline, delete the menue
    PopStatusLine ();
    delete M;
}



void ResEditApp::ChangeLimits (WindowItem* Item)
{
    i32 LMin, LMax;
    double FMin, FMax;
    int Abort;

    // Switch according to the item type
    switch (Item->StreamableID ()) {

        case ID_HexEdit:
        case ID_LongEdit:
            ((LongEdit*) Item)->GetMinMax (LMin, LMax);
            MinMaxPrompt (LMin, LMax, (i32) 0x80000000, 0x7FFFFFFF, Abort);
            if (Abort) {
                return;
            }
            ((LongEdit *) Item)->SetMinMax (LMin, LMax);
            ResChanged = 1;
            break;

        case ID_HexItem:
        case ID_LongItem:
            ((LongItem*) Item)->GetMinMax (LMin, LMax);
            MinMaxPrompt (LMin, LMax, (i32) 0x80000000, 0x7FFFFFFF, Abort);
            if (Abort) {
                return;
            }
            ((LongItem *) Item)->SetMinMax (LMin, LMax);
            ResChanged = 1;
            break;

        case ID_FloatEdit:
            ((FloatEdit*) Item)->GetMinMax (FMin, FMax);
            FMinMaxPrompt (FMin, FMax, -1000000000, 1000000000, Abort);
            if (Abort) {
                return;
            }
            ((FloatEdit*) Item)->SetMinMax (FMin, FMax);
            ResChanged = 1;
            break;

        case ID_FloatItem:
            ((FloatItem*) Item)->GetMinMax (FMin, FMax);
            FMinMaxPrompt (FMin, FMax, -1000000000, 1000000000, Abort);
            if (Abort) {
                return;
            }
            ((FloatItem*) Item)->SetMinMax (FMin, FMax);
            ResChanged = 1;
            break;

        default:
            ErrorMsg ("Cannot change limits of this item type");
            break;

    }
}



void ResEditApp::ChangeCharset (WindowItem* Item)
{
    const miReset       = 1;
    const miAddDigits   = 2;
    const miAddHex      = 3;
    const miAddAlpha    = 4;
    const miAddAll      = 5;
    const miCustom      = 6;
    const miAllowEmpty  = 7;


    // Only RStringItem and TextEdits are allowed to change the charset
    unsigned ID = Item->StreamableID ();
    if (ID != ID_RStringItem && ID != ID_TextEdit) {
        ErrorMsg ("Operation not allowed on this type of item");
        return;
    }

    // Load the menue
    Menue* M = (Menue*) LoadResource ("@RESITEM.CharsetMenue");

    // Change the statusline and activate the menue
    PushStatusLine (siEnd | siSelectChooseKeys);
    M->Activate ();

    CharSet CS;
    int Done = 0;
    while (!Done) {

        String S;
        int Abort;

        switch (M->GetChoice ()) {

            case 0:
                Done = 1;
                break;

            case miReset:
                CS.Clear ();
                SetCharSet (Item, CS);
                ResChanged = 1;
                break;

            case miAddDigits:
                CS = GetCharSet (Item);
                CS.AddRange ('0', '9');
                SetCharSet (Item, CS);
                ResChanged = 1;
                break;

            case miAddHex:
                CS = GetCharSet (Item);
                CS.AddRange ('0', '9');
                CS.AddRange ('A', 'F');
                CS.AddRange ('a', 'f');
                SetCharSet (Item, CS);
                ResChanged = 1;
                break;

            case miAddAlpha:
                CS = GetCharSet (Item);
                CS.AddRange ('a', 'z');
                CS.AddRange ('A', 'Z');
                SetCharSet (Item, CS);
                ResChanged = 1;
                break;

            case miAddAll:
                CS = GetCharSet (Item);
                CS.AddRange (0x20, 0xFF);
                SetCharSet (Item, CS);
                ResChanged = 1;
                break;

            case miCustom:
                S = GetCharSetString (Item);
                NamePrompt ("Charset", S, Abort);
                if (!Abort) {
                    SetCharSetString (Item, S);
                    ResChanged = 1;
                }
                break;

            case miAllowEmpty:
                if (ID == ID_RStringItem) {
                    ((RStringItem*) Item)->AllowEmptyInput ();
                } else if (ID == ID_TextEdit) {
                    ((TextEdit*) Item)->AllowEmptyInput ();
                }
                break;

        }
    }

    // Pop the status line, delete the menue
    PopStatusLine ();
    delete M;
}



void ResEditApp::ChangeSubMenue (WindowItem* Item)
{
    // Try to load the submenue for the item
    String MenueName = ChooseRes ();
    if (MenueName.IsEmpty ()) {
        return;
    }
    Streamable* M = LoadRes (MenueName);
    if (M == 0) {
        return;
    }

    // Check if we really have a menue
    if (M->StreamableID () != ID_Menue) {
        ErrorMsg ("Must have a menue object for assignment");
        delete M;
        return;
    }

    // Change the submenue
    ((SubMenueItem*) Item)->SetSubMenue ((Menue*) M);

    // Resource has changed
    ResChanged = 1;
}



void ResEditApp::Order ()
{
    // Cast the resource pointer
    ItemWindow * Win = (ItemWindow *) Res;

    // Check if the window has items
    if (Win->ItemCount == 0) {
        ErrorMsg ("No items - sorry");
        return;
    }

    PushStatusLine (" Choose the first item");

    // Choose the first item
    WindowItem *Item = ChooseItem ();
    if (Item == NULL) {
        // Abort
        return;
    }

    // Unlink this item and place it in front of all other items
    Win->FirstItem = Item;

    StatusLine->Replace (" Choose next item");

    // Now get one item after the other and place it in a row
    WindowItem *NextItem;
    while ((NextItem = ChooseItem ()) != NULL && NextItem != Win->FirstItem) {

        // If the same item is selected twice, this would result in an error.
        // Since this happens usually as a result of pressing the wrong key,
        // print an error message  and ignore it.
        if (NextItem == Item) {
            ErrorMsg ("Cannot select the same item twice");
        } else {
            NextItem->INode.Unlink ();
            NextItem->INode.InsertAfter (&Item->INode);
            Item = NextItem;
        }
    }

    PopStatusLine ();

    ResChanged = 1;

}



