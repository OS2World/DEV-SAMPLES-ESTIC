/*****************************************************************************/
/*                                                                           */
/*                                  ICCTI.CC                                 */
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



#include "progutil.h"

#include "icmsg.h"
#include "iccti.h"



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



// Message constants
const u16 msCTI_RC_UNKNOWN              = MSGBASE_ICCTI +  0;
const u16 msCTI_RC_ERROR                = MSGBASE_ICCTI +  1;
const u16 msCTI_RC_INVALID_NUMBER       = MSGBASE_ICCTI +  2;
const u16 msCTI_RC_INVALID_SP_STELLE    = MSGBASE_ICCTI +  3;
const u16 msCTI_RC_INVALID_CHANNEL      = MSGBASE_ICCTI +  4;
const u16 msCTI_RC_INVALID_DAY_NIGHT    = MSGBASE_ICCTI +  5;
const u16 msCTI_RC_EEPROM_IN_USE        = MSGBASE_ICCTI +  6;
const u16 msCTI_RC_DAY_NIGHT_CHANGED    = MSGBASE_ICCTI +  7;
const u16 msCTI_RC_DEFAULT_VALUES       = MSGBASE_ICCTI +  8;
const u16 msCTI_RC_DAY_NIGHT_SAME       = MSGBASE_ICCTI +  9;



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



const String& GetCTIErrorDesc (unsigned char RC)
// Return a textual representation of the given error code
{
    unsigned MsgNum;
    switch (RC) {
        case CTI_RC_ERROR:            MsgNum = msCTI_RC_ERROR;            break;
        case CTI_RC_INVALID_NUMBER:   MsgNum = msCTI_RC_INVALID_NUMBER;   break;
        case CTI_RC_INVALID_SP_STELLE:MsgNum = msCTI_RC_INVALID_SP_STELLE;break;
        case CTI_RC_INVALID_CHANNEL:  MsgNum = msCTI_RC_INVALID_CHANNEL;  break;
        case CTI_RC_INVALID_DAY_NIGHT:MsgNum = msCTI_RC_INVALID_DAY_NIGHT;break;
        case CTI_RC_EEPROM_IN_USE:    MsgNum = msCTI_RC_EEPROM_IN_USE;    break;
        case CTI_RC_DAY_NIGHT_CHANGED:MsgNum = msCTI_RC_DAY_NIGHT_CHANGED;break;
        case CTI_RC_DEFAULT_VALUES:   MsgNum = msCTI_RC_DEFAULT_VALUES;   break;
        case CTI_RC_DAY_NIGHT_SAME:   MsgNum = msCTI_RC_DAY_NIGHT_SAME;   break;
        default:                      MsgNum = msCTI_RC_UNKNOWN;          break;
    }

    return LoadAppMsg (MsgNum);
}



String CTIMsgDesc (const unsigned char* Data, unsigned Size)
// Return a string describing the CTI message. This is for debug purposes
// only, the string is hardcoded, not loaded from the resource.
{
    PRECONDITION (Size >= 1 && Data [0] == CTI_START);

    String S = "CTI message";
    return S;
}







