/*****************************************************************************/
/*									     */
/*				   ENVIRON.CC				     */
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



#include <stdlib.h>

#include "str.h"
#include "strparse.h"



/*****************************************************************************/
/*				     Code				     */
/*****************************************************************************/



String GetEnvVar (const char* Var)
// Retrieve an environment var. The result is empty if the string does not
// exist.
{
    return String (getenv (Var));
}



String GetEnvVar (const String& Var)
// Retrieve an environment var. The result is empty if the string does not
// exist.
{
    return String (getenv (Var.GetStr ()));
}



i32 GetEnvVal (const char* Var, const String& ValStr)
// Read the environment variable Var and compare the value with values stored
// in ValStr. ValStr looks like this: "1^ON|0^OFF|1^YES|0^NO|", meaning:
// Return 1 if the value is "ON", return 0 if the value is "OFF" etc.
// Case is ignored when comparing the value. The first value stored in ValStr
// is the default, it is returned if Var does not exist or is invalid.
// The function calls FAIL if the given ValStr is invalid.
// Don't forget the trailing '|' !
{
    // Get the environment value
    String Val = GetEnvVar (Var);

    // Match the keyword ignoring case
    return MatchKeyword (Val.ToUpper (), ToUpper (ValStr));
}



i32 GetEnvVal (const String& Var, const String& ValStr)
// Read the environment variable Var and compare the value with values stored
// in ValStr. ValStr looks like this: "1^ON|0^OFF|1^YES|0^NO|", meaning:
// Return 1 if the value is "ON", return 0 if the value is "OFF" etc.
// Case is ignored when comparing the value. The first value stored in ValStr
// is the default, it is returned if Var does not exist or is invalid.
// The function calls FAIL if the given ValStr is invalid. Don't forget the
// trailing '|' !
{
    // BC complains over a (nonexistant) ambiguity here, so do explicit
    // casting here...
    return GetEnvVal (Var.GetStr (), String (ValStr));
}



i32 GetEnvNum (const char* Var, i32 Default)
// Read the environment variable Var and treat the value as a number string.
// Return the converted number or Default if the variable does not exist or
// contains an invalid value.
{
    // Get the value
    String Val = GetEnvVar (Var);

    // Try to convert it to a number
    StringParser SP (Val);
    i32 Value;
    if (SP.GetI32 (Value) == 0) {
	// Done
	return Value;
    } else {
	// Error
	return Default;
    }
}



i32 GetEnvNum (const String& Var, i32 Default)
// Read the environment variable Var and treat the value as a number string.
// Return the converted number or Default if the variable does not exist or
// contains an invalid value.
{
    return GetEnvNum (Var.GetStr (), Default);
}



int GetEnvBool (const char* Var, int Default)
// Read the environment variable and treat it as a boolean value. Accepted
// values are "on" "off" "yes" "no" "1" "0". Default is returned if the
// variable does not exist or the value is invalid.
{
    if (Default) {
	return GetEnvVal (Var, String ("1^YES|0^NO|1^ON|0^OFF|1^1|0^0|"));
    } else {
	return GetEnvVal (Var, String ("0^NO|1^YES|0^OFF|1^ON|0^0|1^1|"));
    }
}



int GetEnvBool (const String& Var, int Default)
// Read the environment variable and treat it as a boolean value. Accepted
// values are "on" "off" "yes" "no" "1" "0". Default is returned if the
// variable does not exist or the value is invalid. Case of the value is
// ignored by this function.
{
    return GetEnvBool (Var.GetStr (), Default);
}



