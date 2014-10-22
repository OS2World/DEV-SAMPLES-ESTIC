/*****************************************************************************/
/*									     */
/*				   WINMGR.H				     */
/*									     */
/* (C) 1995-96	Ullrich von Bassewitz					     */
/*		Zwehrenbuehlstrasse 33					     */
/*		D-72070 Tuebingen					     */
/* EMail:	uz@ibb.schwaben.com					     */
/*									     */
/*****************************************************************************/



// $Id$
//
// $Log$
//
//



#ifndef _WINMGR_H
#define _WINMGR_H



#include "keydef.h"
#include "strmable.h"
#include "coll.h"
#include "itemwin.h"



/*****************************************************************************/
/*				     Data				     */
/*****************************************************************************/



extern class WindowManager* WinMgr;



/*****************************************************************************/
/*				 class WinColl				     */
/*****************************************************************************/



class WinColl: public SortedCollection<ItemWindow, u16> {

protected:
    virtual int Compare (const u16* Key1, const u16* Key2);
    virtual const u16* KeyOf (const ItemWindow* Item);

public:
    WinColl (unsigned MaxWindows);
    // Construct a WinColl

    WinColl (StreamableInit);
    // Build constructor

    virtual u16 StreamableID () const;
    // Return the objects stream ID

    static Streamable* Build ();
    // Return an empty window

    int FindWindow (u16 Num);
    // Return the index of the window with number Num

    int FindWindow (const Window* Win);
    // Return the index of the window Win

};



/*****************************************************************************/
/*			       class WindowManager			     */
/*****************************************************************************/



class WindowManager: public Streamable {

protected:
    WinColl*		Coll;
    u16			MaxWindows;

    void VerticalTile (Rect Bounds, unsigned Start, unsigned End);
    // Tile the windows with indices Start to end vertically in the given
    // rectangle Bounds

    int CloseWindow (unsigned Index);
    // Close the window with the given Index. Return true if the window has been
    // closed, return false if the window refuses to get closed

    Key GetWindowKey (const ItemWindow* W);
    // Return the hotkey for the given window

public:
    WindowManager (u16 aMaxWindows = 9);
    // Construct a window manager

    WindowManager (StreamableInit);
    // Build constructor

    ~WindowManager ();
    // Delete the window manager instance

    virtual void Load (Stream& S);
    // Load the object from a stream

    virtual void Store (Stream& S) const;
    // Store the object into a stream

    virtual u16 StreamableID () const;
    // Return the objects stream ID

    static Streamable* Build ();
    // Return an empty WindowManager object

    unsigned GetFreeWindowNumber ();
    // Get the next free window number. If 9 windows are already open, print
    // an error message and return 0

    void DeleteWindow (u16 Num);
    // Delete a window

    void DeleteWindow (ItemWindow* Win);
    // Delete a window

    ItemWindow* AddWindow (ItemWindow* Win);
    // Add a window to the list. The window gets assigned  an unused number.
    // If there are too many windows open, NULL is returned but the window
    // is _not_ deleted. If all is ok, the pointer to the window just
    // inserted is returned.

    ItemWindow* FindWindow (unsigned WindowNum);
    // Find a window by number

    ItemWindow* FindWindowWithKey (Key WindowKey);
    // Find a window by hotkey

    ItemWindow* ChooseWindow ();
    // Choose a new active window

    void Browse (ItemWindow* W);
    // Allow browsing the windows

    void Tile ();
    // Tile all windows on the screen

    void Cascade ();
    // Build a window cascade on the screen

    void CloseAll ();
    // Close all windows

    unsigned WindowCount () const;
    // Return the count of windows

    ItemWindow* GetTopWindow ();
    // Return the uppermost window or NULL (if there are no windows)

    int CanClose ();
    // Return true if all windows answer yes to CanClose, false otherwise
};



inline void WindowManager::DeleteWindow (ItemWindow* Win)
// Delete a window
{
    DeleteWindow (Win->GetWindowNumber ());
}



inline unsigned WindowManager::WindowCount () const
// Return the count of windows
{
    return Coll->GetCount ();
}



// End of WINMGR.H

#endif
