/*****************************************************************************/
/*                                                                           */
/*                                 ICCRON.CC                                 */
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



#include "bitset.h"
#include "chartype.h"
#include "coll.h"
#include "str.h"
#include "strcvt.h"
#include "strparse.h"
#include "stdmsg.h"
#include "progutil.h"

#include "icmsg.h"
#include "icevents.h"
#include "icdlog.h"
#include "icerror.h"
#include "iccom.h"
#include "iccprint.h"
#include "icfile.h"
#include "iccron.h"



/*****************************************************************************/
/*                             Message Constants                             */
/*****************************************************************************/



const u16 msOpenError                   = MSGBASE_ICCRON +  0;
const u16 msSyntaxError                 = MSGBASE_ICCRON +  1;
const u16 msUnknownCommand              = MSGBASE_ICCRON +  2;
const u16 msArgError                    = MSGBASE_ICCRON +  3;
const u16 msCronJobProcessing           = MSGBASE_ICCRON +  4;



/*****************************************************************************/
/*                      Explicit template instantiation                      */
/*****************************************************************************/



#ifdef EXPLICIT_TEMPLATES
template class Collection<class CronEvent>;
#endif



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



// Name of the cron file, default is the empty string
String CronFile = "";



// Names of internal commands and there IDs
enum {
    icPrintCharges,
    icClearCharges,
    icLoadConfig,
    icRing,
    icReadCronFile,
    icCount
};

static const char* InternalCommands [icCount] = {
    "PRINTCHARGES",
    "CLEARCHARGES",
    "LOADCONFIG",
    "RING",
    "READCRONFILE"
};



/*****************************************************************************/
/*                              class CronEvent                              */
/*****************************************************************************/



class CronEvent: public Streamable {

public:
    BitSet      Minute;
    BitSet      Hour;
    BitSet      WeekDay;
    BitSet      MonthDay;
    BitSet      Month;
    int         Cmd;

    // Arguments
    String      Filename;               // Filename argument
    unsigned    Device;                 // Device argument
    unsigned    Duration;               // Duration argument


    CronEvent (int CmdID);
    // Constructor

};



CronEvent::CronEvent (int CmdID):
    Minute (60),
    Hour (24),
    WeekDay (7),
    MonthDay (31, 1),
    Month (12, 1),
    Cmd (CmdID)
{
}



/*****************************************************************************/
/*                            class CronEventColl                            */
/*****************************************************************************/



class CronEventColl: public Collection<CronEvent> {

public:
    CronEventColl ();
    // Constructor

};



CronEventColl::CronEventColl ():
    Collection <CronEvent> (25, 25, 1)
{
}



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



static CronEventColl CronEvents;



/*****************************************************************************/
/*                             Helper functions                              */
/*****************************************************************************/



static int FindCmdID (String Cmd)
// Search for the given command in the command table and return the ID. If
// the command is not found, -1 is returned.
{
    // Make the command upper case
    Cmd.ToUpper ();

    // This is a linear search, since the count of commands is very small
    for (int I = 0; I < icCount; I++) {
        if (strcmp (InternalCommands [I], Cmd.GetStr ()) == 0) {
            // Found
            return I;
        }
    }

    // Not found
    return -1;
}



static int InBounds (const BitSet& B, i32 Num)
{
    return (Num >= B.Low () && Num <= B.High ());
}



static int ParseRingArgs (CronEvent& E, const String& Args)
// Parse the numeric arguments for the RING command
{
    // Create a string parser
    StringParser SP (Args, StringParser::SkipWS);

    // Read the device
    u32 Device;
    if (SP.GetU32 (Device) != 0 || Device < 21 || Device > 99) {
        // Device parameter invalid
        return 0;
    }

    // Read the duration
    u32 Duration;
    if (SP.GetU32 (Duration) != 0 || Duration == 0 || Duration > 5*60) {
        // Device parameter invalid
        return 0;
    }

    // Assign the values
    E.Device   = Device;
    E.Duration = Duration;

    // Success
    return 1;
}



static int ParseTimeExpr (BitSet& B, const String& S)
// Parse a cron time expression in the string S and set the time in the
// bitset B. It is assumed that S is non empty.
{
    PRECONDITION (!S.IsEmpty ());

    // Set up a string parser
    StringParser SP (S, 0);

    // Read a number
    u32 Start;
    if (SP.GetU32 (Start) != 0 || !InBounds (B, Start)) {
        return 0;
    }

    // Check if end of string is reached, if yes, set the bit and bail out
    if (SP.EOS ()) {
        B += (int) Start;
        return 1;
    }

    // If the string did not end, we expect a range expression
    if (S [SP.GetPos ()] != '-') {
        // Some error
        return 0;
    }
    SP.SetPos (SP.GetPos () + 1);

    // Read the end of the range
    u32 End;
    if (SP.GetU32 (End) != 0 || !InBounds (B, End)) {
        return 0;
    }

    // End must be greater than Start
    if (End < Start) {
        return 0;
    }

    // Maybe we have an additional space operator
    u32 Space = 1;
    if (!SP.EOS ()) {

        if (S [SP.GetPos ()] != '/') {
            // Some error
            return 0;
        }
        SP.SetPos (SP.GetPos () + 1);

        // Read the space number
        if (SP.GetU32 (Space) != 0) {
            return 0;
        }

        // String must be empty now
        if (!SP.EOS ()) {
            return 0;
        }
    }

    // Now insert the range into the bitset
    while (Start <= End) {
        B += (int) Start;
        Start += Space;
    }

    // Done
    return 1;
}



