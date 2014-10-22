/*****************************************************************************/
/*                                                                           */
/*                                  SCREEN.H                                 */
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



#ifndef _SCREEN_H
#define _SCREEN_H



#include "machine.h"
#include "object.h"
#include "rect.h"
#include "scrmodes.h"



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



// Instance of the screen class to handle screen output. Must be initialized
// from outside (RootWindow)
extern class Screen* TheScreen;



/*****************************************************************************/
/*                               class Screen                                */
/*****************************************************************************/



class Screen: public Object {

private:
    u16                 XSize;
    u16                 YSize;
    u16                 CurrentMode;    // Current video mode
    int                 Color;          // 1 if color mode, 0 if not
    int                 Console;        // 1 if console screen, 0 if not
    int                 CP437;          // 1 if CP437 should be used
    unsigned char*      TransTable;     // Translation table for output

    u16                 StartupMode;    // Mode before app was active
    u16                 StartupCursor;  // Cursor when app became active

    u16                 CF_HiddenCursor;
    u16                 CF_NormalCursor;
    u16                 CF_FatCursor;

    void TCInit ();
    // Initialize the termcap system. Used for *nixen only.

    char* GetIS (char* IS);
    char* GetRS (char* RS);
    // Return a replacement for the init strings IS and RS. Used for *nixen
    // only.

    void SetModeData ();
    // Internally called after setting a new mode, sets cursor data etc.

    unsigned char Translate (unsigned char C);
    // Translate the char via the translation table

    u16* Translate (u16* Target, u16* Source, unsigned Len);
    // Translate a complete buffer via the translation table

public:
    Screen ();
    ~Screen ();

    // Put a rectangular buffer region onto the screen
    void DisplayBuffer (const Rect& R, u16* Buf);

    // Video mode
    void SetMode (u16 Mode);
    u16 GetMode () const;
    int IsColor () const;
    int IsConsole () const;

    // Cursor
    void SetCursorOn ();
    void SetCursorOff ();
    void SetCursorFat ();
    void SetCursorPos (const Point& Pos);

    // Screen size
    u16 GetXSize () const;
    u16 GetYSize () const;
    Rect GetSize () const;

    unsigned TerminalSpeed ();
    // Get some information on the terminal speed. This will return a value
    // between 0..10, where 10 is a very fast (direct access) screen and
    // 0 is a very slow (300 baud) serial line.
    // This may be used to change the amount of screen output, a program
    // produces.

    inline void SetTransTable (unsigned char* NewTable);
    // Set a new translation table. The old table is deleted. NewTable can
    // be NULL to clear the table.

};



inline unsigned char Screen::Translate (unsigned char C)
// Translate the char via the translation table
{
    return TransTable? TransTable [C] : C;
}



inline u16 Screen::GetXSize () const
{
    return XSize;
}



inline u16 Screen::GetYSize () const
{
    return YSize;
}



inline Rect Screen::GetSize () const
{
    return Rect (0, 0, XSize, YSize);
}



inline u16 Screen::GetMode () const
{
    return CurrentMode;
}



inline int Screen::IsColor () const
{
    return Color;
}



inline int Screen::IsConsole () const
{
    return Console;
}



inline void Screen::SetTransTable (unsigned char* NewTable)
// Set a new translation table. The old table is deleted. NewTable can be NULL
// to clear the table.
{
    delete [] TransTable;
    TransTable = NewTable;
}



// End of SCREEN.H

#endif

