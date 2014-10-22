/*****************************************************************************/
/*                                                                           */
/*                                  GETNLS.CC                                */
/*                                                                           */
/* (C) 1995     Ullrich von Bassewitz                                        */
/*              Zwehrenbuehlstrasse 33                                       */
/*              D-72070 Tuebingen                                            */
/* EMail:       uz@ibb.schwaben.de                                           */
/*                                                                           */
/*****************************************************************************/



// $Id$
//
// $Log$
//
//



// This is a helper program to create the national language files...



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define INCL_DOSNLS
#include <os2.h>

#include "machine.h"
#include "nlsinfo.h"
#include "national.h"
#include "check.h"



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



static void Error (const char* S, u32 RC, u32 Country)
{
    fprintf (stderr, "Error reading %s, code = %d, country = %d\n",
             S, RC, Country);
    exit (1);
}



static void ReadCountry (u32 Country, u32 CodePage = 437)
// Sets up the data for the NLS. This function has to be called before any call
// to another function in this module!
{
    _NLSInfo S;

    // Country code for the current country
    COUNTRYCODE CountryCode;
    CountryCode.country  = Country;
    CountryCode.codepage = CodePage;

    COUNTRYINFO Info;
    ULONG DataLength;
    ULONG RC;

    // Get country information
    RC = DosQueryCtryInfo (sizeof (Info), &CountryCode, &Info, &DataLength);
    if (RC != 0) {
        fprintf (stderr, "Error reading DosQueryCtryInfo, code = %d, country = %d\n",
                 RC, Country);
        return;
    }

    // Transfer data
    S.Data.Date         = Info.fsDateFmt;
    S.Data.Time         = Info.fsTimeFmt;
    S.Data.Curr         = Info.fsCurrencyFmt;
    strcpy (S.Data.CurrStr, Info.szCurrency);
    S.Data.ThSep        = Info.szThousandsSeparator [0];
    S.Data.DecSep       = Info.szDecimal [0];
    S.Data.DateSep      = Info.szDateSeparator [0];
    S.Data.TimeSep      = Info.szTimeSeparator [0];
    S.Data.DataSep      = Info.szDataSeparator [0];

    unsigned I;
    for (I = 0; I < 256; I++) {
        S.UpCaseMap [I] = I;
    }

    // Get the current case map
    DosMapCase (sizeof (S.UpCaseMap), &CountryCode, S.UpCaseMap);
    if (RC != 0) {
        Error ("DosMapCase", RC, Country);
    }

    // Now create the lowercase table from the uppercase table
    memset (S.LoCaseMap, 0, 256);
    for (I = 0; I < 256; I++) {
        int Upper = S.UpCaseMap [I];
        int Lower = I;
        if (Upper != Lower) {
            if (S.LoCaseMap [Upper] == 0) {
                S.LoCaseMap [Upper] = Lower;
            }
        }
    }
    for (I = 0; I < 256; I++) {
        if (S.LoCaseMap [I] == 0) {
            S.LoCaseMap [I] = I;
        }
    }

    // Retrieve the collating sequence table
    DosQueryCollate (sizeof (S.CollMap), &CountryCode, S.CollMap, &DataLength);
    if (RC != 0) {
        Error ("DosQueryCollate", RC, Country);
    }

    char Name [128];
    sprintf (Name, "%03d.dat", Country);
    FILE* F = fopen (Name, "wb");
    fwrite (&S, sizeof (S), 1, F);
    fclose (F);
}





int main ()
{
    ReadCountry ( 99, 437);
    ReadCountry ( 61, 437);
    ReadCountry ( 32, 437);
    ReadCountry (  2, 437);
    ReadCountry ( 42, 437);
    ReadCountry ( 45, 437);
    ReadCountry (358, 437);
    ReadCountry ( 33, 437);
    ReadCountry ( 49, 437);
    ReadCountry ( 36, 437);
    ReadCountry (354, 437);
    ReadCountry ( 39, 437);
    ReadCountry (  3, 437);
    ReadCountry ( 31, 437);
    ReadCountry ( 47, 437);
    ReadCountry ( 48, 437);
    ReadCountry (351, 437);
    ReadCountry ( 34, 437);
    ReadCountry ( 46, 437);
    ReadCountry ( 41, 437);
    ReadCountry ( 90, 437);
    ReadCountry ( 44, 437);
    ReadCountry (  1, 437);
    ReadCountry ( 38, 437);
    return 0;
}
