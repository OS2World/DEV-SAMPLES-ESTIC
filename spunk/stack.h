/*****************************************************************************/
/*                                                                           */
/*                                   STACK.H                                 */
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



#ifndef __STACK_H
#define __STACK_H



#include <limits.h>

#include "machine.h"
#include "object.h"
#include "strmable.h"
#include "stream.h"



/*****************************************************************************/
/*                           template class Stack                            */
/*****************************************************************************/



template <class T>
class Stack : public Streamable {

protected:
    T  *Data;
    u16 SP;
    u16 Limit;

protected:
    virtual void PutItem (Stream &S, T Item) const;     // Not defined
    virtual T GetItem (Stream &S);                      // Not defined
    virtual void FreeItem (T Item);

public:
    Stack (u16 Size);
    Stack (StreamableInit);
    virtual ~Stack ();

    // Derived from class Streamable
    virtual void Load (Stream&);
    virtual void Store (Stream&) const;
    static Streamable* Build ();

    // New member functions
    int IsEmpty ();
    void Push (T);
    T Pop ();
    T Peek ();
    void Drop ();
    void Swap ();

};



template <class T>
Stack<T>::Stack (u16 Size) :
    SP (0), Limit (Size)
{
    // Allocate memory
    Data = new T [Limit];
}



template <class T>
inline Stack<T>::Stack (StreamableInit)
{
}



template <class T>
Stack<T>::~Stack ()
{
    // Free remaining items
    while (SP--) {
        FreeItem (Data [SP]);
    }

    // Free the array
    delete [] Data;
}



template <class T>
void Stack<T>::Load (Stream& S)
{
    // Read data
    S >> SP >> Limit;

    // Allocate memory
    Data = new T [Limit];

    // Now read the items
    u16 I = SP;
    while (I--) {
        Data [I] = GetItem (S);
    }
}



template <class T>
void Stack<T>::Store (Stream& S) const
{
    // Store data
    S << SP << Limit;

    // Store the items
    u16 I = SP;
    while (I--) {
        PutItem (S, Data [I]);
    }
}



template <class T>
T Stack<T>::GetItem (Stream&)
{
    ABSTRACT ();
    return * (T *) NULL;
}



template <class T>
void Stack<T>::PutItem (Stream&, T) const
{
    ABSTRACT ();
}



template <class T>
inline void Stack<T>::FreeItem (T)
{
    // Default is to do nothing
}



template <class T>
inline Streamable * Stack<T>::Build ()
{
    return new Stack<T> (Empty);
}



template <class T>
inline int Stack<T>::IsEmpty ()
{
    return (SP == 0);
}



template <class T>
void Stack<T>::Push (T O)
{
    // Check for overflow
    CHECK (SP < Limit);

    // Push the item
    Data [SP++] = O;
}



template <class T>
T Stack<T>::Pop ()
{
    // There must be at least one element
    CHECK (SP >= 1);

    // Pop an element
    return Data [--SP];
}



template <class T>
T Stack<T>::Peek ()
{
    // There must be at least one element
    CHECK (SP >= 1);

    // Pop an element
    return Data [SP-1];
}



template <class T>
void Stack<T>::Drop ()
{
    // There must be at least one element
    CHECK (SP >= 1);

    // Drop the last element
    FreeItem (Data [--SP]);
}



template <class T>
void Stack<T>::Swap ()
{
    // Swap needs two elements at least
    CHECK (SP >= 2);

    // Swap the elements
    T Tmp = Data [SP-1];
    Data [SP-1] = Data [SP-2];
    Data [SP-2] = Tmp;
}



// End of STACK.H

#endif

 
