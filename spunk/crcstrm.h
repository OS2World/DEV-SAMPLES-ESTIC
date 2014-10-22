/*****************************************************************************/
/*                                                                           */
/*                                 CRCSTRM.H                                 */
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



#ifndef _CRCSTRM_H
#define _CRCSTRM_H



#include "crc.h"
#include "nullstrm.h"



/*****************************************************************************/
/*                              class CRCStream                              */
/*****************************************************************************/



class CRCStream : public NullStream {

protected:
    u32 CRC;

public:
    CRCStream ();

    // Derived from class NullStream
    virtual void Write (const void *Buf, size_t Count);

    // New functions
    u32 GetCRC () const;

};



inline CRCStream::CRCStream () :
    NullStream (),
    CRC (0)
{
}



inline u32 CRCStream::GetCRC () const
{
    return CRC;
}



/*****************************************************************************/
/*                            Outside the classes                            */
/*****************************************************************************/



u32 GetCRC (const Streamable*);
u32 GetCRC (const Streamable&);
// Return a CRC value for the instance data



// End of CRCSTRM.H

#endif
