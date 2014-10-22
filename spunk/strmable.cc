/*****************************************************************************/
/*                                                                           */
/*                                 STRMABLE.CC                               */
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



#include <stddef.h>

#include "machine.h"
#include "check.h"
#include "object.h"
#include "strmable.h"



/*****************************************************************************/
/*                             class Streamable                              */
/*****************************************************************************/



void Streamable::Load (Stream&)
{
    ABSTRACT ();
}



void Streamable::Store (Stream&) const
{
    ABSTRACT ();
}



u16 Streamable::StreamableID () const
{
    ABSTRACT ();
    return 0;
}



Streamable* Streamable::Build ()
{
    ABSTRACT ();
    return (Streamable*) NULL;
}


