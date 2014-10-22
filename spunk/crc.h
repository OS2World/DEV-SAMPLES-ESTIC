/*****************************************************************************/
/*                                                                           */
/*                                   CRC.H                                   */
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



// The functions declared in this header file are contained in more than one
// .cc file. This is because rather large tables are used and with more than
// one object file, the linker is able to sort out the needed ones.



#ifndef _CRC_H
#define _CRC_H



#include <stdlib.h>

#include "machine.h"



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



u16 CRC_CCITT (u16 StartCRC, const void* Buf, size_t BufSize);
// Calculate the crc over the data in the buffer using the CCITT polynomial
// x^16 + x^12 + x^5 + 1

void CRC_CCITT (char B, u16& CRC);
// One byte version of the above

u16 CRC_16 (u16 StartCRC, const void* Buf, size_t BufSize);
// Calculate the crc over the data in the buffer using the polynomial
// x^16 + x^15 + x^2 + 1

void CRC_16 (char B, u16& CRC);
// One byte version of the above

u32 CRC_32 (u32 StartCRC, const void* Buf, size_t BufSize);
// Calculate the crc over the data in the buffer using the polynomial
// x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x+1.

void CRC_32 (char B, u32& CRC);
// One byte version of the above



// End of CRC.H

#endif


