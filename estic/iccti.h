/*****************************************************************************/
/*                                                                           */
/*                                  ICCTI.H                                  */
/*                                                                           */
/* (C) 1996     Ullrich von Bassewitz                                        */
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



// Definitions for CTI messages



#ifndef _ICCTI_H
#define _ICCTI_H



#include "str.h"



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



const unsigned char CTI_START                   = 0xCC;
const unsigned char CTI_STOP                    = 0xFF;

const unsigned char CTI_CONF                    = 0x05;
const unsigned char CTI_QUERY                   = 0x06;
const unsigned char CTI_ERROR                   = 0x07;
const unsigned char CTI_ACK                     = 0x08;

const unsigned char CTI_LOAD_NUMBER             = 0x03;
const unsigned char CTI_ALARM                   = 0x05;
const unsigned char CTI_DAY_NIGHT               = 0x07;
const unsigned char CTI_STORE_NUMBER            = 0x09;

const unsigned char CTI_RC_ERROR                = 0x00; // Generic error
const unsigned char CTI_RC_INVALID_NUMBER       = 0x01;
const unsigned char CTI_RC_INVALID_SP_STELLE    = 0x02;
const unsigned char CTI_RC_INVALID_CHANNEL      = 0x03;
const unsigned char CTI_RC_INVALID_DAY_NIGHT    = 0x04;
const unsigned char CTI_RC_EEPROM_IN_USE        = 0x05;
const unsigned char CTI_RC_DAY_NIGHT_CHANGED    = 0x06;
const unsigned char CTI_RC_DEFAULT_VALUES       = 0x07;
const unsigned char CTI_RC_DAY_NIGHT_SAME       = 0x08;



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



const String& GetCTIErrorDesc (unsigned char RC);
// Return a textual representation of the given error code

String CTIMsgDesc (const unsigned char* Data, unsigned Size);
// Return a string describing the CTI message. This is for debug purposes
// only, the string is hardcoded, not loaded from the resource.



// End of ICCTI.H

#endif

