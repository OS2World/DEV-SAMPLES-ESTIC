/*****************************************************************************/
/*                                                                           */
/*                                 CIRCBUF.H                                 */
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



// Some comments on the rewrite (12/96):
//
//   * The size of the circular buffer is now a template argument. This gives
//     somewhat better access times.
//
//   * The counter got removed. This has two implications:
//
//      1. The buffer cannot hold Size items since there is an ambiguitiy
//         when In and Out are equal (this would mean "buffer empty" or
//         "buffer full" if the buffer could hold Size items).
//      2. Get and Put may be called from different threads (class
//         CircularBuffer is thread-safe now). This is true as long as no
//         other functions are called (Load/Store, or even PutInFront are
//         *not* thread-safe).
//
//   * As Get() needs to copy the returned object twice, one should
//     probably put pointers into an CircularBuffer object if the objects
//     are bigger than just a few bytes in size.
//



#ifndef _CIRCBUF_H
#define _CIRCBUF_H



#include "machine.h"
#include "check.h"
#include "object.h"
#include "strmable.h"
#include "stream.h"



/*****************************************************************************/
/*                           class CircularBuffer                            */
/*****************************************************************************/



template <class T, unsigned Size>
class CircularBuffer : public Streamable {

protected:
    T           Data [Size];
    unsigned    In;
    unsigned    Out;

protected:
    virtual void PutItem (Stream& S, T Item) const;
    virtual T GetItem (Stream& S);

public:
    CircularBuffer ();
    // Construct a CircularBuffer object

    CircularBuffer (StreamableInit);
    // Build constructor

    // Derived from class Streamable
    virtual void Load (Stream&);
    virtual void Store (Stream&) const;
    static Streamable* Build ();

    unsigned GetCount () const;
    // Return the count of items in the buffer

    int IsEmpty () const;
    // Return true if the buffer is empty

    int NotEmpty () const;
    // Return true if the buffer is not empty

    int IsFull () const;
    // Return true if the buffer is full

    void Put (const T& Item, int IgnoreOverflow = 1);
    // Put something into the buffer

    void PutInFront (const T& Item, int IgnoreOverflow = 1);
    // Put something as first item into the buffer

    T Get ();
    // Get an item from the buffer

    const T& Peek () const;
    // Peek at the first item in the buffer
};



template <class T, unsigned Size>
CircularBuffer<T, Size>::CircularBuffer () :
    In (0),
    Out (0)
{
    // Check parameters
    PRECONDITION (Size > 0);
}



template <class T, unsigned Size>
inline CircularBuffer<T, Size>::CircularBuffer (StreamableInit)
{
}



template <class T, unsigned Size>
void CircularBuffer<T, Size>::Load (Stream& S)
{
    u16 Count;
    S >> Count;

    // Reset the in/out pointers
    In = Out = 0;

    // Now read the items
    while (Count--) {
        Data [In] = GetItem (S);
        In = (In + 1) % Size;
    }

}



template <class T, unsigned Size>
void CircularBuffer<T, Size>::Store (Stream& S) const
{
    // Store data
    u16 Count = GetCount ();
    S << Count;

    // Store the items
    unsigned O = Out;
    while (Count--) {
        PutItem (S, Data [O]);
        O = (O + 1) % Size;
    }
}



template <class T, unsigned Size>
T CircularBuffer<T, Size>::GetItem (Stream&)
{
    ABSTRACT ();
    return *((T*) NULL);
}



template <class T, unsigned Size>
void CircularBuffer<T, Size>::PutItem (Stream&, T) const
{
    ABSTRACT ();
}



template <class T, unsigned Size>
inline Streamable* CircularBuffer<T, Size>::Build ()
{
    return new CircularBuffer<T, Size> (Empty);
}



template <class T, unsigned Size>
inline unsigned CircularBuffer<T, Size>::GetCount () const
// Return the count of items in the buffer
{
    int Count = int (In) - int (Out);
    if (Count < 0) {
        return Count + Size;
    } else {
        return Count;
    }
}



template <class T, unsigned Size>
inline int CircularBuffer<T, Size>::IsEmpty () const
{
    return (GetCount () == 0);
}



template <class T, unsigned Size>
inline int CircularBuffer<T, Size>::NotEmpty () const
// Return true if the buffer is not empty
{
    return (GetCount () != 0);
}



template <class T, unsigned Size>
inline int CircularBuffer<T, Size>::IsFull () const
{
    return (GetCount () == Size-1);
}



template <class T, unsigned Size>
void CircularBuffer<T, Size>::Put (const T& Item, int IgnoreOverflow)
{
    if (IsFull ()) {
        if (IgnoreOverflow) {
            return;
        } else {
            FAIL ("CircularBuffer::Put: Overflow");
        }
    }

    Data [In] = Item;
    In = (In + 1) % Size;
}



template <class T, unsigned Size>
void CircularBuffer<T, Size>::PutInFront (const T& Item, int IgnoreOverflow)
{
    if (IsFull ()) {
        if (IgnoreOverflow) {
            return;
        } else {
            FAIL ("CircularBuffer::PutInFront: Overflow");
        }
    }

    if (Out == 0) {
        Out = Size - 1;
    } else {
        Out--;
    }
    Data [Out] = Item;
}



template <class T, unsigned Size>
T CircularBuffer<T, Size>::Get ()
{
    // There must be at least one element
    PRECONDITION (NotEmpty ());

    T X = Data [Out];
    Out = (Out + 1) % Size;
    return X;
}



template <class T, unsigned Size>
const T & CircularBuffer<T, Size>::Peek () const
{
    // There must be at least one element
    PRECONDITION (NotEmpty ());
    return Data [Out];
}



// End of CIRCBUF.H

#endif

