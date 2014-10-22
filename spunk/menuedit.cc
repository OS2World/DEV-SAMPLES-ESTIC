/*****************************************************************************/
/*									     */
/*				  MENUEDIT.CC				     */
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



#include "filesys.h"
#include "filepath.h"
#include "menuedit.h"
#include "winattr.h"
#include "stdmsg.h"
#include "progutil.h"
#include "national.h"
#include "chartype.h"
#include "streamid.h"
#include "strparse.h"
#include "strcvt.h"
#include "msgid.h"



// Register the classes
LINK (EditLine, ID_EditLine);
LINK (FloatEdit, ID_FloatEdit);
LINK (LongEdit, ID_LongEdit);
LINK (HexEdit, ID_HexEdit);
LINK (TextEdit, ID_TextEdit);
LINK (TimeEdit, ID_TimeEdit);
LINK (DateEdit, ID_DateEdit);
LINK (PasswordEdit, ID_PasswordEdit);
LINK (FileEdit, ID_FileEdit);



/*****************************************************************************/
/*			       Message constants			     */
/*****************************************************************************/



static const u16 msErrIllegalInput	= MSGBASE_MENUEDIT +  0;
static const u16 msErrTrailingDigits	= MSGBASE_MENUEDIT +  1;
static const u16 msErrFTooSmall		= MSGBASE_MENUEDIT +  2;
static const u16 msErrFTooLarge		= MSGBASE_MENUEDIT +  3;
static const u16 msErrLTooSmall		= MSGBASE_MENUEDIT +  4;
static const u16 msErrLTooLarge		= MSGBASE_MENUEDIT +  5;
static const u16 msErrNoEmptyInput	= MSGBASE_MENUEDIT +  6;
static const u16 msErrNoExtension	= MSGBASE_MENUEDIT +  7;



/*****************************************************************************/
/*				class EditLine				     */
/*****************************************************************************/



EditLine::EditLine (const String& aItemText, i16 aID, u16 aMaxLen,
		    u16 aFieldLen, WindowItem* NextItem) :
    WindowItem (aItemText, aID, NextItem),
    Line (aMaxLen),
    FieldLen (aFieldLen),
    InsertMode (0),
    MaxLen (aMaxLen),
    InsertPos (0),
    FirstChar (0),
    CursorX (0),
    FirstPos (0)
{
    // Check the given length
    PRECONDITION (MaxLen >= 2);

    // Set the width to MinWidth. This function will calculate all
    // values. Warning: This may fail because every time
    // EditLine::MinWidth is called (we are inside the constructor!).
    // If this is a problem, think about a better solution.
    SetWidth (MinWidth ());
}



int EditLine::CharIsOk (Key& K)
// This function is called with the users input to check if the
// input is valid. The function returns 1 if the key is acceptable,
// 0 otherwise. Overload this function for restricting input.
// The function can change the given char!
{
    // Disallow extended keys
    return IsPlainKey (K);
}



int EditLine::EndCheckOk ()
// This function is called when the user has ended editing. It returns
// 1 if the contents of Line are acceptable, 0 if not. In the latter
// case it's up to this function to display an appropriate error
// message.
{
    // No check in EditLine::EndCheckOk
    return 1;
}



void EditLine::DrawLeftArrow (int Arrow)
// Draws or clears the left arrow according to the given flag
{
    char C = Arrow ? LeftTriangle : ' ';
    Owner->Write (LeftArrowX, ItemY, C, atEditHigh);
}



void EditLine::DrawRightArrow (int Arrow)
// Draws or clears the right arrow according to the given flag
{
    char C = Arrow ? RightTriangle : ' ';
    Owner->Write (RightArrowX, ItemY, C, atEditHigh);
}



void EditLine::DrawField ()
// This function is called from Edit and draws the editing field.
// FirstPos is the position (in Line) of the first char that is
// printed. CursorX is the position of the cursor in the window.
// First is true if the user has not pressed any key. In this case,
// a special attribuite for the text is used, because the first
// keypress of a non cursor key will clear the line.
{
    // Get the visible portion of the input line
    String S (Line.Cut (FirstPos, FieldLen));
    S.Pad (String::Right, FieldLen);

    // Draw the line
    Owner->Write (FieldX, ItemY, S, FirstChar ? atEditHigh : atEditNormal);

    // Draw/delete both arrows
    DrawLeftArrow (FirstPos != 0);
    DrawRightArrow ((FirstPos + FieldLen) < Line.Len ());

    // Reset the cursor
    Owner->SetCursorPos (Point (CursorX, ItemY));
}