static int ParseTime (BitSet& B, const String& S)
// Parse a cron time expression in the string S and set the time in the
// bitset B. It is assumed that S is non empty.
{
    PRECONDITION (!S.IsEmpty ());

    // Handle non-digit expression '*'
    if (S == "*") {
        // '* Means: At each full X
        B.SetAll ();
        return 1;
    }

    // Set up a string parser
    StringParser SP (S, 0);

    // Read a list of tokens separated by ',' and evaluate each of those
    // tokens
    String Tok;
    CharSet Separator = ",";
    while (SP.GetToken (Tok, Separator) == 0) {
        // Parse the token
        if (ParseTimeExpr (B, Tok) == 0) {
            // Parse error
            return 0;
        }
        // Skip the token separators
        SP.Skip (Separator);
    }

    // String must be empty now
    if (!SP.EOS ()) {
        return 0;
    }

    // Done with success
    return 1;
}



inline String ExpandName (const String& Filename, const Time& T)
// Expand the name to a real file name.
{
    return T.DateTimeStr (Filename);
}



/*****************************************************************************/
/*                              Cron functions                               */
/*****************************************************************************/



static void CronPrintCharges (const Time& T, const String& Filename)
{
    // Expand the filename
    String Name = ExpandName (Filename, T);

    // Print the charges
    String Msg = PrintCharges (Name);
    if (Msg.IsEmpty ()) {
        // No error
        Msg = "Done";
    }

    // Log the result
    WriteDebugLog (FormatStr ("CRON PrintCharges (%s): %s",
                              Name.GetStr (),
                              Msg.GetStr()));
}



static void CronClearCharges ()
{
    // Create an empty charge object
    IstecCharges Charges;

    // Send the charges
    IstecPutCharges (Charges);

    // Log the action to the debug log
    WriteDebugLog ("CRON ClearCharges (): Done");
}



static void CronLoadConfig (const Time& T, const String& Filename)
{
    // Expand the filename
    String Name = ExpandName (Filename, T);

    // Try to load the config file with the given name
    IstecConfig Config;
    String Msg = IstecLoadFile (Name, Config);
    if (Msg.IsEmpty ()) {
        // No error. Send the stuff to the istec
        int Res = IstecPutConfig (Config);

        // Make an error message from the return code
        if (Res == ieDone) {
            Msg = "Done";
        } else {
            Msg = FormatStr ("Error #%d", Res);
        }
    }

    // Log the result
    WriteDebugLog (FormatStr ("CRON LoadConfig (%s): %s",
                              Name.GetStr (),
                              Msg.GetStr()));
}



static void CronRing (unsigned Device, unsigned Duration)
{
    // Make one long from the device and duration
    unsigned long Parm = ((unsigned long) Duration) * 256 + Device;

    // Posts an event and hope someone will notice :-)
    PostEvent (evForcedRing, Parm);

    // Log the succes of the action
    WriteDebugLog (FormatStr ("CRON Ring (%d, %d): Done", Device, Duration));
}



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



void HandleCronEvent (const Time& T)
// Execute all events that match the given time
{
    // Lock the function against recursive calls that may happen if the
    // execution of one command lasts too long
    static int Running = 0;
    if (Running) {
        return;
    }
    Running = 1;

    // Remember if we should re-read the cron file
    int ReadFile = 0;

    // Convert the given time to separate values
    unsigned Sec, Minute, Hour, WeekDay, MonthDay, Month, Year;
    T.GetDateTime (Year, Month, MonthDay, WeekDay, Hour, Minute, Sec);

    // A window notifying the user
    Window* Win = 0;

    // Loop through all events, checking if there is a match
    for (int I = 0; I < CronEvents.GetCount (); I++) {

        // Get the specific event
        CronEvent* E = CronEvents [I];

        // Check for a match, continue if there is none
        if (E->Minute [Minute]     == 0 || E->Hour [Hour]   == 0 ||
            E->MonthDay [MonthDay] == 0 || E->Month [Month] == 0 ||
            E->WeekDay [WeekDay]   == 0) {

            // No match, continue
            continue;
        }

        // If this is the first match, pop up a window, notifying the user
        if (Win == 0) {
            Win = MsgWindow (LoadAppMsg (msCronJobProcessing), "", paCyan);
        }

        // Execute the function
        switch (E->Cmd) {

            case icPrintCharges:
                CronPrintCharges (T, E->Filename);
                break;

            case icClearCharges:
                CronClearCharges ();
                break;

            case icLoadConfig:
                CronLoadConfig (T, E->Filename);
                break;

            case icRing:
                CronRing (E->Device, E->Duration);
                break;

            case icReadCronFile:
                ReadFile = 1;
                break;

            default:
                FAIL ("HandleCronEvent: Unknown event code");

        }
    }

    // If we must reread the cron file, do it
    if (ReadFile) {
        ReadCronFile ();
        WriteDebugLog ("CRON ReadCronFile (): Done");
    }

    // If we have a window open, close it
    delete Win;

    // Unlock the function
    Running = 0;
}



