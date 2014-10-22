/*****************************************************************************/
/*									     */
/*				   INIFILE.CC				     */
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



#include "inifile.h"
#include "msgid.h"
#include "strparse.h"
#include "progutil.h"
#include "coll.h"



/*****************************************************************************/
/*			       Message constants			     */
/*****************************************************************************/



static const u16 msKeyNotFound		= MSGBASE_INIFILE +  0;



/*****************************************************************************/
/*			Explicit template instantiation			     */
/*****************************************************************************/



#ifdef EXPLICIT_TEMPLATES
template class Collection<class SectionOfs>;
template class SortedCollection<class SectionOfs, String>;
#endif



/*****************************************************************************/
/*			       class SectionOfs				     */
/*****************************************************************************/



class SectionOfs: public Object {

    friend class SectionColl;

private:
    String	Section;	// Name of the section
    u32		Line;		// Line after section header

public:
    SectionOfs (const String& SectionName, u32 LineNum);

};



SectionOfs::SectionOfs (const String& SectionName, u32 LineNum):
    Section (SectionName),
    Line (LineNum)
{
}



/*****************************************************************************/
/*			       class SectionColl			     */
/*****************************************************************************/



class SectionColl: public SortedCollection<SectionOfs, String> {

protected:
    // Derived from class SortedCollection
    virtual int Compare (const String* Key1, const String* Key2);
    virtual const String* KeyOf (const SectionOfs* Item);

public:
    SectionColl ();

    i32 GetSection (const String& SectionName);
    // Search for the section, return -1 if not found, Line otherwise
};



SectionColl::SectionColl ():
    SortedCollection<SectionOfs, String> (50, 10)
{
    ShouldDelete = 1;
}



int SectionColl::Compare (const String* Key1, const String* Key2)
{
    return ::Compare (*Key1, *Key2);
}



const String* SectionColl::KeyOf (const SectionOfs* Item)
{
    return &Item->Section;
}



i32 SectionColl::GetSection (const String& SectionName)
// Search for the section, return -1 if not found, Line otherwise
{
    int Index;
    if (Search (&SectionName, Index) == 0) {
	// Not found
	return -1;
    } else {
	return At (Index)->Line;
    }
}



/*****************************************************************************/
/*				 class IniFile				     */
/*****************************************************************************/



IniFile::IniFile (const String& Name, int InputTranslate):
    S (new TextFileStream (Name)),
    Filename (Name),
    Sections (new SectionColl),
    Translate (InputTranslate)
// Open the stream in read-only mode. A non existant file is considered
// an error.
{
    SetupIndex ();
}



IniFile::~IniFile ()
// Delete the TextFileStream
{
    delete Sections;
    delete S;
}



void IniFile::SetupIndex ()
// Fill all sections into the index collection.
{
    // Seek to the start of the file
    S->LineSeek (0);

    // search for the section start points
    u32 LineNum = 0;
    while (LineNum < S->LineCount ()) {

	// Read the next line
	String Line = S->GetLine ();
	LineNum++;

	// Skip empty lines and lines, not beginning with '['
	if (Line.IsEmpty () || Line [0] != '[') {
	    continue;
	}

	// Ok, a line with a section name is found, delete the '['
	Line.Del (0, 1);

	// Search for the end marker
	int Pos = Line.Pos (']');
	if (Pos == -1) {
	    // Ill-formed line, skip it
	    continue;
	}

	// Ok, we have a valid entry. Extract the section name and uppercase it
	Line.Trunc (Pos);
	Line.ToUpper ();

	// Insert the section together with the starting line into the line coll
	Sections->Insert (new SectionOfs (Line, LineNum));

    }

}



void IniFile::DelComment (String& S)
// Delete a trailing comment from the string S
{
    int Pos = S.Pos (';');
    if (Pos != -1) {
	S.Del (Pos, S.Len () - Pos);
    }
}