void EditLine::Store (Stream& S) const
// Store an EditLine into a stream
{
    WindowItem::Store (S);
    S << Line << FieldLen << FieldX << LeftArrowX << RightArrowX;
    S << InsertMode << MaxLen;
}



void EditLine::Load (Stream& S)
// Load an EditLine from a stream
{
    WindowItem::Load (S);
    S >> Line >> FieldLen >> FieldX >> LeftArrowX >> RightArrowX;
    S >> InsertMode >> MaxLen;

    // Set the working variables
    InsertPos = 0;
    FirstChar = 0;
    CursorX   = 0;
    FirstPos  = 0;
}



u16 EditLine::StreamableID () const
{
    return ID_EditLine;
}



Streamable *EditLine::Build ()
{
    return new EditLine (Empty);
}



void EditLine::SetWidth (u16 NewWidth)
{
    // Get the minimum acceptable width
    u16 Min = MinWidth ();

    // Bail out if the new width is invalid
    if (NewWidth < Min) {
	return;
    }

    // If the length of ItemText is zero, the only allowed width is MinWidth
    // Because SetWidth is also used to set the other variables, we cannot
    // ignore such an request
    if (ItemText.Len () == 0 && NewWidth > Min) {
	NewWidth = Min;
    }

    // Calculate the differnce between the min width and the new width
    u16 Fill = NewWidth - Min;

    // Set up the positions
    u16 Len = ItemText.Len ();
    FieldX = ItemX + 1 + Fill;
    if (Len > 0) {
	FieldX += Len + 2;
    }
    LeftArrowX	= FieldX - 1;
    RightArrowX = FieldX + FieldLen;

    // Remember new value
    ItemWidth = NewWidth;
}



void EditLine::SetPos (u16 X, u16 Y)
{
    ItemX = X;
    ItemY = Y;

    // Use SetWidth to recalculate all data
    SetWidth (ItemWidth);
}



void EditLine::Draw ()
{
    // Draw WindowItem
    WindowItem::Draw ();

    // The rest is done by DrawField
    DrawField ();
}



u16 EditLine::MinWidth ()
{
    u16 Width = ItemText.Len ();
    if (Width > 0) {
	Width += 2;		// One space left and right
    }
    return Width + 2 + FieldLen;
}



void EditLine::SetValue (const String& NewVal)
// Set a new value for Line
{
    // Store the new string
    Line = NewVal;

    // Truncate the given String if it is too long
    if (Line.Len () > MaxLen) {
	Line.ForceLen (MaxLen);
    }

    // Redraw the item
    Draw ();
}



void EditLine::DrawIfFirstChar ()
// Is called to redraw the item if nothing has changed but the item
// has to be redrawn (e.g. because the the state of FirstChar has
// changed).
{
    if (FirstChar) {
	FirstChar = 0;
	DrawField ();
    }
}



void EditLine::SetCursorForm ()
// Sets the cursor shape according to the state of InsertMode
{
    if (InsertMode) {
	Owner->SetCursorFat ();
    } else {
	Owner->SetCursorOn ();
    }
}



void EditLine::AdjustPos ()
// Sets all working variables when InsertPos has changed
{
    if (InsertPos < FirstPos) {
	FirstPos = InsertPos;
    } else if (InsertPos >= (FirstPos + FieldLen)) {
	FirstPos = InsertPos - FieldLen;
    }
    CursorX = FieldX + (InsertPos - FirstPos);
    if (CursorX < FieldX) {
	// Scroll left
	FirstPos--;
	CursorX++;
    } else if (CursorX >= (FieldX + FieldLen)) {
	// Scroll right
	FirstPos++;
	CursorX--;
    }
}



