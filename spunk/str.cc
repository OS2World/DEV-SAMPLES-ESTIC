/*****************************************************************************/
/*                                                                           */
/*                                   STR.CC                                  */
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



#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "chartype.h"
#include "charset.h"
#include "str.h"
#include "strcvt.h"
#include "strparse.h"
#include "stream.h"
#include "streamid.h"



// Register class String
LINK (String, ID_String);



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



// Character used as escape char when calling Match
unsigned char MatchEscChar      = '\\';

// An empty string - for function returns and other stuff
extern const String EmptyString ("");



/*****************************************************************************/
/*                               class String                                */
/*****************************************************************************/



String::String (u16 Size)
{
    Limit  = Reblock (Size + 1);
    CHECK (Limit > 0);
    Str    = new char [Limit];
    *Str   = '\0';
    Length = 0;
}



void String::Init (const char* S, va_list ap)
{
    if (S == NULL || *S == '\0') {
        // S is empty
        Length = 0;
        Limit = Reblock (Length + 1);
        Str = new char [Limit];
        *Str = '\0';
    } else {
        // S is not empty, format the string
        char Buf [1024];

        Length = vsprintf (Buf, S, ap);
        CHECK (Length < sizeof (Buf));
        Limit = Reblock (Length + 1);
        Str = (char *) memcpy (new char [Limit], Buf, Length + 1);
    }
}



String::String (const char* S)
{
    if (S == NULL || *S == '\0') {
        // S is empty
        Length = 0;
        Limit  = Reblock (Length + 1);
        Str = new char [Limit];
        *Str = '\0';
    } else {
        // S is not empty
        Length = strlen (S);
        Limit = Reblock (Length + 1);
        Str = (char *) memcpy (new char [Limit], S, Length + 1);
    }
}



String::String (const String& S)
{
    Length = S.Length;
    Limit  = S.Limit;
    Str    = (char *) memcpy (new char [Limit], S.Str, Length + 1);
}



String::~String ()
{
    delete [] Str;
}



void String::Resize (u16 NewLimit)
{
    NewLimit = Reblock (NewLimit);
    if (NewLimit == Limit) {
        // Nothing has changed...
        return;
    }

    char *P = new char [NewLimit];
    if (Length < NewLimit) {
        // String fits, safe to copy
        memcpy (P, Str, Length + 1);
    } else {
        // New string is shorter
        memcpy (P, Str, NewLimit - 1);
        P [NewLimit] = '\0';
        Length = NewLimit - 1;
    }
    Limit = NewLimit;
    delete [] Str;
    Str = P;
}



void String::Load (Stream &S)
{
    S >> Length;
    delete [] Str;              // Use this to mimic behaviour of builtin types
    Limit = Reblock (Length + 1);
    Str = new char [Limit];
    S.Read (Str, Length + 1);
}



void String::Store (Stream& S) const
{
    S << Length;
    S.Write (Str, Length + 1);
}



u16 String::StreamableID () const
{
    return ID_String;
}



Streamable *String::Build ()
{
    return new String (Empty);
}



void String::Crypt ()
// crypt (make unreadable) by a simple XOR scheme
{
    for (int I = 0; I < Length; I++) {
        Str [I] ^= I+10;
    }
}



void String::Decrypt ()
// Reverses the action done by Crypt
{
    Crypt ();
}



u16 String::Len (const char *S) const
// Return the length of the string not counting chars in S
{
    unsigned Count = 0;
    char *P = Str;

    if (strlen (S) == 1) {
        // One char only
        char C = S [0];
        while (*P != '\0') {
            if (*P != C) {
                Count++;
            }
            P++;
        }
    } else {
        unsigned Span;
        while (*P != '\0') {
            Span = strcspn (P, S);
            P += Span;
            Count += Span;
            while (*P != '\0') {
                if (strchr (S, *P)) {
                    P++;                // Char is in S, skip
                } else {
                    break;              // Char is not in S, break
                }
            }
        }
    }

    return Count;
}



int String::Pos (const char C) const
// Return the first position of the char C in the string. If C is not
// found, the function returns -1
{
    char *P = strchr (Str, C);
    return P ? P - Str : -1;
}



int String::Pos (const char *S) const
// Return the first position of the string S in the string. If S is not
// found, the function returns -1
{
    char *P = strstr (Str, S);
    return P ? P - Str : -1;
}



