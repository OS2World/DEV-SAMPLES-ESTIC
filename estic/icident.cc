/*****************************************************************************/
/*                                                                           */
/*                                ICIDENT.CC                                 */
/*                                                                           */
/* (C) 1995-96  Ullrich von Bassewitz                                        */
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



#include "progutil.h"

#include "icmsg.h"



/*****************************************************************************/
/*                             Message constants                             */
/*****************************************************************************/



const u16 msISTECXXXX           = MSGBASE_ICIDENT +  0;
const u16 msISTEC1003           = MSGBASE_ICIDENT +  1;
const u16 msISTEC1008           = MSGBASE_ICIDENT +  2;
const u16 msISTEC1016           = MSGBASE_ICIDENT +  3;
const u16 msISTEC1024           = MSGBASE_ICIDENT +  4;
const u16 msISTEC2016           = MSGBASE_ICIDENT +  5;
const u16 msISTEC2024           = MSGBASE_ICIDENT +  6;
const u16 msISTEC2400           = MSGBASE_ICIDENT +  7;
const u16 msISTEC2416           = MSGBASE_ICIDENT +  8;
const u16 msISTEC2424           = MSGBASE_ICIDENT +  9;



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



String GetIstecName (unsigned IstecType)
// Get the name of the istec
{
    // Assign the string number
    unsigned MsgNum;
    switch (IstecType) {
        case 1003:      MsgNum = msISTEC1003;   break;
        case 1008:      MsgNum = msISTEC1008;   break;
        case 1016:      MsgNum = msISTEC1016;   break;
        case 1024:      MsgNum = msISTEC1024;   break;
        case 2016:      MsgNum = msISTEC2016;   break;
        case 2024:      MsgNum = msISTEC2024;   break;
        case 2400:      MsgNum = msISTEC2400;   break;
        case 2416:      MsgNum = msISTEC2416;   break;
        case 2424:      MsgNum = msISTEC2424;   break;
        default:        MsgNum = msISTECXXXX;   break;
    }

    // Load and return the correct name
    return LoadAppMsg (MsgNum);
}