void EditLine::InputChar (Key K)
// Inserts a normal char
{
    // Check if the char is valid
    if (!CharIsOk (K)) {
	return;
    }

    // If this is the first char, clear the line
    if (FirstChar) {
	Line.Trunc (0);			// Make an empty string
	FirstPos  = 0;
	InsertPos = 0;
	FirstChar = 0;
    }

    //
    u16 Len = Line.Len ();
    if (InsertPos < Len) {
	// Insert in the mid if Line
	if (InsertMode) {
	    // Insert only if line does not get too long
	    if (Len < MaxLen) {
		Line.Ins (InsertPos, (char) K);
		InsertPos++;
	    }
	} else {
	    Line [InsertPos] = (char) K;
	    InsertPos++;
	}
    } else {
	// Insert at the end
	if (Len < MaxLen) {
	    Line += (char) K;
	    InsertPos++;
	}
    }

    // Adjust variables and redraw the field
    AdjustPos ();
    DrawField ();
}



void EditLine::Edit (int& Abort)
// Start editing mode. Abort is set to 1 if the user aborted the
// input (Line is not changed in this case), 0 if the input was
// accepted (by pressing enter).
{
    // Bail out if the item is not active
    if (!IsActive ()) {
	Abort = 1;
	return;
    }

    // Remember the state of the owner window and the cursor form
    u16 OldState = Owner->GetState ();
    Window::CursorType Cursor = Owner->GetCursor ();

    // Update status line
    u32 StatusFlags = siAbort | siEnter;
    if (HasHelp ()) {
	StatusFlags |= siHelp;
    }
    PushStatusLine (StatusFlags);

    // Make the owner window active
    Owner->Activate ();

    // Remember the old input line. The line is restored if the input
    // is aborted
    String OldLine (Line);

    // Set up the cursor position, insert position etc.
    u16 Len = Line.Len ();
    if (Len < FieldLen) {
	FirstPos = 0;
    } else {
	FirstPos = Len - FieldLen + 1;
    }
    InsertPos = Len;
    AdjustPos ();

    // We are waiting for the first input char
    FirstChar = 1;

    // Draw the output field and set the cursor
    DrawField ();
    SetCursorForm ();

    // Now start editing loop
    int EditDone = 0;
    Key K;
    while (!EditDone) {

	// Get a key
	K = KbdGet ();

	// Check what it is...
	switch (K) {

	    // Help key
	    case vkHelp:
		CallHelp ();
		break;

	    // Resize owner window
	    case vkResize:
		Owner->MoveResize ();
		break;

	    // Abort editing
	    case vkAbort:
		Abort	 = 1;
		EditDone = 1;
		Line	 = OldLine;	// Restore old input line
		break;

	    // Accept the input line
	    case kbEnter:
	    case vkAccept:
		if (EndCheckOk ()) {
		    Abort    = 0;
		    EditDone = 1;
		}
		break;

	    // Switch insert mode
	    case vkIns:
		InsertMode = !InsertMode;
		SetCursorForm ();
		DrawIfFirstChar ();
		break;

	    // Cursor to the left
	    case vkLeft:
		if (InsertPos > 0) {
		    InsertPos--;
		    AdjustPos ();
		    FirstChar = 0;
		    DrawField ();
		} else {
		    DrawIfFirstChar ();
		}
		break;

	    // Cursor to the right
	    case vkRight:
		if (InsertPos < Line.Len ()) {
		    InsertPos++;
		    AdjustPos ();
		    FirstChar = 0;
		    DrawField ();
		} else {
		    DrawIfFirstChar ();
		}
		break;

	    // Destructive backspace
	    case kbBack:
		if (InsertPos > 0) {
		    InsertPos--;
		    Line.Del (InsertPos, 1);
		    AdjustPos ();
		    if (InsertPos == Line.Len ()) {
			if (FirstPos > 0) {
			    FirstPos--;
			    CursorX ++;
			}
		    } else if (CursorX == FieldX && FirstPos > 0) {
			CursorX++;
			FirstPos--;
		    }
		    FirstChar = 0;
		    DrawField ();
		} else {
		    DrawIfFirstChar ();
		}
		break;

	    // Delete char under cursor
	    case vkDel:
		if (InsertPos < Line.Len ()) {
		    Line.Del (InsertPos, 1);
		    FirstChar = 0;
		    DrawField ();
		} else {
		    DrawIfFirstChar ();
		}
		break;

	    // Home key
	    case vkHome:
		if (InsertPos > 0) {
		    InsertPos = 0;
		    AdjustPos ();
		    FirstChar = 0;
		    DrawField ();
		} else {
		    DrawIfFirstChar ();
		}
		break;

	    // End key
	    case vkEnd:
		Len = Line.Len ();
		if (InsertPos < Len) {
		    InsertPos = Len;
		    AdjustPos ();
		    FirstChar = 0;
		    DrawField ();
		} else {
		    DrawIfFirstChar ();
		}
		break;

	    // Insert all chars
	    case kbCtrlP:
		DrawIfFirstChar ();
		K = KbdGet ();
		if (IsPlainKey (K)) {
		    InputChar (K);
		}
		break;

	    // All other keys
	    default:
		if (K != AccelKey && KeyIsRegistered (K)) {
		    // Key is reserved
		    if (EndCheckOk ()) {
			// Input is valid, write back the key
			KbdPut (K);
			Abort = 0;
			EditDone = 1;
		    }
		} else {
		    // Just a normal char
		    InputChar (K);
		}
		break;

	} // End switch

    } // End while

    // Reset variables and redraw the item
    CursorX   = 0;
    FirstPos  = 0;
    FirstChar = 0;
    DrawField ();

    // Set the old window state
    Owner->SetState (OldState);
    Owner->SetCursor (Cursor);

    // Restore the old status line
    PopStatusLine ();
}