String& String::Remove (const CharSet& Chars, unsigned Flags)
// Remove characters from a string according to Flags.
{
    char C;
    unsigned Start = 0;         // Initialize to make gcc happy
    unsigned Stop = 0;          // Initialize to make gcc happy

    // Count the leading characters. This is only needed if we must delete
    // trailing or interword characters
    if (Flags & (rmLeading | rmInterWord)) {

        Start = 0;
        const char* S = Str;
        while ((C = *S++) != '\0' && Chars [C] != 0) {
            Start++;
        }

        // Remove leading characters if needed
        if (Flags & rmLeading) {
            // Do the delete
            Del (0, Start);

            // Reset the pointer to the first character not in chars
            Start = 0;
        }
    }

    // Count trailing characters. This is only needed if we must delete
    // trailing or interword characters.
    if (Flags & (rmTrailing | rmInterWord)) {

        Stop = Length;

        // Find the last char
        while (Stop > 0 && Chars [Str [Stop-1]] != 0) {
            Stop--;
        }

        // Remove trailing characters if needed
        if (Flags & rmTrailing) {
            Trunc (Stop);
        }
    }

    // Start now points to the first character that is not in Chars, Stop
    // holds the character *after* the last character not in Chars. The
    // next step is only needed if we must delete characters inside.
    if (Flags & rmInterWord) {

        // Run from Start to stop, deleting all the characters between
        while (Start < Stop) {
            if (Chars [Str [Start]] != 0) {
                // Got one to delete
                Del (Start);
                Stop--;
            } else {
                // Next one
                Start++;
            }
        }

    }

    // Return a reference to the string
    return *this;
}



String& String::Remove (const char* Chars, unsigned Flags)
// Remove characters from a string according to Flags.
{
    return Remove (CharSet (Chars), Flags);
}



String& String::Remove (const String& Chars, unsigned Flags)
// Remove characters from a string according to Flags.
{
    return Remove (CharSet (Chars), Flags);
}



void String::Replace (unsigned Start, char C)
// Replace the character at index Start with C. The string is expanded
// using spaces if needed.
{
    // Things are simple if the string is long enough
    if (Start < Length) {
        Str [Start] = C;
        return;
    }

    // Must expand string
    Resize (Start + 2);
    memset (Str + Length, ' ', Start - Length);
    Str [Start] = C;
    Length = Start+1;
    Str [Length] = '\0';
}



void String::Replace (unsigned Start, const char* S)
// Replace the characters at index Start with S. The string is expanded
// using spaces if needed.
{
    // Get the length of the string to replace
    unsigned SLen = strlen (S);

    // Check if the string is long enough
    if (Length >= Start + SLen) {
        // Long enough, just copy
        memmove (Str + Start, S, SLen);
        return;
    }

    // Must expand string
    Resize (Start + SLen + 1);
    memset (Str + Length, ' ', Start - Length);
    memmove (Str + Start, S, SLen);
    Length = Start + SLen;
    Str [Length] = '\0';
}



void String::Replace (unsigned Start, const String& S)
// Replace the characters at index Start with S. The string is expanded
// using spaces if needed.
{
    // Check if the string is long enough
    if (Length >= Start + S.Length) {
        // Long enough, just copy
        memmove (Str + Start, S.Str, S.Length);
        return;
    }

    // Must expand string
    Resize (Start + S.Length + 1);
    memset (Str + Length, ' ', Start - Length);
    memmove (Str + Start, S.Str, S.Length);
    Length = Start + S.Length;
    Str [Length] = '\0';
}



String& String::Add (const char* S, unsigned Len, PadType P)
// Build lines of formatted text: If Len is zero, add S to the end of
// this. If Len is non zero, use P to pad S to the given length, then
// add it to the end of P.
{
    return Add (String (S), Len, P);
}



String& String::Add (const String& S, unsigned Len, PadType P)
// Build lines of formatted text: If Len is zero, add S to the end of
// this. If Len is non zero, use P to pad S to the given length, then
// add it to the end of P.
{
    if (Len == 0) {
        return *this += S;
    } else {
        String F = S;
        return *this += F.Pad (P, Len);
    }
}



