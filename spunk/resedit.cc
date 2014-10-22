/*****************************************************************************/
/*                                                                           */
/*                                 RESFILE.H                                 */
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



#include "strparse.h"
#include "filesel.h"

#include "resed.h"



/*****************************************************************************/
/*                      Explicit template instantiation                      */
/*****************************************************************************/



#ifdef EXPLICIT_TEMPLATES
template class ListBox<Msg>;
#endif



/*****************************************************************************/
/*                                MsgListBox                                 */
/*****************************************************************************/



class MsgListBox: public ListBox<Msg> {

protected:
    virtual void Print (int Index, int X, int Y, u16 Attr);
    // Display one of the listbox entries

public:
    MsgListBox (i16 aID, const Point & aSize, WindowItem *NextItem);

};



MsgListBox::MsgListBox (i16 aID, const Point & aSize, WindowItem * NextItem) :
        ListBox<Msg> ("", aID, aSize, NextItem)
{
}



void MsgListBox::Print (int Index, int X, int Y, u16 Attr)
{
    Msg *M = Coll->At (Index);

    // Create the index number as a string
    String S (::ShowControls (FormatStr ("%6u  %s", M->GetNum (),
                              M->GetStr ()),
                              ccHex | ccCStyle | ccSpunk));

    S.Pad (String::Right, Size.X);
    Owner->Write (X, Y, S, Attr);
}



/*****************************************************************************/
/*                             class ResEditApp                              */
/*****************************************************************************/



void ResEditApp::EditResource ()
{
    switch (ResID) {

        case ID_MsgCollection:
            EditMsgCollection ();
            break;

    }
}



int ResEditApp::SearchMsgID (MsgCollection *M, u16 StartNum)
// Search for a free Message ID. Will fail silently if all 65536 message
// numbers are in use ...
{
    int Index;

    // Search for the first number
    M->Search (&StartNum, Index);

    // Proceed until a free id is found
    while (Index < M->GetCount () && M->At (Index)->GetNum () == StartNum) {
        Index++;
        StartNum++;
    }
    return StartNum;
}



void ResEditApp::EditMsgCollection ()
//
{
    // Last used Msg number
    static u16 MsgNum = 0;

    // Get a correct casted pointer
    MsgCollection *M = (MsgCollection *) Res;

    // Make a window not covering the menuebar and the statusline
    Rect WinBounds (Background->OuterBounds ());
    WinBounds.A.Y = 1;
    WinBounds.B.Y--;

    // create an empty listbox
    Point BoxSize (WinBounds.XSize () - 2, WinBounds.YSize () - 2);
    MsgListBox *L = new MsgListBox (1, BoxSize, NULL);

    // Assign the collection for the listbox
    L->SetColl (M);

    // Create the window and set the header
    ItemWindow *Win = new ItemWindow (WinBounds, wfFramed, paGray, 0, L);
    Win->SetHeader (" Available Messages ");

    // Make a statusline and activate the window
    u32 StatusFlags = siEsc_End | siDel_Delete | siIns_Insert | siCR_Change;
    StatusLine->Push (StatusLine::CreateLine (StatusFlags) + " ~Alt-N~ Change number");
    Win->Activate ();

    // Ask for the keys and handle them
    Key K;
    int Done = 0;
    int Selected;
    int Index;
    int Abort;
    i32 Val;
    Msg *OldMsg, *NewMsg;
    while (!Done) {

        K = KbdGet ();
        L->HandleKey (K);
        Selected = L->GetSelected ();

        switch (K) {

            case vkAbort:
            case kbEsc:
                Done = 1;
                break;

            case vkDel:
                L->Delete (Selected);
                ResChanged = 1;
                break;

            case kbMetaN:
                if (Selected >= 0) {
                    OldMsg = M->At (Selected);
                    Val = OldMsg->GetNum ();
                    LongPrompt (" New message number", Val, 0, 0xFFFF, Abort);
                    if (!Abort) {
                        // Check if the message number is in use
                        MsgNum = (u16) Val;
                        if (M->Search (&MsgNum, Index)) {
                            ErrorMsg ("Message id is in use");
                        } else {
                            NewMsg = new Msg (MsgNum, *OldMsg);
                            L->Delete (Selected);
                            L->Insert (NewMsg);
                            ResChanged = 1;
                        }
                    }
                }
                break;

            case vkIns:
                // Search for a free message number
                Val = SearchMsgID (M, MsgNum);
                // Edit that number
                LongPrompt (" New message number", Val, 0, 0xFFFF, Abort);
                if (!Abort) {
                    // Check if the message number is in use
                    MsgNum = (u16) Val;
                    if (M->Search (&MsgNum, Index)) {
                        ErrorMsg ("Message id is in use");
                    } else {
                        NewMsg = new Msg (MsgNum);
                        NamePrompt (" New message", *NewMsg, Abort);
                        if (!Abort) {
                            NewMsg->HideControls ();
                            L->Insert (NewMsg);
                            L->SetSelected (M->IndexOf (NewMsg));
                            ResChanged = 1;
                        } else {
                            delete NewMsg;
                        }
                    }
                }
                break;

            case kbEnter:
                if (Selected >= 0) {
                    NewMsg = new Msg (*(M->At (Selected)));
                    NewMsg->ShowControls (ccHex | ccCStyle | ccSpunk);
                    NamePrompt (" New Message", *NewMsg, Abort);
                    if (!Abort) {
                        NewMsg->HideControls ();
                        L->Replace (Selected, NewMsg);
                        ResChanged = 1;
                    } else {
                        delete NewMsg;
                    }
                }
                break;

        }

    }

    // Take the collection from the listbox
    L->SetColl (NULL);

    // Delete the window including the listbox
    delete Win;

    // Restore the old status line
    StatusLine->Pop ();
}