i16 EditLine::Choose ()
// Calls Edit and returns the id of the entry if the editing was
// not aborted. Otherwise 0 is returned.
{
    int Abort;
    Edit (Abort);
    return Abort ? 0 : ID;
}



/*****************************************************************************/
/*				class FloatEdit				     */
/*****************************************************************************/



FloatEdit::FloatEdit (const String& aItemText, i16 aID, u16 aLD, u16 aTD,
		      WindowItem *NextItem) :
    EditLine (aItemText, aID, aTD ? aLD + aTD + 1 : aLD,
	      aTD ? aLD + aTD + 2 : aLD + 1, NextItem),
    FValue (0),
    LD (aLD),
    TD (aTD),
    FMin (0),
    FMax (0)
{
    // Set the value for the EditLine Line variable
    Line = FloatStr (FValue, LD, TD);
}



FloatEdit::FloatEdit (const String & aItemText, i16 aID, u16 aLD, u16 aTD,
		      double Min, double Max, WindowItem *NextItem) :
    EditLine (aItemText, aID, aTD ? aLD + aTD + 1 : aLD,
	      aTD ? aLD + aTD + 2 : aLD + 1, NextItem),
    FValue (Min),
    LD (aLD),
    TD (aTD),
    FMin (Min),
    FMax (Max)
{
    // Set the value for the EditLine Line variable
    Line = FloatStr (FValue, LD, TD);
}



int FloatEdit::CharIsOk (Key& K)
// This function is called with the users input to check if the
// input is valid. The function returns 1 if the key is acceptable,
// 0 otherwise. Overload this function for restricting input.
// The function can change the given char!
{
    // Examine the key
    if (IsDigit (K)) {
	return 1;
    } else if (K == (unsigned char) NLSData.DecSep) {
	// Decimal separator is allowed in one place only
	return Line.Pos (NLSData.DecSep) < 0;
    } else if (K == '-') {
	// Allowed in the first place only if minimum < 0
	return ((FMin < 0 && InsertPos == 0) || (FMin < 0 && FirstChar));
    } else {
	// Disallow all other chars
	return 0;
    }
}



int FloatEdit::EndCheckOk ()
// This function is called when the user has ended editing. It returns
// 1 if the contents of Line are acceptable, 0 if not. In the latter
// case it's up to this function to display an appropriate error
// message.
{
    double NewVal;

    // Parse the line
    StringParser S (Line);
    unsigned Res = S.GetFloat (NewVal);
    if (Res != 0) {
	// cannot convert line
	ErrorMsg (S.GetMsg (Res));
	return 0;
    }

    // String can be converted to a float. Check for leading/trailing digits
    int SepPos = Line.Pos (NLSData.DecSep);
    if (SepPos >= 0) {
	// Has a separator
	if (Line.Len () - SepPos - 1 > TD) {
	    // Too many trailing digits
	    ErrorMsg (msErrTrailingDigits);
	    return 0;
	}
    }

    // Now check against the limits
    if (NewVal < FMin) {
	ErrorMsg (msErrFTooSmall, FMin);
	return 0;
    }
    if (NewVal > FMax) {
	ErrorMsg (msErrFTooLarge, FMax);
	return 0;
    }

    // That's it. The value is ok
    SetValue (NewVal);
    return 1;
}



