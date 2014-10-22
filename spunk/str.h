/*****************************************************************************/
/*                                                                           */
/*                                    STR.H                                  */
/*                                                                           */
/* (C) 1993-96  Ullrich von Bassewitz                                        */
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



#ifndef _STR_H
#define _STR_H



#include <string.h>
#include <stdarg.h>

#include "machine.h"
#include "object.h"
#include "strmable.h"
#include "national.h"



// Forwards
class Stream;
class CharSet;



/*****************************************************************************/
/*                                 Constants                                 */
/*****************************************************************************/



// Bitmapped constants that describe the behaviour of the ShowControls
// functions
const unsigned ccHex      = 0x0000;         // Use hex character code -
                                            // this is the default
const unsigned ccOct      = 0x0001;         // Use octal codes instead
                                            // of hex codes
const unsigned ccSpunk    = 0x0002;         // Show the special spunk
                                            // characters (\c for example)
                                            // not as codes but as names
const unsigned ccCStyle   = 0x0004;         // Use the C style character
                                            // codes like '\n', '\r'
                                            // instead of numerical ones
const unsigned ccDefault  = ccHex | ccCStyle;



// Bitmapped constants that describe the behaviour of the Remove functions
const unsigned rmLeading    = 0x0001;       // Remove leading chars
const unsigned rmTrailing   = 0x0002;       // Remove trailing chars
const unsigned rmInterWord  = 0x0004;       // Remove chars inside
const unsigned rmAll        = rmLeading | rmTrailing | rmInterWord;



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



// Character used as escape char when calling Match
extern unsigned char MatchEscChar;

// An empty string - for function returns and other stuff
extern const class String EmptyString;



/*****************************************************************************/
/*                               class String                                */
/*****************************************************************************/

// Note: This string class may not work with strings greater than INT_MAX or
//       0xFFFF, whichever is less


class String : public Streamable {

protected:
    char* Str;
    u16 Limit;
    u16 Length;

    // Internal used functions
    u16 Reblock (u16) const;
    void Resize (u16 NewLimit);
    void Init (const char* S, va_list ap);

public:
    enum PadType { Left, Center, Right };

    String (u16 Size = 0);
    String (const char* S);
    String (const String& S);
    String (const char* S, va_list ap);
    String (const String& S, va_list ap);
    String (StreamableInit);
    ~String ();

    // Derived from class Streamable
    virtual void Load (Stream&);
    virtual void Store (Stream&) const;
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    // New member functions

    String& Clear ();
    // Set length to zero, return this

    void Crypt ();
    // crypt (make unreadable) by a simple XOR scheme

    u16 Len () const;
    // Return the length of the string

    u16 Len (const char* S) const;
    // Return the length of the string not counting any chars contained in S

    u16 Len (const String& S) const;
    // Return the length of the string not counting any chars contained in S

    String& Add (const char* S, unsigned Len = 0, PadType P = Right);
    String& Add (const String& S, unsigned Len = 0, PadType P = Right);
    // Build lines of formatted text: If Len is zero, add S to the end of
    // this. If Len is non zero, use P to pad S to the given length, then
    // add it to the end of P.

    String Cut (unsigned Start, unsigned Count) const;
    // Return the part of the string beginning with index Start that is Count
    // chars long

    void Decrypt ();
    // Reverses the action done by Crypt

    void Del (unsigned Start, unsigned Count = 1);
    // Delete Count chars starting at index Start

    void ForceLen (int NewLen, PadType P = Right);
    // Force the string to a new length. If the string is too long, it is
    // replaced with a string of '!'s of length NewLen. If the string is too
    // short, it is padded to the given length using the padtype P.

    void Ins (unsigned Start, char C);
    // Insert a character into the string at position Start

    void Ins (unsigned Start, const char* S);
    // Insert a run of characters into the string at position Start

    void Ins (unsigned Start, const String &S);
    // Insert another string beginning with position Start

    int IsEmpty () const;
    // Return true if the string is empty (the length is zero)

    int NotEmpty () const;
    // Return true if the string is not empty (the length is greater than zero)

    int Match (const String& Pattern) const;
    int Match (const char* Pattern) const;
    // Match the string against Pattern. Pattern may contain the
    // wildcards '*', '?', '[abcd]' '[ab-d]', '[!abcd]', '[!ab-d]'. The
    // function returns a value of zero if Source does not match Pattern,
    // otherwise a non zero value is returned. If Pattern contains an invalid
    // wildcard pattern (e.g. 'A[x'), the function returns zero.

