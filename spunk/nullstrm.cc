/*****************************************************************************/
/*									     */
/*				  NULLSTRM.CC				     */
/*									     */
/* (C) 1993 Ullrich von Bassewitz					     */
/*	    Zwehrenbuehlstrasse 33					     */
/*	    D-72070 Tuebingen						     */
/* EMail:   uz@moppi.sunflower.sub.org					     */
/*									     */
/*****************************************************************************/



// $Id$
//
// $Log$
//
//



#include "strmable.h"
#include "stream.h"
#include "nullstrm.h"


// Return the data size of an instance
u32 GetSize (const Streamable *O)
{
    // Create a null stream
    NullStream S;

    // Write the object to the stream
    S.Put (O);

    // Current stream position is the size of the object
    return S.GetPos ();

}