i32 IniFile::FindSection (String Section)
// Seek to the line after the section header. Return the current line number
// or -1 if the section was not found
{
    // Search for the section entry in the index
    i32 LineNum = Sections->GetSection (Section.ToUpper ());
    if (LineNum >= 0) {
	// Found the section, seek to the position
	S->LineSeek (LineNum);
    }
    return LineNum;
}



String IniFile::FindKey (String Key, u32 CurLine)
// Find the line with the given key. The search ends when end of file is
// reached or when a new section begins. The complete line containing
// the key is returned, or, if the key is not found, an empty string.
{
    // The search is not case sensitive
    Key.ToUpper ();

    // Search for the key
    while (CurLine < S->LineCount ()) {

	// Read a line
	String Line (S->GetLine ());
	CurLine++;

	// If the Line is empty or has a comment leader, skip it
	if (Line.IsEmpty () || Line [0] == ';') {
	    continue;
	}

	// If the line is a section marker, end the search
	if (Line [0] == '[') {
	    break;
	}

	// Otherwise extract the Key part of the line
	int Pos = Line.Pos ('=');
	if (Pos == -1) {
	    // Ill formed line, skip it
	    continue;
	}

	// Extract the first part of the line, uppercase it and delete blanks
	String KeyPart (Line.Cut (0, Pos));
	Pos--;
	while (Pos >= 0 && Line [Pos] == ' ') {
	    Pos--;
	}
	KeyPart.Trunc (Pos+1);
	KeyPart.ToUpper ();

	// Now check if the keys match
	if (KeyPart == Key) {
	    // Found it
	    return Line;
	}

    }

    // Key not found
    return String ("");

}



void IniFile::Fail (const String& Section, const String& Key)
// Called from GetString/GetInt when the section/key is not found. Ends
// the program via FAIL
{
    FAIL (FormatStr (LoadMsg (msKeyNotFound).GetStr (),
	  Filename.GetStr (), Section.GetStr (), Key.GetStr ()).GetStr ());
}



int IniFile::KeyExists (const String& Section, const String& Key)
// returns 1 if the given key exists, 0 otherwise
{
    // Search for the section
    i32 CurLine = FindSection (Section);
    if (CurLine == -1) {
	// Section not found
	return 0;
    }

    return (!FindKey (Key, CurLine).IsEmpty ());
}



int IniFile::KeyExists (const char* Section, const char* Key)
// returns 1 if the given key exists, 0 otherwise
{
    return KeyExists (String (Section), String (Key));
}



String IniFile::ReadLine (const String& Section, const String& Key)
// return the Line or an empty string if the section/key does not exist
{
    // Search for the section
    i32 CurLine = FindSection (Section);
    if (CurLine == -1) {
	// Section not found
	return "";
    }

    // Now search for the key
    String Line (FindKey (Key, CurLine));
    if (Line.IsEmpty ()) {
	// Key not found
	return "";
    }

    // Extract the data part and return it
    int Pos = Line.Pos ('=');
    Line.Del (0, Pos+1);
    return Line;
}



i32 IniFile::ReadInt (const String& Section, const String& Key, i32 Default)
// return the int or Default if the section/key doesn't exist
{
    // Read the line
    String S (ReadLine (Section, Key));
    if (S.IsEmpty ()) {
	// Key not found
	return Default;
    }

    // Allow comments
    DelComment (S);

    // Convert the value
    StringParser P (S);
    P.SetFlags (StringParser::SkipWS | StringParser::PascalHex | StringParser::CHex);
    i32 Val;
    if (P.GetI32 (Val) != 0) {
	// Parsing error, return default
	return Default;
    } else {
	// Ok, return the value
	return Val;
    }
}



i32 IniFile::ReadInt (const char* Section, const char* Key, i32 Default)
// return the int or Default if the section/key doesn't exist
{
    return ReadInt (String (Section), String (Key), Default);
}



