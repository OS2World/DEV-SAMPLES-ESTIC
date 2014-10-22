/*****************************************************************************/
/*									     */
/*				     RNG.CC				     */
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



#include "check.h"
#include "streamid.h"
#include "stream.h"
#include "rng.h"



// Register the classes
LINK (RNG, ID_RNG);



/*****************************************************************************/
/*				 class BaseRNG				     */
/*****************************************************************************/



u32 BaseRNG::Get (u32 Range)
// Get a random number in the range 0..Range-1
{
    PRECONDITION (Range <= GetRange ());

    // Handle ranges < 2 correct
    if (Range <= 1) {
	return 0;
    } else {
	return GetUnscaled () % Range;
    }
}



double BaseRNG::Get ()
// Get a random number in the range [0.0 .. 1.0[
{
    return double (GetUnscaled ()) / double (GetRange ());
}



/*****************************************************************************/
/*				   class RNG				     */
/*****************************************************************************/



RNG::RNG (u32 Seed):
    Front (1),
    Rear (0)
// Create a random number generator with the given seed
{
    Randomize (Seed);
}



RNG::RNG (StreamableInit)
// Create an empty RNG
{
}



void RNG::Randomize (u32 Seed)
// Intialize the RNG for a specific random number stream
{
    // Initialize the state array
    State [0] = Seed;
    for (unsigned I = 1; I < sizeof (State) / sizeof (State [0]); I++) {
	State [I] = 1103515245L * State [I - 1] + 12345;
    }
    Front = 1;
    Rear = 0;

    // Shuffle the state array
    for (unsigned J = 0; J < 10 * (sizeof (State) / sizeof (State [0])); J++) {
	(void) GetUnscaled ();
    }
}


u32 RNG::GetUnscaled ()
// Get an unscaled value from the RNG
{
    // Create the next random number
    State [Front] += State [Rear];
    u32 Val = State [Front];
    if (++Front >= sizeof (State) / sizeof (State [0])) {
	Front = 0;
	Rear++;
    } else if (++Rear >= sizeof (State) / sizeof (State [0])) {
	Rear = 0;
    }
    return Val;
}



u32 RNG::GetRange () const
// Return the range of different values, the random number generator
// is able to return.
{
    // Oh dear, this is incorrect!
    return 0xFFFFFFFF;
}



void RNG::Load (Stream& S)
// Load the RNG from a stream
{
    for (unsigned I = 0; I < sizeof (State) / sizeof (State [0]); I++) {
	S >> State [I];
    }
    S >> Front >> Rear;
}



void RNG::Store (Stream& S) const
// Store the RNG into a stream
{
    for (unsigned I = 0; I < sizeof (State) / sizeof (State [0]); I++) {
	S << State [I];
    }
    S << Front << Rear;
}



u16 RNG::StreamableID () const
// Return the stream ID of the RNG class
{
    return ID_RNG;
}



Streamable* RNG::Build ()
// Return an empty RNG object
{
    return new RNG (Empty);
}



