/*****************************************************************************/
/*                                                                           */
/*                                 STATLINE.H                                */
/*                                                                           */
/* (C) 1993,94  Ullrich von Bassewitz                                        */
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



#ifndef __STATLINE_H
#define __STATLINE_H



#include "machine.h"
#include "object.h"
#include "statflag.h"
#include "listnode.h"
#include "strmable.h"
#include "stream.h"
#include "rect.h"
#include "stack.h"
#include "window.h"




/*****************************************************************************/
/*                            class StatusLine                               */
/*****************************************************************************/



class StatusLine : public Window {

private:
    void Init (const String& FirstLine);
    // Used in the constructors

protected:
    Stack<String*>      Lines;
    String*             CurrentLine;


    StatusLine (StreamableInit);
    // Build constructor

public:
    StatusLine (const String& FirstLine = "");
    StatusLine (u32 StdFlags);
    virtual ~StatusLine ();

    // Derived from class Streamable
    virtual void Store (Stream&) const;
    virtual void Load (Stream&);
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    // Derived from class Window
    virtual void DrawInterior ();

    // New member functions
    void Push (const String& NewLine);
    void Push (u32 StdFlags);
    void Pop ();
    void Replace (const String& NewLine);
    void Replace (u32 StdFlags);

    static String CreateLine (u32 StdFlags);
    // Create a statusline text according to the flags in StdFlags

};



inline StatusLine::StatusLine (StreamableInit) :
    Window (Empty),
    Lines (Empty)
{
}



/*****************************************************************************/
/*                          class BottomStatusLine                           */
/*****************************************************************************/



class BottomStatusLine: public StatusLine {

protected:
    virtual void ScreenSizeChanged (const Rect& NewScreen);
    // Called when the screen got another resolution. NewScreen is the new
    // screen size.

    BottomStatusLine (StreamableInit);
    // Build constructor

public:
    BottomStatusLine (const String& FirstLine);
    BottomStatusLine (u32 StdFlags);
    BottomStatusLine ();

};



inline BottomStatusLine::BottomStatusLine (StreamableInit) :
    StatusLine (Empty)
{
}



inline BottomStatusLine::BottomStatusLine (const String& FirstLine) :
        StatusLine (FirstLine)
{
}



inline BottomStatusLine::BottomStatusLine (u32 StdFlags) :
        StatusLine (StdFlags)
{
}



inline BottomStatusLine::BottomStatusLine () :
        StatusLine ()
{
}



// End of STATLINE.H

#endif