void ResEditApp::MergeMsgText ()
// Merge a text file containing messages
{
    // Ask for the file name
    FileSelector FS (" Open a merge file ", ".msg");
    String MergeFile = FS.GetChoice ();
    if (MergeFile.IsEmpty ()) {
        return;
    }

    // Open the file
    FILE* F;
    if ((F = fopen (MergeFile.GetStr (), "rb")) == NULL) {
        ErrorMsg (FormatStr ("Cannot open %s", MergeFile.GetStr ()));
        return;
    }

    // Get a casted pointer to the message collection
    MsgCollection* M = (MsgCollection*) Res;

    // Read line by line, convert and set up a new message
    int Line = 0;
    while (1) {
        char Buf [512];
        if (fgets (Buf, sizeof (Buf), F) == NULL) {
            // End of file reached
            break;
        }
        Line++;

        // Put the line into a string, then skip white space at the beginning
        String S (Buf);
        while (S.IsEmpty () == 0 && S [0] == ' ') {
            S.Del (0, 1);
        }

        // Delete the trailine newline
        int Len = S.Len ();
        while (S.IsEmpty () == 0 && (S [Len-1] == '\n' || S [Len-1] == '\n')) {
            Len--;
            S.Trunc (Len);
        }

        // Ignore empty and comment lines
        if (S.IsEmpty () || S [0] == ';') {
            continue;
        }

        // Convert the line to the current input charset
        S.InputCvt ();

        // Check if the number at the beginning is hexadecimal.
        unsigned Base = 10;
        if (S.Len () > 2 && S [0] == '0' && S [1] == 'x') {
            S.Del (0, 2);
            Base = 16;
        }

        // Now parse for the message number
        u32 MsgNum;
        StringParser P (S, StringParser::SkipWS         |
                           StringParser::PascalHex      |
                           StringParser::CHex);
        if (P.GetU32 (MsgNum, Base) != 0) {
            ErrorMsg (FormatStr ("Invalid message number in line %d", Line));
            continue;
        }

        // Check the message number for valid values
        if (MsgNum >= 0x10000) {
            ErrorMsg (FormatStr ("Invalid message number in line %d", Line));
            continue;
        }

        // Skip white space and check for double quotes
        P.SkipWhite ();
        if (S [P.GetPos ()] != '"') {
            ErrorMsg (FormatStr ("Missing leading quotes in line %d", Line));
            continue;
        }

        // Extract the message
        String MsgText (S.Cut (P.GetPos () + 1, S.Len () - P.GetPos ()));

        // Cut anything behind the trailing double quote
        int Pos = MsgText.ScanL ('"');
        if (Pos < 0) {
            ErrorMsg (FormatStr ("Unterminated message in line %d", Line));
            continue;
        }
        MsgText.Trunc (Pos);

        // Convert visible control chars to binary values and create a new msg
        Msg* NewMsg = new Msg (MsgNum, ::HideControls (MsgText));

        // Check if the message collection already has a message with the given
        // message number. If so, delete this message
        u16 Key = MsgNum;
        int Index;
        if (M->Search (&Key, Index) != 0) {
            // A msg with this number exists, delete it
            M->AtDelete (Index);
        }

        // Now insert the new message
        M->Insert (NewMsg);

    }

    // Close the file and mark the resource as modified
    ResChanged = 1;
    fclose (F);

}



