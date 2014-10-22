/*****************************************************************************/
/*                                                                           */
/*                                  ERRLOG.H                                 */
/*                                                                           */
/* (C) 1993,94  Ullrich von Bassewitz                                        */
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



#ifndef _ERRLOG_H
#define _ERRLOG_H



#include <stdio.h>

#include "str.h"



/*****************************************************************************/
/*                                 Constants                                 */
/*****************************************************************************/



const u16       elCritical      = 0x0001;       // Critical, open close for each message
const u16       elTruncate      = 0x0002;       // Truncate on first open
const u16       elOpen          = 0x0010;       // File is open
const u16       elFileError     = 0x0100;       // Cannot access file



/*****************************************************************************/
/*                               class ErrLog                                */
/*****************************************************************************/



class ErrLog : public Object {

private:
    FILE*       F;
    u16         Flags;
    String      Name;

    virtual void Open ();
    virtual void Close ();

public:
    ErrLog (const String& Filename, u16 aFlags = elCritical);
    ~ErrLog ();

    void Write (const String& Msg);
    // Write a message to the file

    int HasError ();
    // Return a value != 0 if there have been file errors

    virtual String CompleteMsg (const String& Msg);
    // Build a complete message line from the given partial message

};



inline int ErrLog::HasError ()
// Return a value != 0 if there have been file errors
{
    return (Flags & elFileError);
}



// End of ERRLOG.H

#endif