void FloatEdit::Store (Stream& S) const
// Store the object into a stream
{
    EditLine::Store (S);
    S << FValue << LD << TD << FMin << FMax;
}



void FloatEdit::Load (Stream& S)
// Load the object from a stream
{
    EditLine::Load (S);
    S >> FValue >> LD >> TD >> FMin >> FMax;
}



u16 FloatEdit::StreamableID () const
{
    return ID_FloatEdit;
}



Streamable* FloatEdit::Build ()
{
    return new FloatEdit (Empty);
}



void FloatEdit::SetValue (double NewVal)
// Set a new value for FValue
{
    // Remember the new value
    FValue = NewVal;

    // Set the input line
    EditLine::SetValue (FloatStr (FValue, LD, TD));
}



void FloatEdit::SetMinMax (double Min, double Max)
// Set the FMin/FMax values
{
    PRECONDITION (Min <= Max);
    FMin = Min;
    FMax = Max;
}



/*****************************************************************************/
/*				class LongEdit				     */
/*****************************************************************************/



LongEdit::LongEdit (const String &aItemText, i16 aID, u16 aDigits,
		    WindowItem *NextItem) :
    EditLine (aItemText, aID, aDigits, aDigits + 1, NextItem),
    LValue (0),
    LMin (0),
    LMax (0),
    Digits (aDigits)
{
    // Set the value for the EditLine Line variable
    Line = I32Str (LValue);
}



LongEdit::LongEdit (const String &aItemText, i16 aID, u16 aDigits,
		    i32 Min, i32 Max, WindowItem *NextItem) :
    EditLine (aItemText, aID, aDigits, aDigits + 1, NextItem),
    LValue (Min),
    LMin (Min),
    LMax (Max),
    Digits (aDigits)
{
    // Check Min/Max values
    PRECONDITION (Min <= Max);

    // Set the value for the EditLine Line variable
    Line = I32Str (LValue);
}



int LongEdit::CharIsOk (Key& K)
// This function is called with the users input to check if the
// input is valid. The function returns 1 if the key is acceptable,
// 0 otherwise. Overload this function for restricting input.
// The function can change the given char!
{
    // Digits and a minus sign in the first place are ok
    return IsDigit (K) || (LMin < 0 && InsertPos == 0);
}



int LongEdit::EndCheckOk ()
// This function is called when the user has ended editing. It returns
// 1 if the contents of Line are acceptable, 0 if not. In the latter
// case it's up to this function to display an appropriate error
// message.
{
    // Convert the string
    i32 NewVal = 0;
    StringParser P (Line);
    unsigned Res = P.GetI32 (NewVal);
    if (Res != 0) {
	ErrorMsg (P.GetMsg (Res));
	return 0;
    }

    // Now check against the limits
    if (NewVal < LMin) {
	ErrorMsg (msErrLTooSmall, LMin);
	return 0;
    }
    if (NewVal > LMax) {
	ErrorMsg (msErrLTooLarge, LMax);
	return 0;
    }

    // That's it. The value is ok
    SetValue (NewVal);
    return 1;
}



void LongEdit::Store (Stream& S) const
// Store the object into a stream
{
    EditLine::Store (S);
    S << LValue << LMin << LMax << Digits;
}



void LongEdit::Load (Stream& S)
// Load an object from a stream
{
    EditLine::Load (S);
    S >> LValue >> LMin >> LMax >> Digits;
}



u16 LongEdit::StreamableID () const
{
    return ID_LongEdit;
}



Streamable* LongEdit::Build ()
{
    return new LongEdit (Empty);
}



void LongEdit::SetValue (i32 NewVal)
// Set a new value for LValue
{
    // Remember new value
    LValue = NewVal;

    // Set the input line
    EditLine::SetValue (I32Str (LValue));
}



void LongEdit::SetMinMax (i32 Min, i32 Max)
// Set the FMin/FMax values
{
    // Check the parameters
    PRECONDITION (Min <= Max);

    // Remember the new values
    LMin = Min;
    LMax = Max;
}



