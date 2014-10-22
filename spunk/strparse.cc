/*****************************************************************************/
/*                                                                           */
/*                                STRPARSE.CC                                */
/*                                                                           */
/* (C) 1993-96  Ullrich von Bassewitz                                        */
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



#include <math.h>

#include "chartype.h"
#include "progutil.h"
#include "msgid.h"
#include "national.h"
#include "charset.h"
#include "strparse.h"



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



static CharSet GetValidChars (int Base)
{
    CharSet S;

    do {
        if (Base >= 10) {
            S += Base - 10 + 'A';
            S += Base - 10 + 'a';
        } else {
            S += Base + '0';
        }
        Base--;
    } while (Base >= 0);

    return S;
}



/*****************************************************************************/
/*                            class StringParser                             */
/*****************************************************************************/



StringParser::StringParser (const String& X, u16 ParseFlags):
    P (0),
    S (X),
    Flags (ParseFlags),
    WhiteSpace (::WhiteSpace)
{
}



void StringParser::SetPos (int NewPos)
// Set a new position for parsing
{
    u16 Len = S.Len ();
    P = (NewPos > Len) ? Len : NewPos;
}



void StringParser::Skip (const String& Chars)
// Skip any chars contained in Chars
{
    Skip (CharSet (Chars));
}



void StringParser::Skip (const CharSet& Chars)
// Skip any chars contained in Chars
{
    char C;
    while ((C = S [P]) != '\0' && Chars [C] != 0) {
        P++;
    }
}



int StringParser::AllUsed ()
// Return true if UseAll is not set or if EOS is reached
{
    return (Flags & UseAll) ? S [P] == '\0' : 1;
}



int StringParser::Num (char C)
// Calculate the numeric value of the digit C (assumes ASCII!)
{
    // C must be a digit or letter
    PRECONDITION (IsAlNum (C));

    if (IsDigit (C)) {
        return (C & 0x0F);
    } else {
        return (NLSUpCase (C) - 'A' + 10);
    }
}



const String& StringParser::GetMsg (unsigned Res)
// Return an apropriate error message for the result Res (!= 0)
{
    PRECONDITION (Res > 0 && Res <= prMaxError);
    return LoadMsg (MSGBASE_STRPARSE - 1 + Res);
}



unsigned StringParser::GetI32 (i32& Val, unsigned Base)
{
    // Check for a correct base value
    PRECONDITION (Base > 1 && Base <= 16);

    // save old position in case of errors
    int SavePos = P;

    // Skip white space if needed
    if (Flags & SkipWS) {
        SkipWhite ();
    }

    // Check for a minus sign
    int Sign = 1;
    if (S [P] == '-') {
        P++;
        Sign = -1;
    } else {
        // If pascal or C style hex constants are allowed, check for that.
        // Hex values are _not_ allowed if there is a preceeding minus sign.
        if (Base == 10 && (Flags & PascalHex) != 0) {
            if (S [P] == '$') {
                P++;
                Base = 16;
            }
        }
        if (Base == 10 && (Flags & CHex) != 0) {
            if (S [P] == '0' && S [P+1] == 'x') {
                P += 2;
                Base = 16;
            }
        }
        if (Base == 10 && (Flags & COct) != 0) {
            if (S [P] == '0') {
                P++;
                Base = 8;
            }
        }
    }

    // Get the valid chars according to base
    CharSet Chars = GetValidChars (Base);

    // Calculate the maximum value
    i32 MaxVal = 0x7FFFFFFF / Base;

    // Next char must be a digit
    if (Chars [S [P]] == 0) {
        P = SavePos;
        return prSyntax;
    }

    // Read the number
    Val = 0;
    while (Chars [S [P]] != 0) {
        if (Val > MaxVal) {
            // Value is getting to large
            P = SavePos;
            return prTooLarge;
        }
        Val = Val * Base + Num (S [P]);
        P++;
    }

    // make the value signed
    Val *= Sign;

    // Check if all chars are read
    if (AllUsed ()) {
        return 0;
    } else {
        P = SavePos;
        return prSyntax;
    }
}



