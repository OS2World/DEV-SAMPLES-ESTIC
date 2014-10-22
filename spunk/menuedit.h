/*****************************************************************************/
/*                                                                           */
/*                                MENUEDIT.H                                 */
/*                                                                           */
/* (C) 1993-95  Ullrich von Bassewitz                                        */
/*              Zwehrenbuehlstrasse 33                                       */
/*              D-72070 Tuebingen                                            */
/* EMail:       uz@ibb.schwaben.com                                          */
/*                                                                           */
/*****************************************************************************/



// $Id$
//
// $Log$
//
//



#ifndef __MENUEDIT_H
#define __MENUEDIT_H



#include "itemwin.h"
#include "datetime.h"
#include "charset.h"



/*****************************************************************************/
/*                        Constants used by FileEdit                         */
/*****************************************************************************/



const u16 ffPathOk              = 0x0001;       // path accepted if set
const u16 ffWildcardsOk         = 0x0002;       // wildcards accepted if set
const u16 ffExtensionOk         = 0x0004;       // extension accepted if set



/*****************************************************************************/
/*                              class EditLine                               */
/*****************************************************************************/



class EditLine: public WindowItem {

    friend class ResEditApp;                    // Resource editor is a friend

private:
    void DrawIfFirstChar ();
    // Is called to redraw the item if nothing has changed but the item
    // has to be redrawn (e.g. because the the state of FirstChar has
    // changed).

    void SetCursorForm ();
    // Sets the cursor shape according to the state of InsertMode

    void AdjustPos ();
    // Sets all working variables when InsertPos has changed

    void InputChar (Key K);
    // Inserts a normal char


protected:
    String              Line;                   // The edit line
    u16                 FieldLen;               // Length of the entry field
    u16                 FieldX;                 // Start pos of entry
    u16                 LeftArrowX;             // Position left arrow
    u16                 RightArrowX;            // Position right arrow
    i16                 InsertMode;             // Insert mode on/off
    u16                 MaxLen;                 // max input length

    // Working variables
    u16                 InsertPos;              // Where is the next char inserted?
    int                 FirstChar;
    u16                 CursorX;                // Position of the cursor
    u16                 FirstPos;               // First position in field



    virtual int CharIsOk (Key& K);
    // This function is called with the users input to check if the
    // input is valid. The function returns 1 if the key is acceptable,
    // 0 otherwise. Overload this function for restricting input.
    // The function can change the given char!

    virtual int EndCheckOk ();
    // This function is called when the user has ended editing. It returns
    // 1 if the contents of Line are acceptable, 0 if not. In the latter
    // case it's up to this function to display an appropriate error
    // message.

    void DrawLeftArrow (int Arrow);
    // Draws or clears the left arrow according to the given flag

    void DrawRightArrow (int Arrow);
    // Draws or clears the right arrow according to the given flag

    virtual void DrawField ();
    // This function is called from Edit and draws the editing field.
    // FirstPos is the position (in Line) of the first char that is
    // printed. CursorX is the position of the cursor in the window.
    // First is true if the user has not pressed any key. In this case,
    // a special attribuite for the text is used, because the first
    // keypress of a non cursor key will clear the line.

    EditLine (StreamableInit);
    // Build constructor


public:
    EditLine (const String& ItemText, i16 ItemID, u16 aMaxLen, u16 aFieldLen,
              WindowItem* NextItem);

    // Derived from class Streamable
    virtual void Store (Stream&) const;
    virtual void Load (Stream&);
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    // Derived from class WindowItem
    virtual void SetWidth (u16 NewWidth);
    virtual void SetPos (u16 X, u16 Y);
    virtual void Draw ();
    virtual u16 MinWidth ();

    // -- New functions

    void SetValue (const String& NewVal);
    // Set a new value for Line

    const String& GetValue () const;
    // Return the current value of Line

    void Edit (int& Abort);
    // Start editing mode. Abort is set to 1 if the user aborted the
    // input (Line is not changed in this case), 0 if the input was
    // accepted (by pressing enter).

    virtual i16 Choose ();
    // Calls Edit and returns the id of the entry if the editing was
    // not aborted. Otherwise 0 is returned.

};



inline EditLine::EditLine (StreamableInit) :
        WindowItem (Empty), Line (Empty)
{
}



inline const String&  EditLine::GetValue () const
// Return the value of Line
{
    return Line;
}



