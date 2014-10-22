/*****************************************************************************/
/*                                                                           */
/*                                ISTECMSG.H                                 */
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



// Defines a class that holds an istec message



#ifndef _ISTECMSG_H
#define _ISTECMSG_H



#include "str.h"



/*****************************************************************************/
/*                              class IstecMsg                               */
/*****************************************************************************/



class IstecMsg {
    int ErrCode;

public:
    unsigned Size;
    unsigned char* Data;

    IstecMsg (unsigned MsgSize, const unsigned char* Buf = 0);
    // Create a message

    ~IstecMsg ();
    // Free the data area

    void NewData (unsigned MsgSize, const unsigned char* Buf = 0);
    // Replace the current data by the new stuff. If Buf is not 0, the
    // contents of Buf are copied to the message.

    String AsciiData () const;
    // Return the message converted to a string

    int IsDiagMsg () const;
    // Return true if the message is a diagnostic message

    int IsCLIMsg () const;
    // Return true if the message is a calling line identification message

    int IsCTIMsg () const;
    // Return true if the message is a CTI message

    int IsChargeInfo () const;
    // Return true if the message is a charge info message

    void SetError (int Code);
    // Set the error code

    int GetError () const;
    // Get the error code

    unsigned char& At (unsigned Index);
    // Access the Data member bounds checked.

    const unsigned char& At (unsigned Index) const;
    // Access the Data member bounds checked.
};



inline void IstecMsg::SetError (int Code)
// Set the error code
{
    ErrCode = Code;
}



inline int IstecMsg::GetError () const
// Get the error code
{
    return ErrCode;
}



/*****************************************************************************/
/*                     Other Istec Message related stuff                     */
/*****************************************************************************/



String IstecMsgDesc (const unsigned char* Data);
// Return a string describing the Istec message. This is for debug purposes
// only, the string is hardcoded, not loaded from the resource.

String AsciiData (const unsigned char* Data, unsigned Size);
// Create an ascii string with the hex representation of the message data

int IsDiagMsg (const unsigned char* Data, unsigned Size);
// Return true if the given message is a diagnostic message

int IsCLIMsg (const unsigned char* Data, unsigned Size);
// Return true if the message is a calling line identification message

int IsCTIMsg (const unsigned char* Data, unsigned Size);
// Return true if the message is a CTI message

int IsChargeInfo (const unsigned char* Data, unsigned Size);
// Return true if the message contains the ISTEC charges

unsigned SendMsgSize (const unsigned char* Data, unsigned BufSize);
// The function returns the real size of the given message to send. Since
// the messages given to this function are created by the programmer (that
// is me - hi!), a non determinable message size is a programming error.
// Note: BufSize is the size of the message buffer, the real message that
// is sent maybe smaller than this value.

unsigned RecMsgSize (unsigned char* Data, unsigned Size);
// Calculate the size of a message. First byte is opcode.



// End of ISTECMSG.H

#endif