    u32 MatchKeyword (const String& Keywords) const;
    u32 MatchKeyword (const char* Keywords) const;
    // Match the string against a list of keywords given as a string. The list
    // of keywords looks like this: "1^ON|0^OFF|1^YES|0^NO|", meaning:
    // Return 1 if the value is "ON", return 0 if the value is "OFF" etc.
    // The match is case sensitive and each of the keywords may contain
    // wildcards as Match() is used to check for a match.

    String& Pad (PadType P, u16 Length, char C = ' ');
    // Pad the string to the given length using the padding char C. If the
    // string is too long, it is simply cut of at the end.

    int Pos (const char C) const;
    // Return the first position of the char C in the string. If C is not
    // found, the function returns -1

    int Pos (const char* S) const;
    // Return the first position of the string S in the string. If S is not
    // found, the function returns -1

    int Pos (const String& S) const;
    // Return the first position of the string S in the string. If S is not
    // found, the function returns -1

    String& Remove (const CharSet& Chars, unsigned Flags);
    // Remove characters from a string according to Flags.

    String& Remove (const char* Chars, unsigned Flags);
    // Remove characters from a string according to Flags.

    String& Remove (const String& Chars, unsigned Flags);
    // Remove characters from a string according to Flags.

    void Replace (unsigned Start, char C);
    // Replace the character at index Start with C. The string is expanded
    // using spaces if needed.

    void Replace (unsigned Start, const char* S);
    // Replace the characters at index Start with S. The string is expanded
    // using spaces if needed.

    void Replace (unsigned Start, const String& S);
    // Replace the characters at index Start with S. The string is expanded
    // using spaces if needed.

    int ScanR (const char C) const;
    // Return the first position of the char C in the string. If C is not
    // found, the function returns -1

    int ScanR (const char* S) const;
    // Return the first position of the string S in the string. If S is not
    // found, the function returns -1

    int ScanR (const String& S) const;
    // Return the first position of the string S in the string. If S is not
    // found, the function returns -1

    int ScanL (const char C) const;
    // Return the last position of the char C in the string. If C is not
    // found, the function returns -1

    int ScanL (const char* S) const;
    // Return the last position of the string S in the string. If S is not
    // found, the function returns -1

    int ScanL (const String& S) const;
    // Return the last position of the string S in the string. If S is not
    // found, the function returns -1

    void Set (unsigned Start, unsigned Count, char C = ' ');
    // Beginning from index Start, set Count characters to C. The string
    // is expanded if needed

    void Settle ();
    // Adjust the memory buffer of the string to the actual string length

    String& ToUpper ();
    // Uppercase the string using NLSUpper

    String& ToLower ();
    // Lowercase the string using NLSLower

    String& InputCvt ();
    // Convert the string from external character representation to the
    // internal used representation using the global function InputCvt.

    String& OutputCvt ();
    // Convert the string from internal character representation to the
    // external used representation using the global function OutputCvt.

    String& Trunc (unsigned Position = 0);
    // Truncate the string at the given position. After calling Trunc, the
    // string length is less or equal to Position.

    const char* GetStr () const;
    // Return the string as a asciiz string

    char* PSZ (char* Buf) const;
    char* PSZ (char* Buf, size_t Len) const;
    // Copy the string to the given buffer. The first form assumes, Buf is
    // big enough

    String& operator = (char C);
    String& operator = (const char* S);
    String& operator = (const String& S);
    // Copy something to a string

    char& operator [] (unsigned Index);
    const char& operator [] (unsigned Index) const;
    // Return a char from the string

    String& ShowControls (unsigned Style = ccDefault);
    // Recode the given string and replace every control character by it's
    // visible representation, e.g. "\n" instead of the character with code
    // code 13.

    String& HideControls ();
    // Recode the given string and replace every visible control character
    // representation by the character itself, e.g. replace "\n" by the
    // character with code 13.

    friend String ShowControls (const String& S, unsigned Style = ccDefault);
    // Recode the given string and replace every control character by it's
    // visible representation, e.g. "\n" instead of the character with code
    // code 13.

    friend String HideControls (const String& S);
    // Recode the given string and replace every visible control character
    // representation by the character itself, e.g. replace "\n" by the
    // character with code 13.

