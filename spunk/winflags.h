/*****************************************************************************/
/*                                                                           */
/*                                 WINFLAGS.H                                */
/*                                                                           */
/* (C) 1993-95  Ullrich von Bassewitz                                        */
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



#ifndef _WINFLAGS_H
#define _WINFLAGS_H



#include "machine.h"



// Flags
const u16 wfFramed              = 0x0001;       // Window is framed
const u16 wfActive              = 0x0002;       // Window is active
const u16 wfVisible             = 0x0004;       // Window is visible
const u16 wfSaveVisible         = 0x0008;       // Save visible flag
const u16 wfModal               = 0x0010;       // Window ignores reserved keys
const u16 wfIgnoreAccept        = 0x0020;       // Windows ignores accept key
const u16 wfDirty               = 0x0040;       // Write occured since last lock
const u16 wfLRLink              = 0x0080;       // Left/right cursor movement
const u16 wfResizing            = 0x0100;       // Resizing in progress
const u16 wfCanMove             = 0x0200;       // User is allowed to move
const u16 wfCanResize           = 0x0400;       // User is allowed to resize

// Mask for flags that are written out
const u16 wfSaveFlags           = wfFramed       |
                                  wfSaveVisible  |
                                  wfModal        |
                                  wfIgnoreAccept |
                                  wfLRLink       |
                                  wfCanMove      |
                                  wfCanResize;

// End of WINFLAGS.H

#endif

