/*****************************************************************************/
/*                                                                           */
/*                                SPLITMSG.CC                                */
/*                                                                           */
/* (C) 1993-95  Ullrich von Bassewitz                                        */
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



#include "splitmsg.h"



/*****************************************************************************/
/*                                                                           */
/*****************************************************************************/



ListNode<String>* SplitLine (String Text, Rect &Bounds, unsigned MinLen)
// Split the given string into a list of lines. MinLen is used if any of the
// lines is requested to be centered. In this case, MinLen is the minimum
// length of the centered line (used in windows with header strings).
{
    String* S;

    // Check the parameters
    unsigned Len = Text.Len ();
    PRECONDITION (Len > 0);

    // Initialize bounds
    Bounds.A.X = Bounds.A.Y = 0;

    // Split the text into lots of lines remembering the longest line
    int Longest = MinLen;
    ListNode<String>* Node = NULL;
    while (Len) {

        // Get the end of the line
        int Pos = Text.Pos ('\n');

        //
        int L;
        if (Pos < 0) {
            // Last line
            S = new String (Text);
            L = Text.Len ("@~");
            if (L > Longest) {
                Longest = L;
            }
            Len = 0;
        } else {
            // Cut the piece from the string, deleting the newline
            S = new String (Text.Cut (0, Pos));
            Text.Del (0, Pos + 1);
            Len -= Pos + 1;

            // Get the length and compare to Longest
            L = S->Len ("@~\x01");      // Do not count caret/bar/center
            if (L > Longest) {
                Longest = L;
            }
        }

        // Create a new listnode
        ListNode<String>* N = new ListNode<String> (S);

        // Insert this node into the list
        if (Node) {
            N->InsertBefore (Node);
        } else {
            Node = N;
        }

    }

    // Now the length of the longest line is in Longest. Walk through
    // the line list, delete the center char and center lines if requested.
    // Count the nodes
    ListNode<String>* N = Node;
    int Count = 0;
    do {
        // Get pointer to line
        String& S = *(N->Contents ());
        Count++;

        // Check for center flag if string is not empty
        if (S.IsEmpty () == 0 && S [0] == '\x01') {
            // Delete the center flag
            S.Del (0, 1);

            // Center the string
            S.Pad (String::Center, Longest);
        }

        // Set next node
        N = N->Next ();

    } while (N != Node);

    // Set up the surrounding rectangle
    Bounds.B.X = (i16) Longest;
    Bounds.B.Y = (i16) Count;

    // Return the list
    return Node;
}



void ReleaseLines (ListNode<String> *Node)
// Release a line list build from SplitLine
{
    ListNode<String> *N = Node;

    while (!Node->IsEmpty ()) {

        // Get the next node and unlink it
        N = Node->Next ();
        N->Unlink ();

        // Delete the contents of the node and the node
        delete N->Contents ();
        delete N;

    }

    // Now delete the root node
    delete Node->Contents ();
    delete Node;
}