double IniFile::ReadFloat (const String& Section, const String& Key, double Default)
// Return a double or default if section/key doesn't exist
{
    // Read the key as a string
    String S (ReadLine (Section, Key));
    if (S.IsEmpty ()) {
	// Key not found
	return Default;
    }

    // Allow comments
    DelComment (S);

    StringParser P (S, StringParser::SkipWS | StringParser::AllowDP);
    double Val;
    if (P.GetFloat (Val) != 0) {
	// Parsing error, return default
	return Default;
    } else {
	// Ok, return the value
	return Val;
    }
}



double IniFile::ReadFloat (const char* Section, const char* Key, double Default)
// Return a double or default if section/key doesn't exist
{
    return ReadFloat (String (Section), String (Key), Default);
}



String IniFile::ReadString (const String& Section, const String& Key, const String& Default)
// return the String or default if the section/key does not exist
{
    // Read the data part of the line
    String Line = ReadLine (Section, Key);
    if (Line.IsEmpty ()) {
	// Key not found or empty
	return Default;
    }

    // Extract the string
    int Pos = Line.ScanR ('"');
    if (Pos == -1) {
	return "";
    }
    Line.Del (0, Pos+1);
    Pos = Line.ScanL ('"');
    if (Pos == -1) {
	return "";
    }
    Line.Trunc (Pos);

    // If Translate is set, translate the string from the external into the
    // internal representation
    if (Translate) {
	Line.InputCvt ();
    }

    // Return the string
    return Line;
}



String IniFile::ReadString (const char* Section, const char* Key, const String& Default)
// return the String or default if the section/key does not exist
{
    return ReadString (String (Section), String (Key), Default);
}



i32 IniFile::ReadKeyword (const String& Section, const String& Key, const String& Keywords)
// Read a key value from the file and match it against a list of keywords
// using MatchKeyword from module string. The number associated with the
// matched keyword, or the default is returned. Beware: Case is ignored!
{
    // Read the line
    String S = ReadLine (Section, Key);

    // Allow comments
    DelComment (S);

    // Delete whitespace from the value
    S.Remove (WhiteSpace, rmLeading | rmTrailing);

    // Convert the value
    return MatchKeyword (S.ToUpper (), Keywords);
}



i32 IniFile::ReadKeyword (const char* Section, const char* Key, const String& Keywords)
// Read a key value from the file and match it against a list of keywords
// using MatchKeyword from module string. The number associated with the
// matched keyword, or the default is returned. Beware: Case is ignored!
{
    return ReadKeyword (String (Section), String (Key), Keywords);
}



int IniFile::ReadBool (const String& Section, const String& Key, int Default)
// Return a boolean or default if the section/key does not exist
{
    if (Default) {
	return ReadKeyword (Section, Key, "1^YES|0^NO|1^ON|0^OFF|1^1|0^0|");
    } else {
	return ReadKeyword (Section, Key, "0^NO|1^YES|0^OFF|1^ON|0^0|1^1|");
    }
}



int IniFile::ReadBool (const char* Section, const char* Key, int Default)
// Return a boolean or default if the section/key does not exist
{
    return ReadBool (String (Section), String (Key), Default);
}



String IniFile::GetLine (const String& Section, const String& Key)
// Same as ReadLine, but the Section/Key must exist. If it does not,
// the program is aborted via Fail
{
    // Search for the section
    i32 CurLine = FindSection (Section);
    if (CurLine == -1) {
	// Section not found
	Fail (Section, Key);
    }

    // Now search for the key
    String Line (FindKey (Key, CurLine));
    if (Line.IsEmpty ()) {
	// Key not found
	Fail (Section, Key);
    }

    // Extract data part and return it
    int Pos = Line.Pos ('=');
    Line.Del (0, Pos+1);
    return Line;
}