void ReadCronFile ()
// Delete all existing cron events and reread the cron file. The function
// does nothing if there is no cron file defined.
{
    // Bail out if there is no valid cron file name
    if (CronFile.IsEmpty ()) {
        return;
    }

    // Delete all existing aliases
    CronEvents.DeleteAll ();

    // Try to open the file
    FILE* F = fopen (CronFile.GetStr (), "rt");
    if (F == NULL) {
        // OOPS, file open error
        IstecError (msOpenError);
        return;
    }

    // Ok, file is open now, read it
    unsigned LineCount = 0;
    char Buf [512];
    while (fgets (Buf, sizeof (Buf), F) != NULL) {

        // Got a new line
        LineCount++;

        // Put the line into a string and convert it to the internally used
        // character set
        String S (Buf);
        S.InputCvt ();

        // Delete the trailing newline if any
        int Len = S.Len ();
        if (Len > 0 && S [Len-1] == '\n') {
            Len--;
            S.Trunc (Len);
        }

        // Ignore empty and comment lines
        if (S.IsEmpty () || S [0] == ';') {
            continue;
        }

        // Set up a string parser for the string
        StringParser SP (S, StringParser::SkipWS);

        // Extract the tokens in the order minute, hour, day, month, weekday
        String Minute, Hour, MonthDay, Month, WeekDay, Command;
        if (SP.GetToken (Minute)        != 0 ||
            SP.GetToken (Hour)          != 0 ||
            SP.GetToken (MonthDay)      != 0 ||
            SP.GetToken (Month)         != 0 ||
            SP.GetToken (WeekDay)       != 0 ||
            SP.GetToken (Command)       != 0) {
            // Error
            ErrorMsg (FormatStr (LoadAppMsg (msSyntaxError).GetStr (), LineCount));
            continue;
        }
        SP.SkipWhite ();

        // The rest of the line are arguments for the command
        String Args = S.Cut (SP.GetPos (), S.Len () - SP.GetPos ());

        // Search for the command in the internal command table
        int CmdID = FindCmdID (Command);

        // Create a new cron event
        CronEvent* E = new CronEvent (CmdID);

        // Parse the commands for the command
        switch (CmdID) {

            case icPrintCharges:
            case icLoadConfig:
                // Commands with a filename argument. Remove leading and
                // trailing spaces
                Args.Remove (" \t", rmLeading | rmTrailing);
                if (Args.IsEmpty ()) {
                    ErrorMsg (FormatStr (LoadAppMsg (msArgError).GetStr (), LineCount));
                    delete E;
                    continue;
                }
                E->Filename = Args;
                break;

            case icClearCharges:
            case icReadCronFile:
                // Commands with no arguments
                break;

            case icRing:
                // Two numeric arguments
                if (ParseRingArgs (*E, Args) == 0) {
                    ErrorMsg (FormatStr (LoadAppMsg (msArgError).GetStr (), LineCount));
                    delete E;
                    continue;
                }
                break;


            case -1:
                // Unknown command
                ErrorMsg (FormatStr (LoadAppMsg (msUnknownCommand).GetStr (), LineCount));
                delete E;
                continue;

        }

        // Parse the time items and set the time bitsets
        if (ParseTime (E->Minute,   Minute)     == 0 ||
            ParseTime (E->Hour,     Hour)       == 0 ||
            ParseTime (E->MonthDay, MonthDay)   == 0 ||
            ParseTime (E->Month,    Month)      == 0 ||
            ParseTime (E->WeekDay,  WeekDay)    == 0) {
            ErrorMsg (FormatStr (LoadAppMsg (msSyntaxError).GetStr (), LineCount));
            delete E;
            continue;
        }

        // Insert the cron event into the collection
        CronEvents.Insert (E);

    }

    // Close the file
    fclose (F);
}