/*****************************************************************************/
/*                              class FloatEdit                              */
/*****************************************************************************/



class FloatEdit: public EditLine {

    friend class ResEditApp;                    // Resource editor is a friend

protected:
    double              FValue;                 // Current value
    u16                 LD, TD;                 // Leading and trailing digits
    double              FMin;                   // Minimum
    double              FMax;                   // Maximum


    virtual int CharIsOk (Key& K);
    // This function is called with the users input to check if the
    // input is valid. The function returns 1 if the key is acceptable,
    // 0 otherwise. Overload this function for restricting input.
    // The function can change the given char!

    virtual int EndCheckOk ();
    // This function is called when the user has ended editing. It returns
    // 1 if the contents of Line are acceptable, 0 if not. In the latter
    // case it's up to this function to display an appropriate error
    // message.

    FloatEdit (StreamableInit);
    // Build constructor

public:
    FloatEdit (const String& aItemText, i16 aID, u16 aLD, u16 aTD,
               WindowItem* NextItem);
    FloatEdit (const String& aItemText, i16 aID, u16 aLD, u16 aTD,
               double Min, double Max, WindowItem* NextItem);

    // Derived from class Streamable
    virtual void Store (Stream&) const;
    virtual void Load (Stream&);
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    // -- new functions

    void SetValue (double NewVal);
    // Set a new value for FValue

    double GetValue () const;
    // Get the current value

    void SetMinMax (double Min, double Max);
    // Set the FMin/FMax values

    void GetMinMax (double& Min, double& Max) const;
    // Get the FMin/FMax values

};



inline FloatEdit::FloatEdit (StreamableInit) :
        EditLine (Empty)
{
}



inline double FloatEdit::GetValue () const
{
    return FValue;
}



inline void FloatEdit::GetMinMax (double &Min, double &Max) const
{
    Min = FMin;
    Max = FMax;
}



/*****************************************************************************/
/*                              class LongEdit                               */
/*****************************************************************************/



class LongEdit: public EditLine {

    friend class ResEditApp;                    // Resource editor is a friend

protected:
    i32                 LValue;                 // Current value
    i32                 LMin;                   // Minimum value
    i32                 LMax;                   // Maximum value
    u16                 Digits;


    virtual int CharIsOk (Key& K);
    // This function is called with the users input to check if the
    // input is valid. The function returns 1 if the key is acceptable,
    // 0 otherwise. Overload this function for restricting input.
    // The function can change the given char!

    virtual int EndCheckOk ();
    // This function is called when the user has ended editing. It returns
    // 1 if the contents of Line are acceptable, 0 if not. In the latter
    // case it's up to this function to display an appropriate error
    // message.

    LongEdit (StreamableInit);
    // Build constructor

public:
    LongEdit (const String& aItemText, i16 aID, u16 aDigits,
              WindowItem* NextItem);
    LongEdit (const String& aItemText, i16 aID, u16 aDigits,
              i32 Min, i32 Max, WindowItem* NextItem);

    // Derived from class Streamable
    virtual void Store (Stream&) const;
    virtual void Load (Stream&);
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    // -- new functions

    void SetValue (i32 NewVal);
    // Set a new value for LValue

    i32 GetValue () const;
    // Get the current value

    void SetMinMax (i32 Min, i32 Max);
    // Set the FMin/FMax values

    void GetMinMax (i32 &Min, i32 &Max) const;
    // Get the FMin/FMax values
};



inline LongEdit::LongEdit (StreamableInit) :
        EditLine (Empty)
{
}



inline i32 LongEdit::GetValue () const
{
    return LValue;
}



inline void LongEdit::GetMinMax (i32 &Min, i32 &Max) const
{
    Min = LMin;
    Max = LMax;
}



/*****************************************************************************/
/*                               class HexEdit                               */
/*****************************************************************************/



class HexEdit: public LongEdit {

    friend class ResEditApp;                    // Resource editor is a friend

protected:

    virtual int CharIsOk (Key& K);
    // This function is called with the users input to check if the
    // input is valid. The function returns 1 if the key is acceptable,
    // 0 otherwise. Overload this function for restricting input.
    // The function can change the given char!

    virtual int EndCheckOk ();
    // This function is called when the user has ended editing. It returns
    // 1 if the contents of Line are acceptable, 0 if not. In the latter
    // case it's up to this function to display an appropriate error
    // message.

