/*****************************************************************************/
/*                                                                           */
/*                                 ICDLOG.CC                                 */
/*                                                                           */
/* (C) 1995     Ullrich von Bassewitz                                        */
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



#include "errlog.h"
#include "filepath.h"

#include "icmsg.h"
#include "icerror.h"



/*****************************************************************************/
/*                             Message constants                             */
/*****************************************************************************/



const u16 msOpenError           = MSGBASE_ICDLOG +  0;



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



static ErrLog* Logfile = 0;



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



void DoneDebugLog ()
// Close an open debug log
{
    if (Logfile) {
        delete Logfile;
        Logfile = 0;
    }
}



void InitDebugLog (const String& Filename)
// Initialize (open) the debug log. If the given name is empty, the call is
// ignored. Notify the user in case of errors.
{
    // Ignore an empty file name
    if (Filename.IsEmpty ()) {
        return;
    }

    // Create the logfile
    Logfile = new ErrLog (MakeAbsolute (Filename), 0);

    // Check for errors
    if (Logfile->HasError ()) {

        // Show an error message
        IstecError (msOpenError);

        // Delete the logfile
        DoneDebugLog ();
    }
}



void WriteDebugLog (const String& Msg)
// Log the given message if the debug log is open
{
    if (Logfile) {
        Logfile->Write (Msg);
    }
}