String String::Cut (unsigned Start, unsigned Count) const
{
    // Bail out if count is zero
    if (Count == 0 || Start >= Length) {
        return String ();
    }

    // Calculate the character count to copy
    if ((Start + Count) > Length) {
        Count = Length - Start;
    }

    // Create a new empty string
    String S (Empty);
    S.Length = Count;
    S.Limit  = Reblock (Count + 1);
    S.Str    = new char [S.Limit];

    // just copy the characters as the buffer is big enough
    memcpy (S.Str, &Str [Start], Count);

    // Add the trailing zero
    S.Str [Count] = '\0';

    // That's it
    return S;
}



void String::ForceLen (int NewLen, PadType P)
// Force the string to a new length. If the string is too long, it is
// replaced with a string of '!'s of length NewLen. If the string is too
// short, it is padded to the given length using the padtype P.
{
    if (NewLen > Length) {
        // Needs padding
        Pad (P, NewLen);
    } else if (NewLen < Length) {
        // String too long
        Set (0, NewLen, '!');
        Str [NewLen] = '\0';
    }
}



void String::Del (unsigned Start, unsigned Count)
{
    if (Start >= Length) {
        return;
    }
    if ((Start + Count) > Length) {
        Count = Length - Start;
    }
    if (Count == 0) {
        return;
    }

    // Now use memmove
    memmove (&Str [Start], &Str [Start+Count], Length - Start - Count + 1);

    // Length of the string has changed
    Length -= Count;
}



void String::Ins (unsigned Start, char C)
// Insert a character into a string
{
    Resize (Length + 2);
    memmove (&Str [Start+1], &Str [Start], Length - Start + 1);
    Str [Start] = C;
    Length += 1;
}



void String::Ins (unsigned Start, const char *S)
{
    unsigned Count = strlen (S);

    Resize (Length + Count + 1);
    memmove (&Str [Start+Count], &Str [Start], Length - Start + 1);
    memcpy (&Str [Start], S, Count);
    Length += Count;
}



// Utility macro used in RMatch
#define IncPattern()    Pattern++;                      \
                        if (*Pattern == '\0') {         \
                            return 0;                   \
                        }



static int RealChar (const unsigned char* Pattern)
// Return the next character from Pattern. If the next character is the
// escape character, skip it and return the following.
{
    if (*Pattern == MatchEscChar) {
        Pattern++;
        return (*Pattern == '\0') ? -1 : *Pattern;
    } else {
        return *Pattern;
    }
}



static int RMatch (const unsigned char* Source, const unsigned char* Pattern)
// A recursive pattern matcher
{

    CharSet CS;

    while (1) {

        if (*Pattern == '\0') {

            // Reached the end of Pattern, what about Source?
            return (*Source == '\0') ? 1 : 0;

        } else if (*Pattern == '*') {

            Pattern++;

            if (*Pattern == '\0') {
                // a trailing '*' is always a match
                return 1;
            }

            // Check the rest of the string
            while (*Source) {
                if (RMatch (Source++, Pattern)) {
                    // match!
                    return 1;
                }
            }

            // No match...
            return 0;

        } else if (*Source == '\0') {

            // End of Source reached, no match
            return 0;

        } else {

            // Check a single char. Build a set of all possible characters in
            // CS, then check if the current char of Source is contained in
            // there.
            CS.Clear ();                // Clear the character set

            if (*Pattern == '?') {

                // All chars are allowed
                CS.SetAll ();
                Pattern++;                      // skip '?'

            } else if (*Pattern == MatchEscChar) {

                // use the next char without looking at it
                IncPattern ();
                CS += *Pattern;
                Pattern++;                      // skip the character

            } else if (*Pattern == '[') {

                // a set follows
                IncPattern ();
                int Invert = 0;
                if (*Pattern == '!') {
                    IncPattern ();
                    Invert = 1;
                }
                while (*Pattern != ']') {

                    int C1;
                    if ((C1 = RealChar (Pattern)) == -1) {
                        return 0;
                    }
                    IncPattern ();
                    if (*Pattern != '-') {
                        CS += C1;
                    } else {
                        IncPattern ();
                        int C2;
                        if ((C2 = RealChar (Pattern)) == -1) {
                            return 0;
                        }
                        IncPattern ();
                        for (unsigned char C = C1; C <= C2; C++) {
                            CS += C;
                        }
                    }
                }
                Pattern++;      // skip ']'
                if (Invert) {
                    CS.Reverse ();
                }

            } else {

                // Include the char in the charset
                CS += *Pattern;
                Pattern++;

            }

            if (CS [*Source] == 0) {
                // No match
                return 0;
            }
            Source++;
        }
    }

}




