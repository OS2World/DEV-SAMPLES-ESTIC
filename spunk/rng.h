/*****************************************************************************/
/*									     */
/*				      RNG.H				     */
/*									     */
/* (C) 1996	Ullrich von Bassewitz					     */
/*		Wacholderweg 14						     */
/*		D-70597 Stuttgart					     */
/* EMail:	uz@ibb.schwaben.com					     */
/*									     */
/*****************************************************************************/



// $Id$
//
// $Log$
//
//



// Random number generator



#ifndef _RNG_H
#define _RNG_H



#include "strmable.h"



/*****************************************************************************/
/*				 class BaseRNG				     */
/*****************************************************************************/



// Class BaseRNG is an abstract base class, it defines an interface, not
// an implementation



class BaseRNG: public Streamable {

protected:
    virtual u32 GetUnscaled () = 0;
    // Get an unscaled value from the RNG

public:
    virtual void Randomize (u32 Seed) = 0;
    // Intialize the RNG for a specific random number stream

    u32 Get (u32 Range);
    // Get a random number in the range 0..Range-1.
    // Beware: Range must not be greater than the value returned by GetRange

    double Get ();
    // Get a random number in the range [0.0 .. 1.0[

    virtual u32 GetRange () const = 0;
    // Return the range of different values, the random number generator
    // is able to return.
};



/*****************************************************************************/
/*				   class RNG				     */
/*****************************************************************************/



// This is a default implementation for the RNG. It uses the polynom
// X**63 + X + 1 for random number generation.



class RNG: public BaseRNG {

    u32 State [63];
    u16 Front;
    u16 Rear;

protected:
    RNG (StreamableInit);
    // Create an empty RNG

    virtual u32 GetUnscaled ();
    // Get an unscaled value from the RNG

public:
    RNG (u32 Seed = 0);
    // Create a random number generator with the given seed

    virtual void Randomize (u32 Seed);
    // Intialize the RNG for a specific random number stream

    virtual u32 GetRange () const;
    // Return the range of different values, the random number generator
    // is able to return.

    virtual void Load (Stream&);
    // Load the RNG from a stream

    virtual void Store (Stream&) const;
    // Store the RNG into a stream

    virtual u16 StreamableID () const;
    // Return the stream ID of the RNG class

    static Streamable* Build ();
    // Return an empty RNG object
};



// End of RNG.H

#endif
