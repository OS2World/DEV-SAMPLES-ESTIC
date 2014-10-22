/*****************************************************************************/
/*                                                                           */
/*                                 ICLOG.CC                                  */
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



#include <stdio.h>

#include "event.h"
#include "strcvt.h"
#include "datetime.h"
#include "filepath.h"

#include "icevents.h"
#include "devstate.h"
#include "iccli.h"
#include "iclog.h"



/*****************************************************************************/
/*                             class CallLogger                              */
/*****************************************************************************/



class CallLogger: public EventHandler {

    static String ExpandName (const String& Filename, const Time& T, unsigned Dev);
    // Expand the name to a real file name.

    static void LogOutgoing (const String& Filename, const String& Msg,
                             const Time& T, unsigned Dev);
    // Log the message of the outgoing call to one file

    static void LogIncoming (const String& Filename, const String& Msg,
                             const Time& T);
    // Log the message of an incoming call to one file

public:
    virtual void HandleEvent (Event& E);
    // Handle incoming events

};



String CallLogger::ExpandName (const String& Filename, const Time& T, unsigned Dev)
// Expand the name to a real file name.
{
    // Expand private escape sequences
    unsigned I = 0;
    unsigned Len = Filename.Len ();
    String S (Len);
    while (I < Len) {

        // Get the next character from the source
        char C = Filename [I++];

        // Look after '%' only if another char follows
        if (C == '%' && I < Len) {

            C = Filename [I++];
            switch (C) {

                case 'E':
                    // %E - Insert extension number
                    S += U32Str (Dev+21);
                    break;

                default:
                    S += '%';
                    S += C;
                    break;

            }

        } else {

            // Just copy
            S += C;

        }
    }

    return MakeAbsolute (T.DateTimeStr (S));
}



void CallLogger::LogOutgoing (const String& Filename, const String& Msg,
                              const Time& T, unsigned Dev)
// Log the message of the outgoing call to one file
{
    // If the filename is empty, bail out early
    if (Filename.IsEmpty ()) {
        return;
    }

    // Expand the filename.
    String ExpandedName = ExpandName (Filename, T, Dev);

    // Open the file
    FILE* F = fopen (ExpandedName.GetStr (), "a+t");
    if (F == NULL) {
        // Got an error - ignore it
        return;
    }

    // Write the message to the file
    fputs (Msg.GetStr (), F);
    fputc ('\n', F);

    // Close the file
    fclose (F);
}



void CallLogger::LogIncoming (const String& Filename, const String& Msg,
                              const Time& T)
// Log the message of an incoming call to one file
{
    // If the filename is empty, bail out early
    if (Filename.IsEmpty ()) {
        return;
    }

    // Expand the filename.
    String ExpandedName = MakeAbsolute (T.DateTimeStr (Filename));

    // Open the file
    FILE* F = fopen (ExpandedName.GetStr (), "a+t");
    if (F == NULL) {
        // Got an error - ignore it
        return;
    }

    // Write the message to the file
    fputs (Msg.GetStr (), F);
    fputc ('\n', F);

    // Close the file
    fclose (F);
}



void CallLogger::HandleEvent (Event& E)
// Handle incoming events
{
    String Msg (80);
    DevStateInfo* DS;
    CLI* C;

    switch (E.What) {

        case evCallComplete:
            // Call is complete. E.Info.O is the DevStateInfo object
            DS = (DevStateInfo*) E.Info.O;

            // Check if we should log the call
            if (LogZeroCostCalls || DS->CallCharges > 0) {

                // Ok, get the log message and convert it to the output format
                Msg = DS->LogMsg ();
                Msg.OutputCvt ();

                // Log it in all three files
                LogOutgoing (OutgoingLog1, Msg, DS->CallStart, DS->DevNum);
                LogOutgoing (OutgoingLog2, Msg, DS->CallStart, DS->DevNum);
                LogOutgoing (OutgoingLog3, Msg, DS->CallStart, DS->DevNum);

            }
            break;

        case evIncomingCall:
            // Got an incoming call, E.Info.O is the CLI info object
            C = (CLI*) E.Info.O;

            // Build a message
            Msg = C->LogMsg ();

            // Log it to all three files
            LogIncoming (IncomingLog1, Msg, C->T);
            LogIncoming (IncomingLog2, Msg, C->T);
            LogIncoming (IncomingLog3, Msg, C->T);
            break;


    }
}



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



// Names of the logfiles
String OutgoingLog1 = "outgoing.log";
String OutgoingLog2 = "";
String OutgoingLog3 = "";
String IncomingLog1 = "incoming.log";
String IncomingLog2 = "";
String IncomingLog3 = "";

// If true, log calls with a chargecount of zero
int LogZeroCostCalls    = 1;

// Price of a charge unit
double PricePerUnit = 0.12;

// A static copy of a CallLogger object
static CallLogger CL;