unsigned StringParser::GetU32 (u32& Val, unsigned Base)
{
    // Check for a correct base value
    PRECONDITION (Base > 1 && Base <= 16);

    // save old position in case of errors
    int SavePos = P;

    // Skip white space if needed
    if (Flags & SkipWS) {
        SkipWhite ();
    }

    // If pascal or C style hex constants are allowed, check for that.
    if (Base == 10 && (Flags & PascalHex) != 0) {
        if (S [P] == '$') {
            P++;
            Base = 16;
        }
    }
    if (Base == 10 && (Flags & CHex) != 0) {
        if (S [P] == '0' && S [P+1] == 'x') {
            P += 2;
            Base = 16;
        }
    }
    if (Base == 10 && (Flags & COct) != 0) {
        if (S [P] == '0') {
            P++;
            Base = 8;
        }
    }

    // Get the valid chars according to base
    CharSet Chars = GetValidChars (Base);

    // Calculate the maximum value
    u32 MaxVal = 0xFFFFFFFF / Base;

    // Next char must be a digit
    if (Chars [S [P]] == 0) {
        return prSyntax;
    }

    Val = 0;
    while (Chars [S [P]] != 0) {
        if (Val > MaxVal) {
            // Value is getting to large
            P = SavePos;
            return prTooLarge;
        }
        Val = Val * Base + Num (S [P]);
        P++;
    }

    // Check if all chars are read
    if (AllUsed ()) {
        return 0;
    } else {
        P = SavePos;
        return prSyntax;
    }
}



unsigned StringParser::GetFloat (double& Val)
{
    Val = 0;

    // save old position in case of errors
    int SavePos = P;

    // Skip white space if needed
    if (Flags & SkipWS) {
        SkipWhite ();
    }

    // Check for leading minus
    int Sign = 1;
    switch (S [P]) {

        case '-':
            Sign = -1;
            P++;
            break;

        case '+':
            P++;
            break;

    }

    // Maybe there is a leading decimal separator
    int FracCnt;
    if (S [P] == NLSData.DecSep) {
        P++;
        FracCnt = 0;
    } else {
        FracCnt = -1;
    }

    // Next character must be a number
    if (!IsDigit (S [P])) {
        P = SavePos;
        return prSyntax;
    }

    while (1) {
        char C = S [P];
        if (IsDigit (C)) {
            Val = Val * 10 + Num (C);
            if (FracCnt >= 0) {
                FracCnt++;
            }
        } else if (FracCnt < 0 &&
                   (C == NLSData.DecSep || ((Flags & AllowDP) && C == '.'))) {
            FracCnt++;
        } else {
            break;
        }
        P++;
    }

    // Correct for the fraction and the sign
    while (FracCnt > 0) {
        Val /= 10;
        FracCnt--;
    }
    Val *= Sign;

    // Check if all chars are read
    if (AllUsed ()) {
        return 0;
    } else {
        P = SavePos;
        return prSyntax;
    }
}



unsigned StringParser::GetToken (String& Tok, const CharSet& Chars)
// Return a whitespace separated token from the string
{
    // Clear the result string
    Tok.Clear ();

    // Remember the current position
    int SavePos = P;

    // Skip white space if needed
    if (Flags & SkipWS) {
        SkipWhite ();
    }

    // If the string is empty now, we have a syntax error
    if (S [P] == '\0') {
        P = SavePos;
        return prSyntax;
    }

    // Read characters until end of token is reached
    char C;
    while ((C = S [P]) != '\0' && Chars [C] == 0) {
        // Ok, end of string not reached and no whitespace. Add it.
        Tok += C;
        P++;
    }

    // Ok, done
    return 0;
}



unsigned StringParser::GetString (String& Str)
// Return a string enclosed in double quotes
{
    // Clear the result string
    Str.Clear ();

    // save old position in case of errors
    int SavePos = P;

    // Skip white space if needed
    if (Flags & SkipWS) {
        SkipWhite ();
    }

    // Next char must be a double quote
    if (S [P] != '"') {
        P = SavePos;
        return prSyntax;
    }
    P++;

    // Read characters until end of string is reached
    char C;
    while ((C = S [P]) != '\0' && C != '"') {
        Str += C;
        P++;
    }

    // Check if the ending double quote is missing else skip it
    if (C == '\0') {
        P = SavePos;
        return prSyntax;
    }
    P++;

    // Ok, done
    return 0;
}



