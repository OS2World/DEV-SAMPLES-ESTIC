/*****************************************************************************/
/*                                                                           */
/*                                 CRCSTRM.CC                                */
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



#include "strmable.h"
#include "crcstrm.h"



/*****************************************************************************/
/*                              class CRCStream                              */
/*****************************************************************************/



void CRCStream::Write (const void *Buf, size_t Count)
{
    CRC = CRC_32 (CRC, Buf, Count);
}



/*****************************************************************************/
/*                            Outside the classes                            */
/*****************************************************************************/



u32 GetCRC (const Streamable* O)
{
    CRCStream S;
    S << *O;
    return S.GetCRC ();
}



u32 GetCRC (const Streamable& O)
{
    CRCStream S;
    S << O;
    return S.GetCRC ();
}



