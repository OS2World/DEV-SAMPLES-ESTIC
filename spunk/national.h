/*****************************************************************************/
/*									     */
/*				  NATIONAL.H				     */
/*									     */
/* (C) 1993-96	Ullrich von Bassewitz					     */
/*		Wacholderweg 14						     */
/*		D-70597 Stuttgart					     */
/* EMail:	uz@ibb.schwaben.com					     */
/*									     */
/*****************************************************************************/



// $Id$
//
// $Log$
//
//



#ifndef _NATIONAL_H
#define _NATIONAL_H



#include "machine.h"



/*****************************************************************************/
/*				Types and data				     */
/*****************************************************************************/



// Language constants
const unsigned laGerman		= 1;
const unsigned laEnglish	= 2;
const unsigned laFrench		= 3;
const unsigned laDutch		= 4;
const unsigned laSpanish	= 5;
const unsigned laItalian	= 6;



// Struct holding NLS information
struct _NLSData {
    char	Date;		// Date format, 0=m/d/y, 1=d/m/y, 2=y/m/d
    char	Time;		// Time format, 0 = am/pm, 1 = military/european
    char	Curr;		// Currency style
    char	CurrStr [5];	// Currency string
    char	ThSep;		// Thousands separator
    char	DecSep;		// Decimal separator
    char	DateSep;	// Date separator
    char	TimeSep;	// Time separator
    char	DataSep;	// Data separator
    char	_Fill [19];	// Pad to 32 bytes
};



// The default country and language. This settings are used if no other
// information about language/country is available and must be set _before_
// initialization of the application object. Default is USA and US english.
extern unsigned DefaultCountry;
extern unsigned DefaultLanguage;



// A _NLSData struct set up for the local country the current country and
// the language to use.
extern const _NLSData& NLSData;
extern const unsigned& NLSLanguage;
extern const unsigned& NLSCountry;



// A translation table
typedef char _NLSTransTable [256];



// References to several translation tables
extern const _NLSTransTable& NLSUpCaseMap;
extern const _NLSTransTable& NLSLoCaseMap;
extern const _NLSTransTable& NLSInputMap;
extern const _NLSTransTable& NLSOutputMap;
extern const _NLSTransTable& NLSCollMap;



/*****************************************************************************/
/*				 NLS functions				     */
/*****************************************************************************/



int NLSSetCountry (unsigned CountryCode);
// Set up the NLS system for a new country. This affects the contents of the
// NLSData structure above, and the 3 translation tables (UpCase, LoCase,
// Coll). NLSCountry reflects the new country code after the call. If there
// is no support for the new country found, the call will not change anything.
// The old country code is returned if the change has been successfull, if
// there is no support for the new country code, the function returns -1.

int NLSSetLanguage (unsigned Language);
// A new language code is set, the old one is returned. The complete resource
// system will search for a resource in the given language, then for a generic
// resource, then fall back to an english resource. The function will _not_
// detect if there is no support for the new language.

unsigned NLSGetDefaultLanguage (unsigned Country);
// Get the default language for the given country

inline char NLSUpCase (char C)
// Converts the given char to upper case according to the country information
// currently set
{
    return NLSUpCaseMap [(unsigned char) C];
}

inline char NLSLoCase (char C)
// Converts the given char to lower case according to the country information
// currently set
{
    return NLSLoCaseMap [(unsigned char) C];
}

char* NLSUpStr (char* S);
// Converts the given string to upper case according to the country information
// currently set. S is returned.

char* NLSLoStr (char* S);
// Converts the given string to lower case according to the country information
// currently set. S is returned.

int NLSCmpStr (const char* S1, const char* S2);
// Compare both strings according to the local collating sequence.

void NLSInit ();
// Sets up the data for the NLS. This function has to be called before any call
// to another function in this module!



/*****************************************************************************/
/*	Converting between internal and extern character representation      */
/*****************************************************************************/



inline char InputCvt (char C)
// Transform the given character C from the local input character set to the
// internal used character set
{
    return NLSInputMap [(unsigned char) C];
}

char* InputCvt (char* S);
// Transform the given string S from the local input character set to the
// internal used character set

inline char OutputCvt (char C)
// Transform the given character C from the internal used character set to the
// local output character set
{
    return NLSOutputMap [(unsigned char) C];
}

char* OutputCvt (char* S);
// Transform the given string S from the internal used character set to the
// local output character set



// End of NATIONAL.H

#endif