    HexEdit (StreamableInit);
    // Build constructor

public:
    HexEdit (const String& aItemText, i16 aID, u16 aDigits,
              WindowItem* NextItem);
    HexEdit (const String& aItemText, i16 aID, u16 aDigits,
             i32 Min, i32 Max, WindowItem* NextItem);

    // Derived from class Streamable
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    // -- new functions

    void SetValue (u32 NewVal);
    // Set a new value for LValue

};



inline HexEdit::HexEdit (StreamableInit) :
        LongEdit (Empty)
{
}



inline HexEdit::HexEdit (const String& aItemText, i16 aID, u16 aDigits,
                  WindowItem* NextItem) :
        LongEdit (aItemText, aID, aDigits, NextItem)
{
}



inline HexEdit::HexEdit (const String& aItemText, i16 aID, u16 aDigits,
                  i32 Min, i32 Max, WindowItem* NextItem) :
        LongEdit (aItemText, aID, aDigits, Min, Max, NextItem)
{
}



/*****************************************************************************/
/*                              class TextEdit                               */
/*****************************************************************************/



class TextEdit: public EditLine {

    friend class ResEditApp;            // Resource editor is a friend

protected:
    CharSet     AllowedChars;           // Set of allowed input chars


    virtual int CharIsOk (Key& K);
    // This function is called with the users input to check if the
    // input is valid. The function returns 1 if the key is acceptable,
    // 0 otherwise. Overload this function for restricting input.
    // The function can change the given char!

    virtual int EndCheckOk ();
    // This function is called when the user has ended editing. It returns
    // 1 if the contents of Line are acceptable, 0 if not. In the latter
    // case it's up to this function to display an appropriate error
    // message.

    TextEdit (StreamableInit);
    // Build constructor


public:
    TextEdit (const String& ItemText, i16 ItemID, u16 aMaxLen, u16 aFieldLen,
              WindowItem* NextItem);

    // Derived from class Streamable
    virtual void Store (Stream&) const;
    virtual void Load (Stream&);
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    const CharSet& GetAllowedChars () const;
    // Get the set of allowed input chars

    void SetAllowedChars (const CharSet& CS);
    // Set the allowed input chars

    void AllowEmptyInput ();
    void DisallowEmptyInput ();
    // Allow or disallow an empty input line
};



inline const CharSet& TextEdit::GetAllowedChars () const
// Get the set of allowed input chars
{
    return AllowedChars;
}



inline void TextEdit::AllowEmptyInput ()
// Allow an empty input line
{
    AllowedChars += '\0';
}



inline void TextEdit::DisallowEmptyInput ()
// Disallow an empty input line
{
    AllowedChars -= '\0';
}



/*****************************************************************************/
/*                              class TimeEdit                               */
/*****************************************************************************/



class TimeEdit: public EditLine {

protected:
    Time        TValue;

    virtual int CharIsOk (Key &K);
    // This function is called with the users input to check if the
    // input is valid. The function returns 1 if the key is acceptable,
    // 0 otherwise. Overload this function for restricting input.
    // The function can change the given char!

    virtual int EndCheckOk ();
    // This function is called when the user has ended editing. It returns
    // 1 if the contents of Line are acceptable, 0 if not. In the latter
    // case it's up to this function to display an appropriate error
    // message.

    TimeEdit (StreamableInit);
    // Build constructor

public:
    TimeEdit (const String& aItemText, i16 aID, WindowItem* NextItem);

    // Derived from class Streamable
    virtual void Store (Stream&) const;
    virtual void Load (Stream&);
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    // -- new functions

    void SetValue (const Time& NewVal);
    void SetValue (unsigned Hour, unsigned Min, unsigned Sec);
    void SetValue (u32 Sec);
    // Set a new value for TValue

    const Time& GetValue () const;
    // Get the current value

};



inline TimeEdit::TimeEdit (StreamableInit) :
    EditLine (Empty),
    TValue (Empty)
{
}



inline const Time& TimeEdit::GetValue () const
// Get the current value
{
    return TValue;
}



/*****************************************************************************/
/*                              class DateEdit                               */
/*****************************************************************************/



class DateEdit: public EditLine {

protected:
    Time        TValue;


    virtual int CharIsOk (Key &K);
    // This function is called with the users input to check if the
    // input is valid. The function returns 1 if the key is acceptable,
    // 0 otherwise. Overload this function for restricting input.
    // The function can change the given char!

