/*****************************************************************************/
/*                                                                           */
/*                                 LISTNODE.H                                */
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



#ifndef __LISTNODE_H
#define __LISTNODE_H


#include <stddef.h>

#include "machine.h"
#include "check.h"
#include "object.h"




/*****************************************************************************/
/*                             class _ListNode                               */
/*****************************************************************************/



// This is an implementation class for the template class ListNode. It has
// the complete functionality but uses void pointers instead of typed pointers.
// This hopefully will result in smaller programs.

class _ListNode: public Object {

protected:
    // pointer to the data
    void* ContentsPtr;

    // pointers to the prevoius and next node
    _ListNode* PrevNode;
    _ListNode* NextNode;

public:
    _ListNode (void* DataPtr = NULL);
    ~_ListNode ();

    // get contents of the node
    void* Contents ();

    // get the previous and next nodes
    _ListNode* Next ();
    _ListNode* Prev ();

    // check if the list is empty (one node only)
    int IsEmpty ();

    // linking in the list
    void InsertIn (_ListNode* R);
    void InsertAfter (_ListNode* N);
    void InsertBefore (_ListNode* N);

    // unlink a node
    void Unlink ();

    // traverse through all nodes
    _ListNode* Traverse (int Forward, int (*F) (_ListNode*, void*),
                            void* UserPtr = NULL);

    // count number of nodes
    u16 NodeCount ();

    // convert node to number and vice versa
    _ListNode* NodeWithNumber (u16 X);
    u16 NumberOfNode (_ListNode* N);

};



inline void* _ListNode::Contents ()
{
    return ContentsPtr;
}



inline _ListNode* _ListNode::Next ()
{
    return NextNode;
}



inline _ListNode* _ListNode::Prev ()
{
    return PrevNode;
}



inline int _ListNode::IsEmpty ()
{
    return (NextNode == this);
}



inline void _ListNode::InsertIn (_ListNode* R)
{
    InsertAfter (R);
}



/*****************************************************************************/
/*                              class ListNode                               */
/*****************************************************************************/



template <class T> class ListNode: public _ListNode {

public:
    ListNode (T* DataPtr = NULL);

    // get contents of the node
    T* Contents ();

    // get the previous and next nodes
    ListNode<T>* Next ();
    ListNode<T>* Prev ();

    // linking in the list
    void InsertIn (ListNode<T>* R);
    void InsertAfter (ListNode<T>* N);
    void InsertBefore (ListNode<T>* N);

    // traverse through all nodes
    ListNode* Traverse (int Forward, int (*F) (ListNode<T>*, void*),
                            void* UserPtr = NULL);

    // convert node to number and vice versa
    ListNode<T>* NodeWithNumber (u16 X);
    u16 NumberOfNode (ListNode<T>* N);

};



template <class T>
inline ListNode<T>::ListNode (T* DataPtr):
    _ListNode (DataPtr)
{
}



template <class T>
inline T* ListNode<T>::Contents ()
{
    return (T*) _ListNode::Contents ();
}



template <class T>
inline ListNode<T>* ListNode<T>::Next ()
{
    return (ListNode<T>*) _ListNode::Next ();
}



template <class T>
inline ListNode<T>* ListNode<T>::Prev ()
{
    return (ListNode<T>*) _ListNode::Prev ();
}



template <class T>
inline void ListNode<T>::InsertIn (ListNode<T>* R)
{
    _ListNode::InsertIn ((_ListNode*) R);
}



template <class T>
inline void ListNode<T>::InsertAfter (ListNode<T>* N)
// inserts one node after another
{
    _ListNode::InsertAfter ((_ListNode*) N);
}



template <class T>
inline void ListNode<T>::InsertBefore (ListNode<T>* N)
// inserts one node before another
{
    _ListNode::InsertBefore ((_ListNode*) N);
}



template <class T>
inline ListNode<T>* ListNode<T>::Traverse (int Forward,
                                           int (*F) (ListNode<T>*, void*),
                                           void *UserPtr)
// Traverse through a list, starting with the current node and calling the
// given function F with every node as argument. Ends if finally the current
// node is reached again (all nodes have been visited in this case) or if the
// called function returns a value != 0. In the former case, Traverse returns
// a NULL pointer, in the latter, a pointer to the node is returned.
{
    typedef int (*UntypedFunc) (_ListNode*, void*);
    return (ListNode<T>*) _ListNode::Traverse (Forward, (UntypedFunc) F, UserPtr);
}



template <class T>
inline ListNode<T>* ListNode<T>::NodeWithNumber (u16 X)
// Returns the node with number X. Counting begins with "this", (which has
// number 0) and proceeds in "Next" direction.
// Warning: If X is greater than the number of nodes in the list, the
// result is undefined (wraping around).
{
    return (ListNode<T>*) _ListNode::NodeWithNumber (X);
}



template <class T>
inline u16 ListNode<T>::NumberOfNode (ListNode<T>* N)
// Returns the number of node N. Counting begins with "this" node
// (which has number 0) and proceeds in "Next" direction.
{
    return _ListNode::NumberOfNode ((_ListNode*) N);
}



// End of LISTNODE.H

#endif