    // Friends
    friend String& operator += (String&, const char);
    friend String& operator += (String&, const char*);
    friend inline String& operator += (String&, const String&);
    friend String operator + (const String&, const char);
    friend String operator + (const String&, const char*);
    friend String operator + (const String&, const String&);
    friend String operator + (const char, const String&);
    friend String operator + (const char*, const String&);
    friend inline int operator == (const String&, const String&);
    friend inline int operator != (const String&, const String&);
    friend inline int operator >= (const String&, const String&);
    friend inline int operator <= (const String&, const String&);
    friend inline int operator > (const String&, const String&);
    friend inline int operator < (const String&, const String&);
    friend inline int operator == (const char*, const String&);
    friend inline int operator != (const char*, const String&);
    friend inline int operator >= (const char*, const String&);
    friend inline int operator <= (const char*, const String&);
    friend inline int operator > (const char*, const String&);
    friend inline int operator < (const char*, const String&);
    friend inline int operator == (const String&, const char*);
    friend inline int operator != (const String&, const char*);
    friend inline int operator >= (const String&, const char*);
    friend inline int operator <= (const String&, const char*);
    friend inline int operator > (const String&, const char*);
    friend inline int operator < (const String&, const char*);

    friend inline int Compare (const String& S1, const String& S2);
    // Compares two strings alphabetically

    friend int Match (const char* Source, const char* Pattern);
    friend int Match (const String& Source, const String& Pattern);
    friend int Match (const String& Source, const char* Pattern);
    friend int Match (const char* Source, const String& Pattern);
    // Match the string in Source against Pattern. Pattern may contain the
    // wildcards '*', '?', '[abcd]' '[ab-d]', '[!abcd]', '[!ab-d]'. The
    // function returns a value of zero if Source does not match Pattern,
    // otherwise a non zero value is returned. If Pattern contains an invalid
    // wildcard pattern (e.g. 'A[x'), the function returns zero.
    // NOTE: Do not use this function for matching file names - use FMatch
    // (module filepath) instead!

    friend String FormatStr (const char* S, ...);
};



// The Build-constructor has to clear Str, because Load will free an already
// allocated String. This behaviour is different to that of most other
// classes, but mimics better the builtin types.
inline String::String (StreamableInit) :
    Str (NULL)
{
}



inline String::String (const char* S, va_list ap)
{
    Init (S, ap);
}



inline String::String (const String& S, va_list ap)
{
    Init (S.Str, ap);
}



inline u16 String::Reblock (u16 Size) const
// Re-Blocks the given size to achieve a granularity != 1
{
    return u16 ((Size + 0x000F) & 0xFFF0);
}



inline String& String::Clear ()
// Set length to zero
{
    *Str = '\0';
    Length = 0;
    return *this;
}



inline int String::IsEmpty () const
// Return true if the string is empty (the length is zero)
{
    return Length == 0;
}



inline int String::NotEmpty () const
// Return true if the string is not empty (the length is greater than zero)
{
    return Length != 0;
}



inline u16 String::Len () const
// Return the length of the string
{
    return Length;
}



inline u16 String::Len (const String& S) const
// Return the length of the string not counting any chars in S
{
    return Len (S.Str);
}



inline int String::Match (const String& Pattern) const
{
    return ::Match (Str, Pattern.Str);
}



inline int String::Match (const char* Pattern) const
{
    return ::Match (Str, Pattern);
}



inline int String::Pos (const String& S) const
// Return the first position of the string S in the string. If S is not
// found, the function returns -1
{
    return Pos (S.Str);
}



inline void String::Ins (unsigned Start, const String& S)
{
    Ins (Start, S.Str);
}



inline int String::ScanR (const char C) const
// Return the first position of the char C in the string. If C is not
// found, the function returns -1
{
    return Pos (C);
}



inline int String::ScanR (const char* S) const
// Return the first position of the string S in the string. If S is not
// found, the function returns -1
{
    return Pos (S);
}



inline int String::ScanR (const String& S) const
// Return the first position of the string S in the string. If S is not
// found, the function returns -1
{
    return Pos (S);
}



inline int String::ScanL (const String& S) const
{
    return ScanL (S.Str);
}



inline void String::Settle ()
// Adjust buffer size to the memory needed
{
    Resize (Length + 1);
}



inline String& String::ToUpper ()
// Uppercase the string using NLSUpper
{
    NLSUpStr (Str);
    return *this;
}



inline String& String::ToLower ()
// Lowercase the string using NLSLower
{
    NLSLoStr (Str);
    return *this;
}