int Match (const char* Source, const char* Pattern)
// Match the string in Source against Pattern. Pattern may contain the
// wildcards '*', '?', '[abcd]' '[ab-d]', '[!abcd]', '[!ab-d]'. The
// function returns a value of zero if Source does not match Pattern,
// otherwise a non zero value is returned. If Pattern contains an invalid
// wildcard pattern (e.g. 'A[x'), the function returns zero.
{
    if (Pattern == NULL || *Pattern == '\0') {
        if (Source == NULL || *Source == '\0') {
            return 1;
        } else {
            return 0;
        }
    }

    // Do the real thing
    return RMatch ((unsigned char *) Source, (unsigned char *) Pattern);
}



int Match (const String& Source, const String& Pattern)
{
    return Match (Source.Str, Pattern.Str);
}



int Match (const String& Source, const char* Pattern)
{
    return Match (Source.Str, Pattern);
}



int Match (const char* Source, const String& Pattern)
{
    return Match (Source, Pattern.Str);
}



static String GetStr (String& ValStr, char C)
// Helper function for MatchKeyword. Extract from ValStr the string delimited
// by C. Delete this string (including C) from ValStr and return it.
// Call FAIL if ValStr is invalid
{
    // Get the position of the delimiter and make shure, it's there
    int DelimPos = ValStr.Pos (C);
    CHECK (DelimPos > 0);

    // Get the partial string
    String Val = ValStr.Cut (0, DelimPos);

    // Delete the partial string
    ValStr.Del (0, DelimPos + 1);

    // Return the extracted string
    return Val;
}



static u32 GetNumber (String& ValStr)
// Helper function for MatchKeyword. Extract the numeric value from ValStr and
// delete this number including the following '^'. Call FAIL if ValStr is
// invalid
{
    // Get the number
    String Val = GetStr (ValStr, '^');

    // Set up the string parser
    StringParser SP (Val, StringParser::UseAll | StringParser::PascalHex |
                          StringParser::CHex);

    // Get the value
    u32 Value;
    CHECK (SP.GetU32 (Value) == 0);

    // return the value
    return Value;
}



u32 MatchKeyword (const String& ID, String Keywords)
// Match the string ID against a list of keywords given as a string. The list
// of keywords looks like this: "1^ON|0^OFF|1^YES|0^NO|", meaning:
// Return 1 if the value is "ON", return 0 if the value is "OFF" etc.
// The match is case sensitive and each of the keywords may contain
// wildcards as Match() is used to check for a match.
{
    // Get the default value (this is the first number in Keywords)
    int HaveDefault = 0;
    u32 Default = 0;            // Initialize to make gcc happy

    // If ID is empty, we are already done
    if (ID.IsEmpty ()) {
        // Return the default
        return GetNumber (Keywords);
    }

    // Repeat until Keywords is gone..
    do {

        // Get the next numeric value
        u32 NumVal = GetNumber (Keywords);

        // Use this value as default value if we don't have one until now
        if (!HaveDefault) {
            HaveDefault = 1;
            Default = NumVal;
        }

        // Compare the next value
        if (Match (ID, GetStr (Keywords, '|'))) {
            // Found the value
            return NumVal;
        }

    } while (!Keywords.IsEmpty ());

    // Not found
    return Default;
}



u32 String::MatchKeyword (const String& Keywords) const
// Match the string against a list of keywords given as a string. The list
// of keywords looks like this: "1^ON|0^OFF|1^YES|0^NO|", meaning:
// Return 1 if the value is "ON", return 0 if the value is "OFF" etc.
// The match is case sensitive and each of the keywords may contain
// wildcards as Match() is used to check for a match.
{
    return ::MatchKeyword (*this, Keywords);
}



u32 String::MatchKeyword (const char* Keywords) const
// Match the string against a list of keywords given as a string. The list
// of keywords looks like this: "1^ON|0^OFF|1^YES|0^NO|", meaning:
// Return 1 if the value is "ON", return 0 if the value is "OFF" etc.
// The match is case sensitive and each of the keywords may contain
// wildcards as Match() is used to check for a match.
{
    return ::MatchKeyword (*this, Keywords);
}



