/*****************************************************************************/
/*                                                                           */
/*                                ICALIAS.CC                                 */
/*                                                                           */
/* (C) 1995-96  Ullrich von Bassewitz                                        */
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



#include "chartype.h"
#include "coll.h"
#include "str.h"
#include "strcvt.h"
#include "strparse.h"
#include "stdmsg.h"
#include "progutil.h"

#include "icmsg.h"



/*****************************************************************************/
/*                             Message Constants                             */
/*****************************************************************************/



const u16 msDuplicateNumber             = MSGBASE_ICALIAS +  0;
const u16 msOpenError                   = MSGBASE_ICALIAS +  1;
const u16 msSyntaxError                 = MSGBASE_ICALIAS +  2;



/*****************************************************************************/
/*                      Explicit template instantiation                      */
/*****************************************************************************/



#ifdef EXPLICIT_TEMPLATES
template class Collection<class DevAlias>;
template class SortedCollection<class DevAlias, String>;
#endif



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



// Name of the alias file, default is the empty string
String AliasFile = "";

// If true, reread the alias file before trying to resolve call data
int AutoReadAliases     = 0;



/*****************************************************************************/
/*                              class DevAlias                               */
/*****************************************************************************/



class DevAlias: public Streamable {

public:
    String              Number;
    String              Alias;

    DevAlias (const String& aNumber, const String& aAlias);

};



DevAlias::DevAlias (const String& aNumber, const String& aAlias):
    Number (aNumber),
    Alias (aAlias)
{
}



/*****************************************************************************/
/*                              class AliasColl                              */
/*****************************************************************************/



class AliasColl: public SortedCollection<DevAlias, String> {

protected:
    virtual int Compare (const String* Key1, const String* Key2);
    virtual const String* KeyOf (const DevAlias* Item);

public:
    AliasColl ();
    // Create a AliasColl

    virtual DevAlias* At (int Index);
    // Return a pointer to the item at position Index.
    // OVERRIDE FOR DEBUGGING

};



int AliasColl::Compare (const String* Key1, const String* Key2)
{
    return ::Compare (*Key1, *Key2);
}



const String* AliasColl::KeyOf (const DevAlias* Item)
{
    return &Item->Number;
}



AliasColl::AliasColl ():
    SortedCollection <DevAlias, String> (25, 25, 1)
{
}



DevAlias* AliasColl::At (int Index)
// Return a pointer to the item at position Index.
// OVERRIDE FOR DEBUGGING
{
    // Check range
    if (Index < 0 || Index >= Count) {
        FAIL ("AliasColl::At: Index out of bounds");
        return NULL;
    }

    return SortedCollection<DevAlias, String>::At (Index);
}



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



static AliasColl AliasBase;



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



static void CleanupNumber (String& Number)
// Remove all non-digit characters from the given number
{
    // Remove all non-digit characters from the number
    unsigned I = 0;
    while (I < Number.Len ()) {
        if (!IsDigit (Number [I])) {
            // No digit, delete it
            Number.Del (I, 1);
        } else {
            // Next char
            I++;
        }
    }
}



void NewAlias (String Number, const String& Alias)
// Create a new number alias.
{
    // Remove invalid digits
    CleanupNumber (Number);

    // If the number is empty now, return silently
    if (Number.IsEmpty ()) {
        return;
    }

    // Check if there is already an alias for this device
    int Index;
    if (AliasBase.Search (&Number, Index) != 0) {
        // OOPS - this should not happen. Print an error message
        ErrorMsg (FormatStr (LoadAppMsg (msDuplicateNumber).GetStr (),
                             Number.GetStr ()));
        return;
    }

    // Create a new object instance
    DevAlias* DA = new DevAlias (Number, Alias);

    // Insert it into the AliasBase
    AliasBase.Insert (DA);
}



void NewAlias (unsigned char Dev, const String& Alias)
// Create a new device alias. The valid range for dev is 21...
{
    // Convert the device number to a string. Use a temporary variable
    // because gcc will not work correctly otherwise :-(
    String Number = U32Str (Dev);
    NewAlias (Number, Alias);
}



const String& GetAlias (String Number)
// Return the alias of a number. Return an empty string if there is no alias.
{
    // Remove invalid digits
    CleanupNumber (Number);

    // If the number is empty now, we do not have an alias
    if (Number.IsEmpty ()) {
        return EmptyString;
    }

    // Search for the entry
    int Index;
    if (AliasBase.Search (&Number, Index) == 0) {
        // No entry, return an empty string
        return EmptyString;
    } else {
        // Found, return the alias
        return AliasBase [Index]->Alias;
    }
}



const String& GetAlias (unsigned char Dev)
// Return the alias of a device. Return an empty string if there is no alias.
// The valid range for Dev is 21...
{
    // Convert the device number to a string. Use a temporary variable
    // because gcc will not work correctly otherwise :-(
    String Number = U32Str (Dev);
    return GetAlias (Number);
}



void ReadAliasFile ()
// Delete all existing aliases and read in the aliasfile with the given name.
// The function does nothing if there is nov external aliasfile defined.
{
    // Bail out if there is no valid alias file name
    if (AliasFile.IsEmpty ()) {
        return;
    }

    // Delete all existing aliases
    AliasBase.DeleteAll ();

    // Try to open the file
    FILE* F = fopen (AliasFile.GetStr (), "rt");
    if (F == NULL) {
        // OOPS, file open error
        ErrorMsg (LoadAppMsg (msOpenError));
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

        // Extract the first token (the number)
        String Number;
        if (SP.GetToken (Number) != 0) {
            // Error
            ErrorMsg (FormatStr (LoadAppMsg (msSyntaxError).GetStr (), LineCount));
            continue;
        }

        // Remove all non-digit characters from the number
        CleanupNumber (Number);

        // The number must not be empty now
        if (Number.IsEmpty ()) {
            // Error
            ErrorMsg (FormatStr (LoadAppMsg (msSyntaxError).GetStr (), LineCount));
            continue;
        }

        // Now get the alias
        String Alias;
        if (SP.GetString (Alias) != 0) {
            // Error
            ErrorMsg (FormatStr (LoadAppMsg (msSyntaxError).GetStr (), LineCount));
            continue;
        }

        // Ok, done. Insert the alias into the alias database
        NewAlias (Number, Alias);

    }

    // Close the file
    fclose (F);
}