/*****************************************************************************/
/*				 class HexEdit				     */
/*****************************************************************************/



int HexEdit::CharIsOk (Key& K)
// This function is called with the users input to check if the
// input is valid. The function returns 1 if the key is acceptable,
// 0 otherwise. Overload this function for restricting input.
// The function can change the given char!
{
    if (IsXDigit (K)) {
	// These are ok, but convert to caps
	K = (Key) NLSUpCase ( (char) K);
	return 1;
    } else {
	return 0;
    }
}



int HexEdit::EndCheckOk ()
// This function is called when the user has ended editing. It returns
// 1 if the contents of Line are acceptable, 0 if not. In the latter
// case it's up to this function to display an appropriate error
// message.
{
    // Convert the string
    u32 NewVal = 0;
    StringParser P (Line);
    unsigned Res = P.GetU32 (NewVal);
    if (Res != 0) {
	ErrorMsg (P.GetMsg (Res));
	return 0;
    }

    // Now check against the limits
    if (NewVal < u32 (LMin)) {
	ErrorMsg (msErrLTooSmall, LMin);
	return 0;
    }
    if (NewVal > u32 (LMax)) {
	ErrorMsg (msErrLTooLarge, LMax);
	return 0;
    }

    // That's it. The value is ok
    SetValue (NewVal);
    return 1;
}



u16 HexEdit::StreamableID () const
{
    return ID_HexEdit;
}



Streamable * HexEdit::Build ()
{
    return new HexEdit (Empty);
}



void HexEdit::SetValue (u32 NewVal)
// Set a new value for LValue
{
    // Remember new value
    LValue = NewVal;

    // Set the input line
    EditLine::SetValue (U32Str (LValue, 16));
}



/*****************************************************************************/
/*				class TextEdit				     */
/*****************************************************************************/



TextEdit::TextEdit (const String& ItemText, i16 ItemID, u16 aMaxLen,
		    u16 aFieldLen, WindowItem* NextItem):
    EditLine (ItemText, ItemID, aMaxLen, aFieldLen, NextItem)
{
    // Allow all chars but control chars, ASCII ONLY
    AllowedChars.AddRange (0x20, 0xFF);
}



TextEdit::TextEdit (StreamableInit):
    EditLine (Empty),
    AllowedChars (Empty)
{
}



int TextEdit::CharIsOk (Key& K)
// This function is called with the users input to check if the
// input is valid. The function returns 1 if the key is acceptable,
// 0 otherwise. Overload this function for restricting input.
// The function can change the given char!
{
    return IsPlainKey (K) && AllowedChars [K] != 0;
}



int TextEdit::EndCheckOk ()
// This function is called when the user has ended editing. It returns
// 1 if the contents of Line are acceptable, 0 if not. In the latter
// case it's up to this function to display an appropriate error
// message.
{
    // Char #0 is a flag that determines if an empty line is allowed
    if (AllowedChars [0] == 0 && Line.IsEmpty ()) {
	ErrorMsg (msErrNoEmptyInput);
	return 0;
    } else {
	return 1;
    }
}



void TextEdit::SetAllowedChars (const CharSet& CS)
// Set the allowed input chars
{
    AllowedChars = CS;
}



void TextEdit::Store (Stream& S) const
{
    EditLine::Store (S);
    S << AllowedChars;
}



void TextEdit::Load (Stream& S)
{
    EditLine::Load (S);

    // Use the else case instead reading AllowedChars to convert old resources
#if 1
    S >> AllowedChars;
#else
    AllowedChars.Clear ();
    AllowedChars.AddRange (0x20, 0xFF);
#endif
}



u16 TextEdit::StreamableID () const
{
    return ID_TextEdit;
}



Streamable* TextEdit::Build ()
{
    return new TextEdit (Empty);
}



/*****************************************************************************/
/*				class TimeEdit				     */
/*****************************************************************************/



int TimeEdit::CharIsOk (Key& K)
// This function is called with the users input to check if the
// input is valid. The function returns 1 if the key is acceptable,
// 0 otherwise. Overload this function for restricting input.
// The function can change the given char!
{
    return IsDigit (K) || (K == (unsigned char) NLSData.TimeSep);
}



