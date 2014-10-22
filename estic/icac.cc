/*****************************************************************************/
/*									     */
/*				    ICAC.CC				     */
/*									     */
/* (C) 1997	Ullrich von Bassewitz					     */
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



// This is the ESTIC interface to the area code module



#include "chartype.h"

#include "../areacode/areacode.h"

#include "icac.h"



/*****************************************************************************/
/*				     Data				     */
/*****************************************************************************/



// Code for the country we say we are in
String CountryCode = "";

// Code for the area, we say we are in
String AreaCode = "711";

// The digit that is used as a dial prefix (usually '0')
char DialPrefix = '0';



/*****************************************************************************/
/*				     Code				     */
/*****************************************************************************/



void SetAreaCodeFilename (const String& Name)
// Set the name of the areacode data file
{
    unsigned Len = Name.Len ();
    if (Len > 0) {
	acFileName = strcpy (new char [Len+1], Name.GetStr ());
    }
}



String GetFullyQualifiedPhone (const String& Phone)
// Return a fully qualified phone number from the given number.
{
    const char IntPrefix [3]  = { DialPrefix, DialPrefix };
    const char AreaPrefix [2] = { DialPrefix };

    // Try to build a fully qualified number from the given phone number.
    String FQP = Phone;
    if (FQP.Cut (0, 2) == IntPrefix) {
	// This number is already fully qualified. Remove the dial prefix.
	FQP.Del (0, 2);
    } else if (FQP.Cut (0, 1) == AreaPrefix) {
	// The number has an area code. Remove the dial prefix and add the
	// country code.
	FQP.Del (0, 1);
	FQP.Ins (0, CountryCode);
    } else {
	// This is a local number. Add the area code and the country code.
	FQP = CountryCode + AreaCode + FQP;
    }

    // Return the result
    return FQP;
}



String IstecGetAreaCodeInfo (const String& Phone, unsigned& AreaCodeLen)
// Return an info string for the given phone number. The phone number is
// expected to use the usual national conventions, that is:
//
//    * if it is prefixed by two DialPrefix chars, it is assumed to be
//	"fully qualified", that means, it starts with a country code.
//    * if it is prefixed by one DialPrefix chars, it is assumed to be
//	a number with an area code but without a country code.
//    * if it does not start with any DialPrefix chars, it is assumed to
//	be a local number.
//
// The function returns the info string found and sets AreaCodeLen to the
// length of the area code in Phone. If no info is found, AreaCodeLen is set
// to zero.
{
    // Try to build a fully qualified number from the given phone number,
    // since this is, what the areacode module expects.
    String FQP = GetFullyQualifiedPhone (Phone);

    // Use the areacode module to search for an info.
    acInfo Info;
    GetAreaCodeInfo (&Info, FQP.GetStr ());

    // Check if we did find anything
    if (Info.AreaCodeLen == 0) {
	// OOPS, we did not find anything
	AreaCodeLen = 0;
	return "";
    }

    // We found an info. Return the data found
    AreaCodeLen = int (Info.AreaCodeLen) - (FQP.Len () - Phone.Len ());
    return InputCvt (Info.Info);
}



unsigned IstecGetAreaCodeLen (const String& Phone)
// Return the length of the area code for the given phone number. The phone
// number is expected to use the usual national conventions, that is:
//
//    * if it is prefixed by two DialPrefix chars, it is assumed to be
//	"fully qualified", that means, it starts with a country code.
//    * if it is prefixed by one DialPrefix chars, it is assumed to be
//	a number with an area code but without a country code.
//    * if it does not start with any DialPrefix chars, it is assumed to
//	be a local number.
//
// If no info is found, zero is returned.
{
    unsigned AreaCodeLen;
    String Dummy = IstecGetAreaCodeInfo (Phone, AreaCodeLen);
    return AreaCodeLen;
}



String IstecBeautifyPhone (const String& Phone)
// Try to "beautify" the phone number by putting a separator between the
// areacode and the local number
{
    // Be careful here: If we do not find the number in the database,
    // don't change anything, leave all separators in place
    if (Phone.NotEmpty ()) {
	String S = Phone;
	S.Remove ("+-/()", rmAll);
	unsigned AreaCodeLen = IstecGetAreaCodeLen (S);
	if (AreaCodeLen == 0) {
	    // Return the original string unchanged
	    return Phone;
	} else {
	    S.Ins (AreaCodeLen, '/');
	    return S;
	}
    } else {
	return Phone;
    }
}