    virtual int EndCheckOk ();
    // This function is called when the user has ended editing. It returns
    // 1 if the contents of Line are acceptable, 0 if not. In the latter
    // case it's up to this function to display an appropriate error
    // message.

    DateEdit (StreamableInit);
    // Build constructor

public:
    DateEdit (const String& aItemText, i16 aID, WindowItem* NextItem);

    // Derived from class Streamable
    virtual void Store (Stream&) const;
    virtual void Load (Stream&);
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    // -- new functions

    void SetValue (const Time& NewVal);
    void SetValue (unsigned Year, unsigned Month, unsigned Day);
    // Set a new value for TValue

    const Time& GetValue () const;
    // Get the current value

};



inline DateEdit::DateEdit (StreamableInit) :
    EditLine (Empty),
    TValue (Empty)
{
}



inline const Time& DateEdit::GetValue () const
{
    return TValue;
}



/*****************************************************************************/
/*                            class PasswordEdit                             */
/*****************************************************************************/



class PasswordEdit: public EditLine {

protected:

    PasswordEdit (StreamableInit);
    // Build constructor

    virtual void DrawField ();
    // This function is called from Edit and draws the editing field.
    // FirstPos is the position (in Line) of the first char that is
    // printed. CursorX is the position of the cursor in the window.
    // First is true if the user has not pressed any key. In this case,
    // a special attribuite for the text is used, because the first
    // keypress of a non cursor key will clear the line.
    // This functions overloads EditLine::DrawField and outputs '*'s
    // instead of the real line

public:
    PasswordEdit (const String& aItemText, i16 aID, u16 aMaxLen, u16 aFieldLen,
                  WindowItem* NextItem);

    // Derived from class Streamable
    virtual u16 StreamableID () const;
    static Streamable* Build ();

};



inline PasswordEdit::PasswordEdit (const String& aItemText, i16 aID,
                                   u16 aMaxLen, u16 aFieldLen,
                                   WindowItem* NextItem) :
    EditLine (aItemText, aID, aMaxLen, aFieldLen, NextItem)
{
}



inline PasswordEdit::PasswordEdit (StreamableInit) :
    EditLine (Empty)
{
}



/*****************************************************************************/
/*                              class FileEdit                               */
/*****************************************************************************/



class FileEdit: public EditLine {

    friend class ResEditApp;                    // Resource editor is a friend

protected:

    u16         FileFlags;                      //


    virtual int CharIsOk (Key& K);
    // This function is called with the users input to check if the
    // input is valid. The function returns 1 if the key is acceptable,
    // 0 otherwise. Overload this function for restricting input.
    // The function can change the given char!

    virtual int EndCheckOk ();
    // This function is called when the user has ended editing. It returns
    // 1 if the contents of Line are acceptable, 0 if not. In the latter
    // case it's up to this function to display an appropriate error
    // message.

    FileEdit (StreamableInit);
    // Build constructor


public:
    FileEdit (const String& ItemText, i16 ItemID, u16 aMaxLen, u16 aFieldLen,
              u16 aFileFlags, WindowItem* NextItem);

    // Derived from class Streamable
    virtual void Store (Stream&) const;
    virtual void Load (Stream&);
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    virtual void Draw ();
    // Overloads function TEditLine::Draw. Cuts the path if it is to long
    // to be displayed

    u16 GetFileFlags () const;
    // Return the current FileFlags settings

    virtual void SetFileFlags (u16 NewFlags);
    // Change the FileFlags settings

    int PathOk () const;
    // Return the setting from FileFlags

    int WildcardsOk () const;
    // Return the setting from FileFlags

    int ExtensionOk () const;
    // Return the setting from FileFlags

};



inline FileEdit::FileEdit (StreamableInit):
    EditLine (Empty)
{
}



inline u16 FileEdit::GetFileFlags () const
{
    return FileFlags;
}



inline int FileEdit::PathOk () const
// Return the setting from FileFlags
{
    return (FileFlags & ffPathOk) != 0;
}



inline int FileEdit::WildcardsOk () const
// Return the setting from FileFlags
{
    return (FileFlags & ffWildcardsOk) != 0;
}



inline int FileEdit::ExtensionOk () const
// Return the setting from FileFlags
{
    return (FileFlags & ffExtensionOk) != 0;
}



// End of MENUEDIT.H

#endif

