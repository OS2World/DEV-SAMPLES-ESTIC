/*****************************************************************************/
/*									     */
/*				  STRPARSE.H				     */
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



#ifndef _STRPARSE_H
#define _STRPARSE_H



#include "chartype.h"
#include "str.h"
#include "charset.h"
#include "datetime.h"



/*****************************************************************************/
/*			 Constants for parsing results			     */
/*****************************************************************************/



const unsigned prOk		= 0x0000;	// Conversion done
const unsigned prTooLarge	= 0x0001;	// Value too large for data type
const unsigned prTooSmall	= 0x0002;	// Value to small for data type
const unsigned prSyntax		= 0x0003;	// Syntax error
const unsigned prMaxError	= 0x0003;



/*****************************************************************************/
/*			      class StringParser			     */
/*****************************************************************************/



class StringParser: public Object {

public:
    enum {
	SkipWS	      = 0x0001,		// Skip white space before parsing
	UseAll	      = 0x0002,		// Error if no EOS is reached
	PascalHex     = 0x0004,		// Allow pascal style hex values
	CHex	      = 0x0008,		// Allow C style hex values
	COct	      = 0x0010,		// Allow C style octal digits
	AllowDP       = 0x0020		// Allow real decimal point ('.')
    };


protected:
    int			P;		// Position in String
    const String&	S;
    u16			Flags;		// Parsing flags
    CharSet		WhiteSpace;	// White space characters

private:
    int AllUsed ();
    // Return true if UseAll is not set or if EOS is reached

    static int Num (char C);
    // Calculate the numeric value of the digit C (assumes ASCII)

public:
    StringParser (const String& X, u16 ParseFlags = StringParser::UseAll);

    void Reset ();
    // Reset position to 0

    int GetPos () const;
    // Get current position in string

    void SetPos (int NewPos);
    // Set a new position for parsing

    void SkipWhite ();
    // Skip white space

    void Skip (const String& Chars);
    // Skip any chars contained in Chars

    void Skip (const CharSet& Chars);
    // Skip any chars contained in Chars

    int EOS () const;
    // Return 1 if the end of the string is reached

    const String& GetMsg (unsigned Res);
    // Return an apropriate error message for the result Res (!= 0)

    void SetFlags (u16 F);
    void ResetFlags (u16 F);
    u16 GetFlags ();
    // Handling the flags

    // Extracting data
    unsigned GetI32 (i32& Val, unsigned Base = 10);
    unsigned GetU32 (u32& Val, unsigned Base = 10);
    unsigned GetFloat (double& Val);
    unsigned GetTime (Time& Val);
    unsigned GetDate (Time& Val);

    unsigned GetToken (String& Tok, const CharSet& Chars = ::WhiteSpace);
    // Return token from the string that is separated by characters contained
    // in Chars

    unsigned GetString (String& S);
    // Return a string enclosed in double quotes

};



inline void StringParser::Reset ()
// Reset position to 0
{
    P = 0;
}



inline int StringParser::GetPos () const
// Get current position in string
{
    return P;
}



inline int StringParser::EOS () const
// Return 1 if the end of the string is reached
{
    return S [P] == '\0';
}



inline void StringParser::SkipWhite ()
// Skip white space
{
    Skip (WhiteSpace);
}



inline void StringParser::SetFlags (u16 F)
{
    Flags |= F;
}



inline void StringParser::ResetFlags (u16 F)
{
    Flags &= ~F;
}



inline u16 StringParser::GetFlags ()
{
    return Flags;
}



// End of STRPARSE.H

#endif
