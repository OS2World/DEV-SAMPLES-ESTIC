/*****************************************************************************/
/*									     */
/*				      IMON.H				     */
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



// This module implements a simple isdn4linux line monitor



#ifndef _ICIMON_H
#define _ICIMON_H



#include <stdio.h>

#include "itemwin.h"



/*****************************************************************************/
/*				 class IMonWindow			     */
/*****************************************************************************/



class IMonWindow: public ItemWindow {

private:
    // Types of lines read from isdninfo
    enum {
	li_idmap,
	li_chmap,
	li_drmap,
	li_usage,
	li_flags,
	li_phone,
	li_count
    };

    int Status;
    // Status of the window

    static unsigned WindowCount;
    // Count of IMonWindows

    FILE*	F;
    // /dev/isdninfo

    Rect	ZoomSize;
    // Small window size for zooming

public:
    IMonWindow (StreamableInit);
    // Build constructor

    IMonWindow (const Point& Pos);
    // Construct an IMonWindow

    virtual ~IMonWindow ();
    // Destruct an IMonWindow

    virtual void Store (Stream &) const;
    virtual void Load (Stream &);
    virtual u16 StreamableID () const;
    static Streamable* Build ();
    // Make the window persistent

    virtual unsigned MinXSize () const;
    // Return the minimum X size of the window. Override this to limit resizing.

    virtual unsigned MinYSize () const;
    // Return the minumim Y size of the window. Override this to limit resizing.

    virtual void Zoom ();
    // Zoom the window

    void Update ();
    // Update the window if information has changed

    void HandleEvent (Event& E);
    // Handle incoming events. Calls Update() if the application is idle

};



// End of ICIMON.H

#endif


