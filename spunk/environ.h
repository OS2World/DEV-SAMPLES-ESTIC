/*****************************************************************************/
/*									     */
/*				   ENVIRON.H				     */
/*									     */
/* (C) 1995-96	Ullrich von Bassewitz					     */
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



#ifndef _ENVIRON_H
#define _ENVIRON_H



#include "str.h"



/*****************************************************************************/
/*				     Code				     */
/*****************************************************************************/



String GetEnvVar (const char* Var);
// Retrieve an environment var. The result is empty if the string does not
// exist.

String GetEnvVar (const String& Var);
// Retrieve an environment var. The result is empty if the string does not
// exist.

i32 GetEnvVal (const char* Var, const String& ValStr);
// Read the environment variable Var and compare the value with values stored
// in ValStr. ValStr looks like this: "1^ON|0^OFF|1^YES|0^NO|", meaning:
// Return 1 if the value is "ON", return 0 if the value is "OFF" etc.
// Case is ignored when comparing the value. The first value stored in ValStr
// is the default, it is returned if Var does not exist or is invalid.
// The function calls FAIL if the given ValStr is invalid. Don't forget the
// trailing '|' !

i32 GetEnvVal (const String& Var, const String& ValStr);
// Read the environment variable Var and compare the value with values stored
// in ValStr. ValStr looks like this: "1^ON|0^OFF|1^YES|0^NO|", meaning:
// Return 1 if the value is "ON", return 0 if the value is "OFF" etc.
// Case is ignored when comparing the value. The first value stored in ValStr
// is the default, it is returned if Var does not exist or is invalid.
// The function calls FAIL if the given ValStr is invalid. Don't forget the
// trailing '|' !

i32 GetEnvNum (const char* Var, i32 Default = 0);
// Read the environment variable Var and treat the value as a number string.
// Return the converted number or Default if the variable does not exist or
// contains an invalid value.

i32 GetEnvNum (const String& Var, i32 Default = 0);
// Read the environment variable Var and treat the value as a number string.
// Return the converted number or Default if the variable does not exist or
// contains an invalid value.

int GetEnvBool (const char* Var, int Default = 1);
// Read the environment variable and treat it as a boolean value. Accepted
// values are "on" "off" "yes" "no" "1" "0". Default is returned if the
// variable does not exist or the value is invalid.

int GetEnvBool (const String& Var, int Default = 1);
// Read the environment variable and treat it as a boolean value. Accepted
// values are "on" "off" "yes" "no" "1" "0". Default is returned if the
// variable does not exist or the value is invalid. Case of the value is
// ignored by this function.



// End of ENVIRON.H

#endif

