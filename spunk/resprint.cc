/*****************************************************************************/
/*                                                                           */
/*                                RESPRINT.CC                                */
/*                                                                           */
/* (C) 1993,94  Ullrich von Bassewitz                                        */
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



#include "resed.h"
#include "strcvt.h"



/*****************************************************************************/
/*                             class ResEditApp                              */
/*****************************************************************************/



int ResEditApp::PrintOneItem (ListNode<WindowItem>* Node, void* _F)
// Print the data of one menue item
{
    // Get a pointer to the window item
    WindowItem* I = Node->Contents ();

    // Create the string
    String Line (I->GetItemText ());
    Line.Trunc (30);
    Line.Pad (String::Right, 31);

    String State ("Unknown");
    if (I->IsSelected ()) {
        State = "Selected";
    } else if (I->IsActive ()) {
        State = "Active";
    } else if (I->IsGrayed ()) {
        State = "Grayed";
    }
    Line += State.Pad (String::Right, 16) +
            I32Str (I->GetID ()).Pad (String::Left, 3) +
            "    " +
            U32Str (I->GetAccelKey (), 16).Pad (String::Left, 4, '0');

    fprintf ((FILE*) _F, "%s\n", Line.GetStr ());

    return 0;

}



void ResEditApp::PrintMenue (FILE* F)
// Print the item data into a file
{
    // Cast the resource into type menue
    GenericMenue* M = (GenericMenue*) Res;

    // Build some sort of header
    String Header1 ("Resource: ");
    Header1 += ResName;
    String Header2 ("Text                           State           ID     Key");
    String Header3 (75);
    Header3.Set (0, 75, '-');
    fprintf (F, "%s\n\n", Header1.GetStr ());
    fprintf (F, "%s\n", Header2.GetStr ());
    fprintf (F, "%s\n", Header3.GetStr ());

    // Traverse through the menue, printing data about all items
    M->Traverse (PrintOneItem, F);
}



int ResEditApp::PrintOneMsg (Msg* M, void* _F)
// Print one message
{
    // Get the line and convert control chars
    String S (::ShowControls (*M, ccHex | ccCStyle | ccSpunk));

    // Convert the string to the current output character set
    S.OutputCvt ();

    // Print the line to print
    fprintf ((FILE*) _F, "%5u  \"%s\"\n", M->GetNum (), S.GetStr ());
    return 0;
}



void ResEditApp::PrintMsgCollection (FILE* F)
// Print the contents of a message collection
{
    ((MsgCollection*) Res)->Traverse (1, PrintOneMsg, F);
}



void ResEditApp::Print ()
{
    // Name of the output file
    static String OutFile ("PRN");

    // Ask for the file name. The name may be a device like PRN
    int Abort;
    NamePrompt (" Output device ", OutFile, Abort);
    if (Abort) {
        return;
    }

    // Try to open the output file
    FILE* F = fopen (OutFile.GetStr (), "wt");
    if (F == NULL) {
        ErrorMsg (FormatStr ("Error opening %s", OutFile.GetStr ()));
        return;
    }

    // Decide, what to print
    switch (ResID) {

        case ID_MsgCollection:
            PrintMsgCollection (F);
            break;

        case ID_Menue:
            PrintMenue (F);
            break;

        default:
            ErrorMsg ("Cannot print: Unknown resource type");
            break;

    }

    // Close the output file
    (void) fclose (F);
}



