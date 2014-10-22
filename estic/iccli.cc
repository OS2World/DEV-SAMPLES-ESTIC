/*****************************************************************************/
/*                                                                           */
/*                                    ICCLI.CC                               */
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



// Calling line identification stuff



#include "chartype.h"
#include "national.h"
#include "progutil.h"

#include "icevents.h"
#include "icalias.h"
#include "icac.h"
#include "iccli.h"



/*****************************************************************************/
/*                                 class CLI                                 */
/*****************************************************************************/



String CLI::LogMsg ()
// Return a log message for the CLI
{
    String Msg (80);

    // Convert date and time and pad them to the correct length
    Msg += T.DateStr().Pad (String::Right, 10);
    Msg += "  ";
    Msg += T.TimeStr().Pad (String::Right, 8);
    Msg += "  ";

    // Pad the incoming number
    String Phone = Number;
    Msg += Phone.Pad (String::Right, 20);
    Msg += "  ";

    // Create the additional info. If we have an alias, use it for the
    // additional info. If we don't have an alias, use the prefix info
    String S = Alias;
    if (S.IsEmpty ()) {
        S = AreaCodeInfo;
    }
    Msg += S.Trunc (34);

    // Return the complete line
    return Msg;
}



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



void HandleCLIMsg (const unsigned char* Data, unsigned Size)
// Handle a CLI message
{
    // Do some checks
    PRECONDITION (Size >= 6);

    // Create a new CLI object
    CLI* C = new CLI;

    // Extract the info fields
    C->TypeOfNumber = Data [3] - '0';
    C->PresInd      = Data [4] - '0';

    // If we have a number, handle it
    if (C->PresInd == piPresAllowed) {

        // Copy the number from the given data into the string. To avoid problems
        // with garbage characters, and to handle the FF/CR trailer, drop all
        // control characters.
        for (unsigned I = 5; I < Size; I++) {
            char Digit = (char) Data [I];
            if (!IsCntrl (Digit)) {
                C->Number += Digit;
            }
        }

        // Make a universal number (include the country code if needed)
        String Phone = C->Number;
        if (C->TypeOfNumber == ntNational) {
            // Add a leading dial prefix to the number (for display)
            C->Number.Ins (0, DialPrefix);
        } else if (C->TypeOfNumber == ntInternational) {
            // Add two dial prefixes
            const char Prefix [3] = { DialPrefix, DialPrefix };
            C->Number.Ins (0, Prefix);
        }

        // Get the areacode info from the number
        unsigned AreaCodeLen;
        C->AreaCodeInfo = IstecGetAreaCodeInfo (C->Number, AreaCodeLen);

        // Get the number alias.
        if (AutoReadAliases) {
            ReadAliasFile ();
        }
        C->Alias = GetAlias (C->Number);

        // If we have an areacode info, we can insert a '/' in the correct place.
        if (AreaCodeLen > 0) {
            C->Number.Ins (AreaCodeLen, '/');
        }
    }

    // Create the log message for the incoming call
    String Msg = C->LogMsg ();

    // Post an event with the CLI object as parameter, THIS WILL DELETE IT!
    PostEvent (evIncomingCall, C);

    // Post an event with the log message
    PostEvent (evIncomingLogMsg, new String (Msg));
}



