/*****************************************************************************/
/*                                                                           */
/*                                 STRCVT.CC                                 */
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



#include <stdio.h>

#include "check.h"
#include "strcvt.h"



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



static void IntCvt (u32 Val, unsigned Base, String& S)
{
    unsigned C = Val % Base;
    if (Val /= Base) {
        IntCvt (Val, Base, S);
    }
    S += (C > 9) ? C - 10 + 'A' : C + '0';      // Assumes ASCII
}



String I32Str (i32 Val, unsigned Base)
// Convert an integer into a string
{
    String S;

    // Handle negative values
    if (Val < 0) {
        S += '-';
        Val = -Val;
    }

    // Build the string
    IntCvt (Val, Base, S);

    // Return the result
    return S;
}



String U32Str (u32 Val, unsigned Base)
// Convert an unsigned into a string
{
    String S;

    // Build the string
    IntCvt (Val, Base, S);

    // Return the result
    return S;
}



String FloatStr (double Val, unsigned LDigits, unsigned TDigits)
// Convert a double into a string. There is a difference between this function
// and the converting functions for integers: The result definitely has not
// more than the given width.
{
    char Buf [256];

    // Calculate width
    unsigned Width = TDigits + LDigits;
    if (TDigits > 0) {
        Width++;                // Decimal separator
    }
    CHECK (Width < sizeof (Buf));

    // Convert the float
    sprintf (Buf, "%.*f", (int) TDigits, Val);

    // Create the result string from the buffer
    String S (Buf);

    // Localize the decimal separator
    int SepPos = S.Pos ('.');
    if (SepPos >= 0) {
        S [SepPos] = NLSData.DecSep;
    }

    // Assure the maximum length
    if (S.Len () > Width) {
        S.ForceLen (Width);
    }

    // Return the result
    return S;
}



String TimeStr (u32 TimeInSec)
// Convert a time (given in seconds since midnight) into a string.
{
    // Check the given parameter
    PRECONDITION (TimeInSec <= 23*3600 + 59*60 + 59);

    // Calculate hour/min/sec
    unsigned Sec = TimeInSec % 60;
    TimeInSec /= 60;
    unsigned Min = TimeInSec % 60;
    TimeInSec /= 60;
    unsigned Hour = TimeInSec;

    // Set up string according to local time format
    int PM = 0;
    switch (NLSData.Time) {

        case 0:
            // am/pm
            if (Hour >= 12) {
                Hour -= 12;
                PM = 1;
            }
            return FormatStr ("%02d%c%02d%c%02d %s",
                              Hour, NLSData.TimeSep,
                              Min,  NLSData.TimeSep,
                              Sec,  PM ? "pm" : "am");

        case 1:
            return FormatStr ("%02d%c%02d%c%02d",
                              Hour, NLSData.TimeSep,
                              Min,  NLSData.TimeSep,
                              Sec);

        default:
            FAIL ("Unknown value for NLSData.Time");

    }

    // Never reached
    return String ();
}



