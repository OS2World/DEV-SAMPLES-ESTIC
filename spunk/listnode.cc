/*****************************************************************************/
/*                                                                           */
/*                                 LISTNODE.CC                               */
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



#include "listnode.h"



/*****************************************************************************/
/*                             class _ListNode                               */
/*****************************************************************************/



_ListNode::_ListNode (void* DataPtr)
{
    NextNode = PrevNode = this;
    ContentsPtr = DataPtr;
}



_ListNode::~_ListNode ()
{
    Unlink ();
}



void _ListNode::InsertAfter (_ListNode* N)
// inserts one node after another
{
    // the node cannot be inserted after itself
    PRECONDITION (N != this);

    // switch pointers
    NextNode = N->NextNode;
    PrevNode = N;
    N->NextNode = this;
    NextNode->PrevNode = this;
}



void _ListNode::InsertBefore (_ListNode* N)
// inserts one node before another
{
    // the node cannot be inserted before itself
    PRECONDITION (N != this);

    // switch pointers
    PrevNode = N->PrevNode;
    NextNode = N;
    N->PrevNode = this;
    PrevNode->NextNode = this;
}



void _ListNode::Unlink ()
// Unlinks a node from a list.
{
    // If the list constists only of this element, no unlink is necessary
    if (NextNode == this) {
        return;
    }

    // unlink node
    PrevNode->NextNode = NextNode;
    NextNode->PrevNode = PrevNode;

    // the unlinked node is a list with one node
    NextNode = PrevNode = this;
}



_ListNode* _ListNode::Traverse (int Forward, int (*F) (_ListNode*, void*),
                                     void *UserPtr)
// Traverse through a list, starting with the current node and calling the
// given function F with every node as argument. Ends if finally the current
// node is reached again (all nodes have been visited in this case) or if the
// called function returns a value != 0. In the former case, Traverse returns
// a NULL pointer, in the latter, a pointer to the node is returned.
{
    _ListNode* N = this;

    do {

        if (F (N, UserPtr)) {
            // function returned true
            return N;
        }

        if (Forward) {
            N = N->NextNode;
        } else {
            N = N->PrevNode;
        }

    } while (N != this);

    // Walked through the whole list, return NULL
    return NULL;
}



u16 _ListNode::NodeCount ()
// Returns the node count of the list
{
    register u16 Count = 0;

    _ListNode* N = this;

    do {
        Count++;
        N = N->NextNode;
    } while (N != this);

    return Count;
}



_ListNode* _ListNode::NodeWithNumber (u16 X)
// Returns the node with number X. Counting begins with "this", (which has
// number 0) and proceeds in "Next" direction.
// Warning: If X is greater than the number of nodes in the list, the
// result is undefined (wraping around).
{
    _ListNode* N = this;

    while (X--) {
        N = N->NextNode;
    }

    return N;
}



u16 _ListNode::NumberOfNode (_ListNode* N)
// Returns the number of node N. Counting begins with "this" node
// (which has number 0) and proceeds in "Next" direction.
{
    register u16 Count = 0;
    _ListNode* R = this;

    while (R != N) {
        Count++;
        R = R->NextNode;
        CHECK (R != this);              // Means N is not in list
    }

    return Count;
}