int TimeEdit::EndCheckOk ()
// This function is called when the user has ended editing. It returns
// 1 if the contents of Line are acceptable, 0 if not. In the latter
// case it's up to this function to display an appropriate error
// message.
{
    // Set up a string parser for Line
    Time NewVal;
    StringParser P (Line);
    unsigned Res = P.GetTime (NewVal);
    if (Res != 0) {
	ErrorMsg (P.GetMsg (Res));
	return 0;
    }

    // That's it. The value is ok
    SetValue (NewVal);
    return 1;
}



TimeEdit::TimeEdit (const String& aItemText, i16 aID, WindowItem* NextItem) :
    EditLine (aItemText, aID, 8, 9, NextItem),
    TValue ()
{
    // Set the value for the EditLine Line variable
    Line = TValue.TimeStr ();
}



void TimeEdit::Store (Stream& S) const
{
    // Store parental data
    EditLine::Store (S);

    // Store own data
    S << TValue;
}



void TimeEdit::Load (Stream& S)
{
    // Load parental data
    EditLine::Load (S);

    // Load own data
    S >> TValue;
}



u16 TimeEdit::StreamableID () const
{
    return ID_TimeEdit;
}



Streamable* TimeEdit::Build ()
{
    return new TimeEdit (Empty);
}



void TimeEdit::SetValue (const Time& NewVal)
// Set a new value for TValue
{
    // Remember new value
    TValue = NewVal;

    // Set the input line
    EditLine::SetValue (TValue.TimeStr ());
}



void TimeEdit::SetValue (unsigned Hour, unsigned Min, unsigned Sec)
{
    // Set the new value
    TValue.SetTime (Hour, Min, Sec);

    // Set the input line
    EditLine::SetValue (TValue.TimeStr ());
}



void TimeEdit::SetValue (u32 Seconds)
{
    unsigned Sec  = Seconds % 60;
    Seconds /= 60;
    unsigned Min  = Seconds % 60;
    unsigned Hour = Seconds / 60;
    CHECK (Hour < 24);

    // Set the new value
    SetValue (Hour, Min, Sec);
}



/*****************************************************************************/
/*				class DateEdit				     */
/*****************************************************************************/



int DateEdit::CharIsOk (Key& K)
// This function is called with the users input to check if the
// input is valid. The function returns 1 if the key is acceptable,
// 0 otherwise. Overload this function for restricting input.
// The function can change the given char!
{
    return IsDigit (K) || (K == (unsigned char) NLSData.DateSep);
}



int DateEdit::EndCheckOk ()
// This function is called when the user has ended editing. It returns
// 1 if the contents of Line are acceptable, 0 if not. In the latter
// case it's up to this function to display an appropriate error
// message.
{
    // Set up a string parser for Line
    Time NewVal;
    StringParser P (Line);
    unsigned Res = P.GetDate (NewVal);
    if (Res != 0) {
	ErrorMsg (P.GetMsg (Res));
	return 0;
    }

    // That's it. The value is ok
    SetValue (NewVal);
    return 1;
}



DateEdit::DateEdit (const String& aItemText, i16 aID,
			   WindowItem* NextItem) :
    EditLine (aItemText, aID, 10, 11, NextItem)
{
    // Set the value for the EditLine Line variable
    Line = TValue.DateStr ();
}



void DateEdit::Store (Stream& S) const
{
    // Store parental data
    EditLine::Store (S);

    // Store own data
    S << TValue;
}



void DateEdit::Load (Stream& S)
{
    // Load parental data
    EditLine::Load (S);

    // Load own data
    S >> TValue;
}



u16 DateEdit::StreamableID () const
{
    return ID_DateEdit;
}



Streamable* DateEdit::Build ()
{
    return new DateEdit (Empty);
}



void DateEdit::SetValue (const Time& NewVal)
// Set a new value for TValue
{
    // Remember new value
    TValue = NewVal;

    // Set the input line
    EditLine::SetValue (TValue.DateStr ());
}



void DateEdit::SetValue (unsigned Year, unsigned Month, unsigned Day)
{
    // Set new value
    TValue.SetDate (Year, Month, Day);

    // Set the input line
    EditLine::SetValue (TValue.DateStr ());
}



/*****************************************************************************/
/*			      class PasswordEdit			     */
/*****************************************************************************/



