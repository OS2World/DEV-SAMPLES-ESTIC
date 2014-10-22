/*****************************************************************************/
/*                                                                           */
/*                                 STRMABLE.H                                */
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



#ifndef _STRMABLE_H
#define _STRMABLE_H



#include "machine.h"
#include "object.h"



// Dummy argument for build constructor
enum StreamableInit { Empty };



/*****************************************************************************/
/*                             class Streamable                              */
/*****************************************************************************/



// Forwards
class Stream;

// Every streamable object must be derived from class Streamable
class Streamable : public Object {

public:
    virtual void Load (Stream&);
    virtual void Store (Stream&) const;
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    friend inline Stream& operator << (Stream&, const Streamable&);
    friend inline Stream& operator >> (Stream&, Streamable&);
    friend inline Stream& operator << (Stream&, const Streamable*);
    friend inline Stream& operator >> (Stream&, Streamable*);

};



inline Stream& operator << (Stream& S, const Streamable& O)
{
    O.Store (S);
    return S;
}



inline Stream& operator >> (Stream& S, Streamable& O)
{
    O.Load (S);
    return S;
}



inline Stream& operator << (Stream& S, const Streamable* O)
{
    return S << *O;
}



inline Stream& operator >> (Stream& S, Streamable* O)
{
    return S >> *O;
}



// End of STRMABLE.H

#endif