String& String::Pad (PadType P, u16 NewLen, char C)
// Pad the string to the given length using the padding char C. If the
// string is too long, it is simply cut of at the end.
{
    if (Length > NewLen) {
        // String is too long, nothing to pad
        Trunc (NewLen);
        return *this;
    } else if (Length == NewLen) {
        // Nothing to do
        return *this;
    }

    // Allocate memory for the new string
    u16 NewLimit = Reblock (NewLen + 1);
    char *S = new char [NewLimit];

    // Initialize the following variables to prevent gcc warnings
    u16 LeftPad  = 0;
    u16 RightPad = 0;

    switch (P) {

        case Left:
            LeftPad = NewLen - Length;
            RightPad = 0;
            break;

        case Center:
            RightPad = NewLen - Length;
            LeftPad = RightPad / 2;
            RightPad -= LeftPad;
            break;

        case Right:
            LeftPad = 0;
            RightPad = NewLen - Length;
            break;

    }

    memset (S, C, LeftPad);
    memcpy (&S [LeftPad], Str, Length);
    memset (&S [LeftPad + Length], C, RightPad);
    S [NewLen] = '\0';

    // Now delete the old string data end store the new
    delete [] Str;
    Str = S;
    Limit = NewLimit;
    Length += LeftPad + RightPad;

    // Return a reference to this
    return *this;
}



int String::ScanL (const char C) const
// Return the last position of the char C in the string. If C is not
// found, the function returns -1
{
    char *P = strrchr (Str, C);
    return P ? P - Str : -1;
}



int String::ScanL (const char* S) const
// Return the last position of the string S in the string. If S is not
// found, the function returns -1
{
    char *P = Str;
    int Pos = -1;

    // Some optimization would be nice here, but it works ...
    while (1) {
        P = strstr (P, S);
        if (P == NULL) {
            // Not found, return last occurence
            return Pos;
        } else {
            // Found, remember this occurence, skip one char
            Pos = P - Str;
            P++;
        }
    }
}



void String::Set (unsigned Start, unsigned Count, char C)
// Beginning from index Start, set Count characters to C. The string
// is expanded if needed
{
    if ((Start + Count + 1) > Limit) {
        Resize (Start + Count + 1);
    }
    memset (&Str [Start], C, Count);
    if ((Start + Count) > Length) {
        // Length has changed, add trailing zero, correct length
        Str [Start+Count] = '\0';
        Length = Start + Count;
    }
}



String& String::Trunc (unsigned Position)
// Truncate the string at the given position. After calling Trunc, the
// string length is less or equal to Position.
{
    // Ignore the cut if the string is shorter
    if (Position < Length) {
        Str [Position] = '\0';
        Length = Position;
    }
    return *this;
}



char* String::PSZ (char* Buf, size_t BufLen) const
// Copy the string to the given buffer.
{
    if (Length < BufLen) {
        return strcpy (Buf, Str);
    } else {
        // Must truncate
        memcpy (Buf, Str, BufLen-1);
        Buf [BufLen-1] = '\0';
        return Buf;
    }
}



String& String::operator = (const char* S)
{
    if (S && *S != '\0') {
        // S is not empty. Check if there is enough room to copy
        Length = strlen (S);
        if (Length+1 <= Limit) {
            // Room enough, just copy
            memcpy (Str, S, Length+1);
        } else {
            // Need more memory
            delete [] Str;
            Limit = Reblock (Length+1);
            Str = (char *) memcpy (new char [Limit], S, Length + 1);
        }
    } else {
        // S is empty, do it the simple way
        *Str = '\0';
    }
    return *this;
}



String& String::operator = (const String& S)
{
    if (&S != this) {                   // beware of S = S;
        delete [] Str;
        Limit  = S.Limit;
        Length = S.Length;
        Str    = (char *) memcpy (new char [Limit], S.Str, Length + 1);
    }
    return *this;
}



char& String::operator [] (unsigned Index)
{
    // Make shure, Index is valid
    PRECONDITION (Index <= Length);
    return Str [Index];
}



const char& String::operator [] (unsigned Index) const
{
    // Check the index against the string length
    PRECONDITION (Index <= Length);
    return Str [Index];
}



String& operator += (String& S, const char C)
{
    S.Resize (S.Length + 2);
    S.Str [S.Length] = C;
    S.Length++;
    S.Str [S.Length] = '\0';
    return S;
}



