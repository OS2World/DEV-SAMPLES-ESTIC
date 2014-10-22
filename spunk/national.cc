/*****************************************************************************/
/*                                                                           */
/*                                NATIONAL.CC                                */
/*                                                                           */
/* (C) 1993-95  Ullrich von Bassewitz                                        */
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



#include <string.h>

#include "check.h"
#include "resource.h"
#include "cont.h"
#include "progutil.h"
#include "nlsinfo.h"
#include "national.h"



/*****************************************************************************/
/*                              Types and data                               */
/*****************************************************************************/



// Data structure used
static _NLSData NLSD = {
    0,                          // m/d/y
    0,                          // 12 hour format
    1,                          // Currency style: "$10"
    "$",                        // Currency string
    ',',                        // Thousands separator
    '.',                        // Decimal separator
    '/',                        // Date separator
    '.',                        // Time separator
    ';'                         // Data separator
};



// The default country and language. This settings are used if no other
// information about language/country is available and must be set _before_
// initialization of the application object. Default is USA and US english.
unsigned DefaultCountry         = 1;    // USA
unsigned DefaultLanguage        = laEnglish;

// An external reference to this object declared as const
const _NLSData& NLSData = NLSD;

// Language and country code
static unsigned Country  = 1;           // USA
static unsigned Language = laEnglish;

// External const references to the language and country codes
const unsigned& NLSCountry  = Country;
const unsigned& NLSLanguage = Language;



// Translation table from a char to it's uppercase translation. This table
// assumes strict ASCII.
static _NLSTransTable UpCaseMap = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
    0x60, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
    0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
    0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,
    0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
    0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
    0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
    0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
    0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
};

// Translation table from a char to it's lowercase translation. This table
// assumes strict ASCII
static _NLSTransTable LoCaseMap = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x40, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
    0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
    0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,
    0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
    0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
    0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
    0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
    0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
};


// A collation table of all chars. Initially this is a no-op table, this may
// be changed at runtime.
static _NLSTransTable CollMap = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
    0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
    0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,
    0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
    0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
    0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
    0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
    0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
};



// External references to above tables
const _NLSTransTable& NLSUpCaseMap      = UpCaseMap;
const _NLSTransTable& NLSLoCaseMap      = LoCaseMap;
const _NLSTransTable& NLSCollMap        = CollMap;



/*****************************************************************************/
/*                               NLS functions                               */
/*****************************************************************************/



int NLSSetCountry (unsigned NewCountry)
// Set up the NLS system for a new country. This affects the contents of the
// NLSData structure above, and the 3 translation tables (UpCase, LoCase,
// Coll). NLSCountry reflects the new country code after the call. If there
// is no support for the new country found, the call will not change anything.
// The old country code is returned if the change has been successfull, if
// there is no support for the new country code, the function returns -1.
{
    // Build the name of the data resource
    String ResName = FormatStr ("NATIONAL.%03d", NewCountry);

    // Check if the country data is available
    Container* C = (Container*) LoadResource (ResName, 0);
    if (C == NULL) {
        // No data available
        return -1;
    }

    // Retrieve the data from the container
    _NLSInfo* Info = (_NLSInfo*) C->GetData ();

    // Copy the data
    memcpy (&NLSD,     &Info->Data,     sizeof (NLSD));
    memcpy (UpCaseMap, Info->UpCaseMap, sizeof (UpCaseMap));
    memcpy (LoCaseMap, Info->LoCaseMap, sizeof (LoCaseMap));
    memcpy (CollMap,   Info->CollMap,   sizeof (CollMap));

    // Delete the container
    delete C;

    // Remember the new country code and return the old one
    unsigned OldCountry = Country;
    Country = NewCountry;
    return OldCountry;

}



int NLSSetLanguage (unsigned NewLanguage)
// A new language code is set, the old one is returned. The complete resource
// system will search for a resource in the given language, then for a generic
// resource, then fall back to an english resource. The function will _not_
// detect if there is no support for the new language.
{
    unsigned OldLanguage = Language;
    Language = NewLanguage;
    return OldLanguage;
}



unsigned NLSGetDefaultLanguage (unsigned Country)
// Get the default language for the given country
{
    static struct {
        u16 Country;
        u16 Language;
    } Map [] = {
        {       1,      laEnglish       },
        {       44,     laEnglish       },
        {       33,     laFrench        },
        {       49,     laGerman        },
        {       61,     laEnglish       },
        {       99,     laEnglish       },
        {       31,     laDutch         },
        {       34,     laSpanish       },
        {       39,     laItalian       },
        {       41,     laGerman        },      // ? Swiss?
        {       2,      laFrench        },

        // The following are incorrect set
        {       32,     laEnglish       },      // Belgium
        {       42,     laEnglish       },      // Czechoslovakia
        {       45,     laEnglish       },      // Denmark
        {       358,    laEnglish       },      // Finland
        {       36,     laEnglish       },      // Hungary
        {       354,    laEnglish       },      // Iceland
        {       3,      laEnglish       },      // Latin america
        {       47,     laEnglish       },      // Norway
        {       48,     laEnglish       },      // Poland
        {       351,    laEnglish       },      // Portugal
        {       46,     laEnglish       },      // Sweden
        {       90,     laEnglish       }       // Turkey
    };


    // Linear search
    for (unsigned I = 0; I < sizeof (Map) / sizeof (Map [0]); I++) {
        if (Map [I].Country == Country) {
            return Map [I].Language;
        }
    }

    // Not found, use english
    return laEnglish;
}



char* NLSUpStr (char* S)
// Converts the given string to upper case according to the country information
// currently set. S is returned.
{
    char *P = S;
    while (*P) {
        *P = NLSUpCaseMap [(unsigned char) *P];
        P++;
    }
    return S;
}



char* NLSLoStr (char* S)
// Converts the given string to lower case according to the country information
// currently set. S is returned.
{
    char *P = S;
    while (*P) {
        *P = NLSLoCaseMap [(unsigned char) *P];
        P++;
    }
    return S;
}



int NLSCmpStr (const char* S1, const char* S2)
// Compare both strings according to the local collating sequence.
{
    int Diff = 0;
    while (1) {
        Diff = ((int)(unsigned char) NLSCollMap [(unsigned char) *S1]) -
               ((int)(unsigned char) NLSCollMap [(unsigned char) *S2]);
        if (Diff != 0 || *S1 == '\0') {
            return Diff;
        }
        S1++;
        S2++;
    }
}



/*****************************************************************************/
/*      Converting between internal and extern character representation      */
/*****************************************************************************/



char* InputCvt (char* S)
// Transform the given string S from the local input character set to the
// internal used character set
{
    char* P = S;
    while (*P) {
        *P = NLSInputMap [(unsigned char) *P];
        P++;
    }
    return S;
}



char* OutputCvt (char* S)
// Transform the given string S from the internal used character set to the
// local output character set
{
    char* P = S;
    while (*P) {
        *P = NLSOutputMap [(unsigned char) *P];
        P++;
    }
    return S;
}



