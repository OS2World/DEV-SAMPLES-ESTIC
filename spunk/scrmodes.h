/*****************************************************************************/
/*                                                                           */
/*                                 SCRMODES.H                                */
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



#ifndef __SCRMODES_H
#define __SCRMODES_H



#include "machine.h"



/*****************************************************************************/
/*                           Video mode constants                            */
/*****************************************************************************/


static const u16 vmBW40         = 0x0000;
static const u16 vmCO40         = 0x0001;
static const u16 vmBW80         = 0x0002;
static const u16 vmCO80         = 0x0003;
static const u16 vmMono         = 0x0007;

// Extended modes
static const u16 vmVGA_80x25    = vmCO80;
static const u16 vmVGA_80x30    = 0x0101;
static const u16 vmVGA_80x34    = 0x0102;
static const u16 vmVGA_80x43    = 0x0103;
static const u16 vmVGA_80x50    = 0x0104;
static const u16 vmVGA_80x60    = 0x0105;
static const u16 vmVGA_94x25    = 0x0110;
static const u16 vmVGA_94x30    = 0x0111;
static const u16 vmVGA_94x34    = 0x0112;
static const u16 vmVGA_94x43    = 0x0113;
static const u16 vmVGA_94x50    = 0x0114;
static const u16 vmVGA_94x60    = 0x0115;

// Card specific modes
static const u16 vmET4_100x40   = 0x012A;

// Ask the hardware for the current mode (*nix, X Window)
static const u16 vmAsk          = 0x0200;

// The "don't know" video mode
static const u16 vmInvalid      = 0xFFFF;



// End of SCRMODES.H

#endif

