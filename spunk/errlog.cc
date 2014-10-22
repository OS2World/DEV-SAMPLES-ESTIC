/*****************************************************************************/
/*                                                                           */
/*                                  ERRLOG.CC                                */
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



#include <stdio.h>
#include <time.h>

#include "errlog.h"



/*****************************************************************************/
/*                               class ErrLog                                */
/*****************************************************************************/



ErrLog::ErrLog (const String& Filename, u16 aFlags) :
    Flags (u16 (aFlags & elCritical)),
    Name (Filename)
{
    // If the elTruncate flag is set, remove the file without evaluating
    // the error code (the file may not exist).
    if (aFlags & elTruncate) {
        (void) remove (Name.GetStr ());
    }

    // Try to open the log file, setting the error bit
    Open ();

    // If the file is critical close it again
    if (Flags & elCritical) {
        Close ();
    }

}



ErrLog::~ErrLog ()
{
    Close ();
}



void ErrLog::Open ()
// Open the file
{
    if (HasError ()) {
        return;
    }

    if ((Flags & elOpen) == 0) {
        // Open the file
        F = fopen (Name.GetStr (), "at");
        if (F == NULL) {
            // Remember error condition
            Flags |= elFileError;
        } else {
            // Set buffering mode to line buffered
            setvbuf (F, NULL, _IOLBF, BUFSIZ);

            // Mark file as open
            Flags |= elOpen;
        }
    }
}



void ErrLog::Close ()
// Close the log file
{
    if (HasError ()) {
        return;
    }

    if (Flags & elOpen) {
        if (fclose (F) != 0) {
            Flags |= elFileError;
            return;
        }
        Flags &= ~elOpen;
    }
}



void ErrLog::Write (const String& Msg)
// Write a message to the file
{
    // Open the file if needed
    if (Flags & elCritical) {
        Open ();
    }

    // Bail out if the file cannot be accessed
    if (Flags & elFileError) {
        return;
    }

    // Create the complete message and write it to the file
    String S (CompleteMsg (Msg));

    fputs (S.GetStr (), F);

    // If the log file is critical, close the file
    if (Flags & elCritical) {
        Close ();
    }
}



String ErrLog::CompleteMsg (const String& Msg)
// Build a complete message line from the given partial message
{
    time_t timer = time (NULL);
    char Buf [30];
    strcpy (Buf, asctime (localtime (&timer)));
    Buf [24] = '\0';
    return FormatStr ("[%s]  ", Buf) + Msg + '\n';
}