inline String& String::InputCvt ()
// Convert the string from external character representation to the
// internal used representation using the global function InputCvt.
{
    ::InputCvt (Str);
    return *this;
}



inline String& String::OutputCvt ()
// Convert the string from internal character representation to the
// external used representation using the global function OutputCvt.
{
    ::OutputCvt (Str);
    return *this;
}



inline const char* String::GetStr () const
// Return the string as a asciiz string
{
    return Str;
}



inline char* String::PSZ (char* Buf) const
// Copy the string to the given buffer.
{
    return strcpy (Buf, Str);
}



inline String& String::operator = (char C)
// Copy a char as a string of length one to a string
{
    // Str is at least 16 bytes long, so it's safe to copy
    Str [0] = C;
    Str [1] = '\0';
    Length = 1;
    return *this;
}



inline String& String::ShowControls (unsigned Style)
// Recode the given string and replace every control character by it's
// visible representation, e.g. "\n" instead of the character with code
// code 13.
{
    return (*this = ::ShowControls (*this, Style));
}



inline String& String::HideControls ()
// Recode the given string and replace every visible control character
// representation by the character itself, e.g. replace "\n" by the
// character with code 13.
{
    return (*this = ::HideControls (*this));
}



inline String & operator += (String& S1, const String& S2)
{
    return (S1 += S2.Str);
}



inline int operator == (const String& S1, const String& S2)
{
    return (strcmp (S1.Str, S2.Str) == 0);
}



inline int operator != (const String& S1, const String& S2)
{
    return (strcmp (S1.Str, S2.Str) != 0);
}



inline int operator >= (const String& S1, const String& S2)
{
    return (NLSCmpStr (S1.Str, S2.Str) >= 0);
}



inline int operator <= (const String& S1, const String& S2)
{
    return (NLSCmpStr (S1.Str, S2.Str) <= 0);
}



inline int operator > (const String& S1, const String& S2)
{
    return (NLSCmpStr (S1.Str, S2.Str) > 0);
}



inline int operator < (const String& S1, const String& S2)
{
    return (NLSCmpStr (S1.Str, S2.Str) < 0);
}



inline int operator == (const char* S1, const String& S2)
{
    return (strcmp (S1, S2.Str) == 0);
}



inline int operator != (const char* S1, const String& S2)
{
    return (strcmp (S1, S2.Str) != 0);
}



inline int operator >= (const char* S1, const String& S2)
{
    return (NLSCmpStr (S1, S2.Str) >= 0);
}



inline int operator <= (const char* S1, const String& S2)
{
    return (NLSCmpStr (S1, S2.Str) <= 0);
}



inline int operator > (const char* S1, const String& S2)
{
    return (NLSCmpStr (S1, S2.Str) > 0);
}



inline int operator < (const char* S1, const String& S2)
{
    return (NLSCmpStr (S1, S2.Str) < 0);
}



inline int operator == (const String& S1, const char* S2)
{
    return (strcmp (S1.Str, S2) == 0);
}



inline int operator != (const String& S1, const char* S2)
{
    return (strcmp (S1.Str, S2) != 0);
}



inline int operator >= (const String& S1, const char* S2)
{
    return (NLSCmpStr (S1.Str, S2) >= 0);
}



inline int operator <= (const String& S1, const char* S2)
{
    return (NLSCmpStr (S1.Str, S2) <= 0);
}



inline int operator > (const String& S1, const char* S2)
{
    return (NLSCmpStr (S1.Str, S2) > 0);
}



inline int operator < (const String& S1, const char* S2)
{
    return (NLSCmpStr (S1.Str, S2) < 0);
}



inline int Compare (const String& S1, const String& S2)
// Compares two strings alphabetically
{
    return NLSCmpStr (S1.Str, S2.Str);
}



u32 MatchKeyword (const String& ID, String Keywords);
// Match the string ID against a list of keywords given as a string. The
// list of keywords looks like this: "1^ON|0^OFF|1^YES|0^NO|", meaning:
// Return 1 if the value is "ON", return 0 if the value is "OFF" etc.
// The match is case sensitive and each of the keywords may contain
// wildcards as Match() is used to check for a match.



inline String ToUpper (String S)
// Uppercase the string using NLSUpper
{
    return S.ToUpper ();
}



inline String ToLower (String S)
// Lowercase the string using NLSLower
{
    return S.ToLower ();
}



// End of STR.H

#endif