String& operator += (String& S, const char* P)
{
    u16 PLen = strlen (P);
    S.Resize (S.Length + PLen + 1);
    memcpy (&S.Str [S.Length], P, PLen  + 1);
    S.Length += PLen;
    return S;
}



String operator + (const String& S, const char C)
{
    String Ret = S;
    Ret += C;
    return Ret;
}



String operator + (const String& S, const char* P)
{
    // Create an emtpy string to avoid calling operator new
    String Res (Empty);

    // Build the combined string
    u16 PLen = strlen (P);
    Res.Length = S.Length + PLen;
    Res.Limit  = Res.Reblock (Res.Length + 1);
    Res.Str    = new char [Res.Limit];
    memcpy (Res.Str, S.Str, S.Length);
    memcpy (&Res.Str [S.Length], P, PLen + 1);

    // ... and return it
    return Res;
}



String operator + (const String& S1, const String& S2)
{
    // Create an emtpy string to avoid calling operator new
    String Res (Empty);

    // Build the combined string
    Res.Length = S1.Length + S2.Length;
    Res.Limit  = Res.Reblock (Res.Length + 1);
    Res.Str    = new char [Res.Limit];
    memcpy (Res.Str, S1.Str, S1.Length);
    memcpy (&Res.Str [S1.Length], S2.Str, S2.Length + 1);

    // ... and return it
    return Res;
}



String operator + (const char* P, const String& S2)
{
    // Create empty object to avoid call to operator new
    String Res (Empty);

    u16 PLen   = strlen (P);
    Res.Length = PLen + S2.Length;
    Res.Limit  = Res.Reblock (Res.Length + 1);
    Res.Str    = new char [Res.Limit];
    memcpy (Res.Str, P, PLen);
    memcpy (&Res.Str [PLen], S2.Str, S2.Length + 1);

    return Res;
}



String operator + (const char C, const String& S)
{
    // Create empty object to avoid call to operator new
    String Res (Empty);

    Res.Length  = S.Length + 1;
    Res.Limit   = Res.Reblock (Res.Length + 1);
    Res.Str     = new char [Res.Limit];
    Res.Str [0] = C;
    memcpy (&Res.Str [1], S.Str, S.Length + 1);

    return Res;
}



String ShowControls (const String& S, unsigned Style)
// Recode the given string and replace every control character by it's
// visible representation, e.g. "\n" instead of the character with code
// code 13.
{
    // Numeric codes in hex
    static const char HexCodes [32][5] = {
        "\\x00", "\\x01", "\\x02", "\\x03", "\\x04", "\\x05", "\\x06", "\\x07",
        "\\x08", "\\x09", "\\x0A", "\\x0B", "\\x0C", "\\x0D", "\\x0E", "\\x0F",
        "\\x10", "\\x11", "\\x12", "\\x13", "\\x14", "\\x15", "\\x16", "\\x17",
        "\\x18", "\\x19", "\\x1A", "\\x1B", "\\x1C", "\\x1D", "\\x1E", "\\x1F"
    };

    // Numeric codes in octal
    static const char OctCodes [32][5] = {
        "\\000", "\\001", "\\002", "\\003", "\\004", "\\005", "\\006", "\\007",
        "\\010", "\\011", "\\012", "\\013", "\\014", "\\015", "\\016", "\\017",
        "\\020", "\\021", "\\022", "\\023", "\\024", "\\025", "\\026", "\\027",
        "\\030", "\\031", "\\032", "\\033", "\\034", "\\035", "\\036", "\\037",
    };

    // Create a new string with a length of 110% of the string to recode.
    // This is just an assumption of the probable string growth and
    // is thought to prevent too many string resizes (each one calls
    // operator new).
    String T ((S.Len () * 11) / 10);

    // When recoding, access S directly to avoid the range check code
    // that would slow things down and is not needed here
    char* Str = S.Str;
    while (*Str) {

        switch (*Str) {

            case 0x01:
                // This is the special spunk char '\c'
                if (Style & ccSpunk) {
                    T += "\\c";
                } else {
                    T += (Style & ccOct)? OctCodes [*Str] : HexCodes [*Str];
                }
                break;

            case '\\':
                if (Style & ccCStyle) {
                    T += "\\\\";
                } else {
                    T += (Style & ccOct)? OctCodes [*Str] : HexCodes [*Str];
                }
                break;

            case '\a':
                if (Style & ccCStyle) {
                    T += "\\a";
                } else {
                    T += (Style & ccOct)? OctCodes [*Str] : HexCodes [*Str];
                }
                break;

            case '\b':
                if (Style & ccCStyle) {
                    T += "\\b";
                } else {
                    T += (Style & ccOct)? OctCodes [*Str] : HexCodes [*Str];
                }
                break;

            case '\f':
                if (Style & ccCStyle) {
                    T += "\\f";
                } else {
                    T += (Style & ccOct)? OctCodes [*Str] : HexCodes [*Str];
                }
                break;

            case '\n':
                if (Style & ccCStyle) {
                    T += "\\n";
                } else {
                    T += (Style & ccOct)? OctCodes [*Str] : HexCodes [*Str];
                }
                break;

            case '\t':
                if (Style & ccCStyle) {
                    T += "\\t";
                } else {
                    T += (Style & ccOct)? OctCodes [*Str] : HexCodes [*Str];
                }
                break;

            case '\v':
                if (Style & ccCStyle) {
                    T += "\\v";
                } else {
                    T += (Style & ccOct)? OctCodes [*Str] : HexCodes [*Str];
                }
                break;

            default:
                if (!IsCntrl (*Str)) {
                    // No control char, just add the char itself
                    T += *Str;
                } else {
                    // Another control char
                    T += (Style & ccOct)? OctCodes [*Str] : HexCodes [*Str];
                }
                break;
        }

        // Next char
        Str++;
    }

    // Done, return the result
    return T;
}



