/*****************************************************************************/
/*									     */
/*				    ICAC.H				     */
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



#ifndef _ICAC_H
#define _ICAC_H



#include "str.h"



/*****************************************************************************/
/*				     Data				     */
/*****************************************************************************/



// Code for the country we say we are in
extern String CountryCode;

// Code for the area, we say we are in
extern String AreaCode;

// The digit that is used as a dial prefix (usually '0')
extern char DialPrefix;



/*****************************************************************************/
/*				     Code				     */
/*****************************************************************************/



void SetAreaCodeFilename (const String& Name);
// Set the name of the areacode data file

String IstecGetAreaCodeInfo (const String& Phone, unsigned& AreaCodeLen);
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

unsigned IstecGetAreaCodeLen (const String& Phone);
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

String IstecBeautifyPhone (const String& Phone);
// Try to "beautify" the phone number by putting a separator between the
// areacode and the local number



// End of ICAC.H

#endif