i32 IniFile::GetInt (const String& Section, const String& Key)
// Same as ReadInt, but the Section/Key must exist. If it does not,
// the program is aborted via Fail
{
    // Read the key as a string
    String S (GetLine (Section, Key));
    if (S.IsEmpty ()) {
	// Key not found
	Fail (Section, Key);
    }

    // Allow comments
    DelComment (S);

    // Convert the value
    StringParser P (S);
    P.SetFlags (StringParser::SkipWS | StringParser::PascalHex | StringParser::CHex);
    i32 Val;
    if (P.GetI32 (Val) != 0) {
	// Parsing error, abort program
	Fail (Section, Key);
    }

    // Ok, return the value
    return Val;
}



i32 IniFile::GetInt (const char* Section, const char* Key)
// Same as ReadInt, but the Section/Key must exist. If it does not,
// the program is aborted via Fail
{
    return GetInt (String (Section), String (Key));
}



double IniFile::GetFloat (const String& Section, const String& Key)
// Same as ReadFloat, but the Section/Key must exist. If it does not,
// the program is aborted via FAIL
{
    // Read the key as a string
    String S (GetLine (Section, Key));
    if (S.IsEmpty ()) {
	// Key not found
	Fail (Section, Key);
    }

    // Allow comments
    DelComment (S);

    StringParser P (S, StringParser::SkipWS | StringParser::AllowDP);
    double Val;
    if (P.GetFloat (Val) != 0) {
	// Parsing error, abort program
	Fail (Section, Key);
    }

    // Ok, return the value
    return Val;
}



double IniFile::GetFloat (const char* Section, const char* Key)
// Same as ReadFloat, but the Section/Key must exist. If it does not,
// the program is aborted via FAIL
{
    return GetFloat (String (Section), String (Key));
}



String IniFile::GetString (const String& Section, const String& Key)
// Same as ReadString, but the Section/Key must exist. If it does not,
// the program is aborted via Fail
{
    // Now search for the key
    String Line = GetLine (Section, Key);
    if (Line.IsEmpty ()) {
	// Key not found
	Fail (Section, Key);
    }

    // Extract the string and return it
    int Pos = Line.ScanR ('"');
    if (Pos == -1) {
	Fail (Section, Key);
    }
    Line.Del (0, Pos+1);
    Pos = Line.ScanL ('"');
    if (Pos == -1) {
	Fail (Section, Key);
    }
    Line.Trunc (Pos);

    // If Translate is set, translate the string from the external into the
    // internal representation
    if (Translate) {
	Line.InputCvt ();
    }

    // Return the result
    return Line;
}



String IniFile::GetString (const char* Section, const char* Key)
// Same as ReadString, but the Section/Key must exist. If it does not,
// the program is aborted via Fail
{
    return GetString (String (Section), String (Key));
}



i32 IniFile::GetKeyword (const String& Section, const String& Key, const String& Keywords)
{
    // Read the line
    String S = ReadLine (Section, Key);
    if (S.IsEmpty ()) {
	// Key not found
	Fail (Section, Key);
    }

    // Allow comments
    DelComment (S);

    // Delete whitespace from the value
    S.Remove (WhiteSpace, rmLeading | rmTrailing);

    // Convert the value
    return MatchKeyword (S.ToUpper (), Keywords);
}



i32 IniFile::GetKeyword (const char* Section, const char* Key, const String& Keywords)
// Same as ReadKeyword, but the Section/Key must exist. If it does not,
// the program is aborted via FAIL
{
    return GetKeyword (String (Section), String (Key), Keywords);
}



int IniFile::GetBool (const String& Section, const String& Key, int Default)
// Same as ReadBool, but the Section/Key must exist. If it does not,
// the program is aborted via FAIL
{
    if (Default) {
	return GetKeyword (Section, Key, "1^YES|0^NO|1^ON|0^OFF|1^1|0^0|");
    } else {
	return GetKeyword (Section, Key, "0^NO|1^YES|0^OFF|1^ON|0^0|1^1|");
    }
}



int IniFile::GetBool (const char* Section, const char* Key, int Default)
// Same as ReadBool, but the Section/Key must exist. If it does not,
// the program is aborted via FAIL
{
    return GetBool (String (Section), String (Key), Default);
}