static unsigned Num (char C)
// Return the numeric representation of the character C (assumes ASCII)
{
    if (C >= 'a') {
        return C - 'a' + 10;
    } else if (C >= 'A') {
        return C - 'A' + 10;
    } else {
        return C - '0';
    }
}



String HideControls (const String& S)
// Recode the given string and replace every visible control character
// representation by the character itself, e.g. replace "\n" by the
// character with code 13.
{
    // Create the target string with the same length as the source string.
    // The string will definitely not grow when recoding, so there is no
    // further call to operator new needed.
    String T (S.Len ());

    // Access the source string directly to avoid the range check code
    // that is not needed here.
    char* Str = S.Str;

    // Recode the string
    while (*Str) {

        if (*Str == '\\') {
            // A control char. Check it.
            Str++;
            switch (*Str) {

                case '\0':
                    // Ohh help! The string ends with an open sequence -
                    // just copy the start of the sequence. Then remember
                    // to decrease S 1 char back to match the end condition.
                    T += '\\';
                    Str--;
                    break;

                case '\\':
                    T += '\\';
                    break;

                case 'a':
                    T += '\a';
                    break;

                case 'b':
                    T += '\b';
                    break;

                case 'c':
                    // This an invalid sequence in c but used as the spunk
                    // "center string" mark.
                    T += char (0x01);
                    break;

                case 'f':
                    T += '\f';
                    break;

                case 'n':
                    T += '\n';
                    break;

                case 't':
                    T += '\t';
                    break;

                case 'v':
                    T += '\v';
                    break;

                case 'x':
                case 'X':
                    // Character code in hex follows. Check if there is really
                    // a character code.
                    if (IsXDigit (Str [1]) && IsXDigit (Str [2])) {
                        T += char (Num (Str [1]) * 16 + Num (Str [2]));
                        Str += 2;
                    } else {
                        // Something is wrong. Copy the '\x', which is the
                        // only part, we know it is correct, and let the
                        // next round determine what to do with the remaining
                        // chars;
                        T += "\\x";
                    }
                    break;

                default:
                    // This is an unknown control char. Swallow it.
                    break;

            }

        } else {

            // Some other char, just copy it
            T += *Str;

        }

        // Next char
        Str++;

    }

    // Done, return the result
    return T;
}



String FormatStr (const char* S, ...)
// Return a via sprintf formatted string
{
    va_list ap;
    va_start (ap, S);
#if defined(DOS) || defined (DOS32) || defined (OS2)
    // Does not call va_end but works on DOS/OS2 and does not need an extra copy
    return String (S, ap);
#else
    // Portable version
    String Res (S, ap);
    va_end (ap);
    return Res;
#endif
}