unsigned StringParser::GetTime (Time& Val)
{
    // save old position in case of errors
    int SavePos = P;

    // Skip white space if needed, then reset the skip and UseAll flags
    u16 SaveFlags = Flags;
    if (Flags & SkipWS) {
        SkipWhite ();
    }
    Flags &= ~(SkipWS | UseAll);

    // State machine
    u32 Hour, Min, Sec;
    unsigned Res = 0;
    enum { stStart, stSec, stMin, stHour, stError, stDone } State = stStart;
    while (State != stDone) {

        switch (State) {

            case stStart:
                State = stHour;
                break;

            case stHour:
                Res = GetU32 (Hour);
                if (Res != 0) {
                    State = stError;
                } else if (Hour > 23) {
                    Res = prTooLarge;
                    State = stError;
                } else if (S [P] != NLSData.TimeSep) {
                    Res = prSyntax;
                    State = stError;
                } else {
                    P++;
                    State = stMin;
                }
                break;

            case stMin:
                Res = GetU32 (Min);
                if (Res != 0) {
                    State = stError;
                } else if (Min > 59) {
                    Res = prTooLarge;
                    State = stError;
                } else if (S [P] != NLSData.TimeSep) {
                    Sec = 0;
                    State = stDone;
                } else {
                    P++;
                    State = stSec;
                }
                break;

            case stSec:
                Res = GetU32 (Sec);
                if (Res != 0) {
                    State = stError;
                } else if (Sec > 59) {
                    Res = prTooLarge;
                    State = stError;
                } else {
                    State = stDone;
                }
                break;

            case stError:
                P = SavePos;
                State = stDone;
                break;

            default:
                FAIL ("Illegal state machine value in StringParser::GetTime");

        }       // switch

    }       // while

    // Restore original flags
    Flags = SaveFlags;

    // Set result
    if (Res == 0) {
        // Check if all chars are read
        if (AllUsed ()) {
            Val.SetTime (Hour, Min, Sec);
            return 0;
        } else {
            return prSyntax;
        }
    } else {
        return Res;
    }
}



unsigned StringParser::GetDate (Time& Val)
{
    // save old position in case of errors
    int SavePos = P;

    // Skip white space if needed, then reset the skip and UseAll flags
    u16 SaveFlags = Flags;
    if (Flags & SkipWS) {
        SkipWhite ();
    }
    Flags &= ~(SkipWS | UseAll);

    // State machine
    u32 Year, Month, Day;
    unsigned Res = 0;
    enum { stStart, stDay, stDaySep, stMonth,
           stMonthSep, stYear, stYearSep, stError, stDone
         } State = stStart;
    while (State != stDone) {

        switch (State) {

            case stStart:
                switch (NLSData.Date) {
                    case 0:     State = stMonth;        break;
                    case 1:     State = stDay;          break;
                    case 2:     State = stYear;         break;
                }
                break;

            case stYearSep:
                if (S [P] != NLSData.DateSep) {
                    Res = prSyntax;
                    State = stError;
                } else {
                    P++;
                    State = stYear;
                }
                break;

            case stYear:
                Res = GetU32 (Year);
                if (Res != 0) {
                    State = stError;
                } else {
                    if (Year >= 70 && Year < 100) {
                        Year += 1900;
                    } else if (Year <= 30) {
                        Year += 2000;
                    }
                    if (Year < 1970) {
                        Res = prTooSmall;
                        State = stError;
                    } else if (Year >= 2100) {
                        Res = prTooLarge;
                        State = stError;
                    } else {
                        switch (NLSData.Date) {
                            case 0:     State = stDone;         break;
                            case 1:     State = stDone;         break;
                            case 2:     State = stMonthSep;     break;
                        }
                    }
                }
                break;

            case stMonthSep:
                if (S [P] != NLSData.DateSep) {
                    Res = prSyntax;
                    State = stError;
                } else {
                    P++;
                    State = stMonth;
                }
                break;

            case stMonth:
                Res = GetU32 (Month);
                if (Res != 0) {
                    State = stError;
                } else if (Month > 12) {
                    Res = prTooLarge;
                    State = stError;
                } else if (Month == 0) {
                    Res = prTooSmall;
                    State = stError;
                } else {
                    switch (NLSData.Date) {
                        case 0: State = stDaySep;  break;
                        case 1: State = stYearSep; break;
                        case 2: State = stDaySep;  break;
                    }
                }
                break;

            case stDaySep:
                if (S [P] != NLSData.DateSep) {
                    Res = prSyntax;
                    State = stError;
                } else {
                    P++;
                    State = stDay;
                }
                break;

            case stDay:
                Res = GetU32 (Day);
                if (Res != 0) {
                    State = stError;
                } else if (Day > 31) {
                    Res = prTooLarge;
                    State = stError;
                } else if (Day == 0) {
                    Res = prTooSmall;
                    State = stError;
                } else {
                    switch (NLSData.Date) {
                        case 0:     State = stYearSep;      break;
                        case 1:     State = stMonthSep;     break;
                        case 2:     State = stDone;         break;
                    }
                }
                break;

            case stError:
                P = SavePos;
                State = stDone;
                break;

            default:
                FAIL ("Illegal state machine value in StringParser::GetDate");

        }       // switch

    }       // while

    // Restore original flags
    Flags = SaveFlags;

    // Set result
    if (Res == 0) {
        // Check if all chars are read
        if (AllUsed ()) {
            Val.SetDate (Year, Month, Day);
            return 0;
        } else {
            return prSyntax;
        }
    } else {
        return Res;
    }
}



