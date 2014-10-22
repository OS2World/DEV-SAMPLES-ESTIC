/*****************************************************************************/
/*									     */
/*				   INIFILE.H				     */
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



#ifndef __INIFILE_H
#define __INIFILE_H



#include "textstrm.h"



/*****************************************************************************/
/*				 class IniFile				     */
/*****************************************************************************/



// Forward to an implementation class
class SectionColl;



class IniFile: public Object {

protected:
    TextFileStream*	S;
    String		Filename;	// Name of the ini file
    class SectionColl*	Sections;	// Collection with section offsets
    int			Translate;	// Translate input strings if true


    void SetupIndex ();
    // Fill all sections into the index collection.

    void DelComment (String& S);
    // Delete a trailing comment from the string S

    i32 FindSection (String Section);
    // Seek to the line after the section header. Return the current line number
    // or -1 if the section was not found

    String FindKey (String Key, u32 CurLine);
    // Find the line with the given key. The search ends when end of file is
    // reached or when a new section begins. The complete line containing
    // the key is returned, or, if the key is not found, an empty string.

    String ReadLine (const String& Section, const String& Key);
    // return the Line or an empty string if the section/key does not exist

    String GetLine (const String& Section, const String& Key);
    // Same as ReadLine, but the Section/Key must exist. If it does not,
    // the program is aborted via Fail

    void Fail (const String& Section, const String& Key);
    // Called from GetString/GetInt when the section/key is not found. Ends
    // the program via FAIL


public:
    IniFile (const String& Name, int InputTranslate = 1);
    // Open the stream in read-only mode. A non existant file is considered
    // as an error. If InputTranslate is set to 1, read strings(!) are
    // translated from the external to the internal used character set.

    virtual ~IniFile ();
    // Delete the TextFileStream

    int KeyExists (const String& Section, const String& Key);
    int KeyExists (const char* Section, const char* Key);
    // returns 1 if the given key exists, 0 otherwise

    i32 ReadInt (const String& Section, const String& Key, i32 Default);
    i32 ReadInt (const char* Section, const char* Key, i32 Default);
    // return the int or Default if the section/key doesn't exist

    double ReadFloat (const String& Section, const String& Key, double Default);
    double ReadFloat (const char* Section, const char* Key, double Default);
    // Return a double or default if section/key doesn't exist

    String ReadString (const String& Section, const String& Key, const String& Default);
    String ReadString (const char* Section, const char* Key, const String& Default);
    // return the String or default if the section/key does not exist

    i32 ReadKeyword (const String& Section, const String& Key, const String& Keywords);
    i32 ReadKeyword (const char* Section, const char* Key, const String& Keywords);
    // Read a key value from the file and match it against a list of keywords
    // using MatchKeyword from module string. The number associated with the
    // matched keyword, or the default is returned. Beware: Case is ignored!

    int ReadBool (const String& Section, const String& Key, int Default);
    int ReadBool (const char* Section, const char* Key, int Default);
    // Return a boolean or default if the section/key does not exist

    i32 GetInt (const String& Section, const String& Key);
    i32 GetInt (const char* Section, const char* Key);
    // Same as ReadInt, but the Section/Key must exist. If it does not,
    // the program is aborted via FAIL

    double GetFloat (const String& Section, const String& Key);
    double GetFloat (const char* Section, const char* Key);
    // Same as ReadFloat, but the Section/Key must exist. If it does not,
    // the program is aborted via FAIL

    String GetString (const String& Section, const String& Key);
    String GetString (const char* Section, const char* Key);
    // Same as ReadString, but the Section/Key must exist. If it does not,
    // the program is aborted via FAIL

    i32 GetKeyword (const String& Section, const String& Key, const String& Keywords);
    i32 GetKeyword (const char* Section, const char* Key, const String& Keywords);
    // Same as ReadKeyword, but the Section/Key must exist. If it does not,
    // the program is aborted via FAIL. Note: The function will not detect if
    // section and key are valid but the key value is not.

    int GetBool (const String& Section, const String& Key, int Default);
    int GetBool (const char* Section, const char* Key, int Default);
    // Same as ReadBool, but the Section/Key must exist. If it does not,
    // the program is aborted via FAIL

    int GetStatus () const;
    // Get the stream status

    int GetErrorInfo () const;
    // Get the stream error info
};



inline int IniFile::GetStatus () const
// Get the stream status
{
    return S->GetStatus ();
}



inline int IniFile::GetErrorInfo () const
// Get the stream error info
{
    return S->GetErrorInfo ();
}



// End of INIFILE.H

#endif