void PasswordEdit::DrawField ()
// This function is called from Edit and draws the editing field.
// FirstPos is the position (in Line) of the first char that is
// printed. CursorX is the position of the cursor in the window.
// First is true if the user has not pressed any key. In this case,
// a special attribuite for the text is used, because the first
// keypress of a non cursor key will clear the line.
// This functions overloads EditLine::DrawField and outputs '*'s
// instead of the real line
{
    // Create a new string with the length of the visible portion
    // of the input line
    String S (FieldLen);
    S.Set (0, Line.Len () - FirstPos, '*');
    S.Pad (String::Right, FieldLen);

    // Draw the line
    Owner->Write (FieldX, ItemY, S, FirstChar ? atEditHigh : atEditNormal);

    // Draw/delete both arrows
    DrawLeftArrow (FirstPos != 0);
    DrawRightArrow ((FirstPos + FieldLen) < Line.Len ());

    // Reset the cursor
    Owner->SetCursorPos (Point (CursorX, ItemY));
}



u16 PasswordEdit::StreamableID () const
{
    return ID_PasswordEdit;
}



Streamable* PasswordEdit::Build ()
{
    return new PasswordEdit (Empty);
}



/*****************************************************************************/
/*				class FileEdit				     */
/*****************************************************************************/



int FileEdit::CharIsOk (Key& K)
// This function is called with the users input to check if the
// input is valid. The function returns 1 if the key is acceptable,
// 0 otherwise. Overload this function for restricting input.
// The function can change the given char!
{
    if (IsPlainKey (K) == 0 || IsCntrl (K)) {
	// Invalid char
	return 0;
    }

    // Normal character?
    if (FileSysValidChar ((char) K)) {
	// Ok
	return 1;
    }

    // Wildcard character?
    if (strchr ("*?[]{,}", K) != NULL) {
	// Wildcard are allowed if the corresponding flag is set
	return WildcardsOk ();
    }

    // Path separator?
    if (K == (unsigned char) FileSysPathSep) {
	return PathOk ();
    }

#if defined(FILESYS_HAS_DRIVES) || defined(FILESYS_HAS_VOLUMES)
    // A colon is ok if paths are allowed and the file system supports
    // drives or volumes
    if (K == ':') {
	return PathOk () && Line.Pos (':') == -1;
    }
#endif

    // invalid char
    return 0;

}



int FileEdit::EndCheckOk ()
// This function is called when the user has ended editing. It returns
// 1 if the contents of Line are acceptable, 0 if not. In the latter
// case it's up to this function to display an appropriate error
// message.
{
    if (!ExtensionOk ()) {
	// No extension allowed - check this
	String Path;
	String Name;
	String Ext;
	FSplit (Line, Path, Name, Ext);

	if (!Ext.IsEmpty ()) {
	    ErrorMsg (msErrNoExtension);
	    return 0;
	}
    }

    // Line is ok
    return 1;
}



FileEdit::FileEdit (const String& ItemText, i16 ItemID, u16 aMaxLen,
		    u16 aFieldLen, u16 aFileFlags, WindowItem* NextItem):
    EditLine (ItemText, ItemID, aMaxLen, aFieldLen, NextItem),
    FileFlags (aFileFlags)
{
}



void FileEdit::Store (Stream& S) const
{
    // Store parental data
    EditLine::Store (S);

    // Store class data
    S << FileFlags;
}



void FileEdit::Load (Stream& S)
{
    // Load parental data
    EditLine::Load (S);

    // Load class data
    S >> FileFlags;
}



u16 FileEdit::StreamableID () const
{
    return ID_FileEdit;
}



Streamable* FileEdit::Build ()
{
    return new FileEdit (Empty);
}



void FileEdit::Draw ()
// Overloads function TEditLine::Draw. Cuts the path if it is to long
// to be displayed
{
    // Is the line too long to be displayed?
    if (Line.Len () > FieldLen - 3) {

	// It is too long, display a shortened version
	String LineTmp = Line;
	Line = ShortPath (Line, FieldLen - 3);

	// Use the draw function from EditLine now
	EditLine::Draw ();

	// Restore the old line contents
	Line = LineTmp;

    } else {

	// The line fits, just display it
	EditLine::Draw ();

    }
}



void FileEdit::SetFileFlags (u16 NewFlags)
// Change the FileFlags settings
{
    FileFlags = NewFlags;
}




