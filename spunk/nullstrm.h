/*****************************************************************************/
/*									     */
/*				   NULLSTRM.H				     */
/*									     */
/* (C) 1993,94	Ullrich von Bassewitz					     */
/*		Zwehrenbuehlstrasse 33					     */
/*		D-72070 Tuebingen					     */
/* EMail:	uz@ibb.schwaben.com					     */
/*									     */
/*****************************************************************************/



// $Id$
//
// $Log$
//
//



#ifndef __NULLSTRM_H
#define __NULLSTRM_H



#include "machine.h"
#include "strmable.h"
#include "stream.h"



/*****************************************************************************/
/*			       class NullStream				     */
/*****************************************************************************/



class NullStream : public Stream {

protected:
    u32 CurrentPos;

public:
    NullStream ();

    virtual u32 GetPos ();
    virtual void Write (const void* Buf, size_t Count);

};



inline NullStream::NullStream ():
    Stream (),
    CurrentPos (0)
{
}



inline u32 NullStream::GetPos ()
{
    return CurrentPos;
}




inline void NullStream::Write (const void*, size_t Count)
{
    CurrentPos += Count;
}



/*****************************************************************************/
/*			      Outside the classes			     */
/*****************************************************************************/



u32 GetSize (const Streamable*);
// Return the data size of an instance

inline u32 GetSize (const Streamable& O)
// Return the data size of an instance
{
    return GetSize (&O);
}



// End of NULLSTRM.H

#endif

