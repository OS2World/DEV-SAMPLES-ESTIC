/*****************************************************************************/
/*                                                                           */
/*                                    STR.H                                  */
/*                                                                           */
/* (C) 1993,94  Ullrich von Bassewitz                                        */
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



#include "machine.h"
#include "object.h"
#include "strmable.h"
#include "stream.h"
#include "string.h"
#include "msg.h"
#include "streamid.h"



// Link in class Msg
LINK (Msg, ID_Msg);


/*****************************************************************************/
/*                                 class Msg                                 */
/*****************************************************************************/



void Msg::Load (Stream &S)
{
    String::Load (S);
    S >> MsgNum;
}




void Msg::Store (Stream &S) const
{
    String::Store (S);
    S << MsgNum;
}





u16 Msg::StreamableID () const
{
    return ID_Msg;
}





Streamable * Msg::Build ()
{
    return new Msg (Empty);
}



Msg & Msg::operator = (const Msg &S)
{
    String::operator = (S);
    MsgNum = S.MsgNum;
    return *this;
}





 
