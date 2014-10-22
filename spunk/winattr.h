/*****************************************************************************/
/*									     */
/*				   WINATTR.H				     */
/*									     */
/* (C) 1993-95	Ullrich von Bassewitz					     */
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



#ifndef __WINATTR_H
#define __WINATTR_H



#include "strmable.h"
#include "coll.h"



/*****************************************************************************/
/*				 Charset stuff				     */
/*****************************************************************************/



// Constants for use with the frame char arrays
static const unsigned fcTopLeft     =  0;
static const unsigned fcTopRight    =  1;
static const unsigned fcBotLeft     =  2;
static const unsigned fcBotRight    =  3;
static const unsigned fcHorizontal  =  4;
static const unsigned fcVertical    =  5;
static const unsigned fcLeftTee     =  6;
static const unsigned fcRightTee    =  7;
static const unsigned fcTopTee	    =  8;
static const unsigned fcBotTee	    =  9;
static const unsigned fcCross	    = 10;

// Characters for use in window frames etc. (these are in frame.cc and
// initialized to CP850/CP437)
extern unsigned char* ActiveFrame;
extern unsigned char* InactiveFrame;

// Other window frame arrays that may be used instead of the default ones
// (these are also in frame.cc)
extern unsigned char CP850_InactiveFrame [];
extern unsigned char CP850_ActiveFrame [];
extern unsigned char KOI8r_InactiveFrame [];
extern unsigned char KOI8r_ActiveFrame [];
extern unsigned char SimpleFrame [];

// Some other useful characters. These must not be in the range 0x20..0x7F!
const unsigned char LeftArrow		=	0x1B;
const unsigned char RightArrow		=	0x1A;
const unsigned char UpArrow		=	0x18;
const unsigned char DownArrow		=	0x19;
const unsigned char LeftTriangle	=	0x11;
const unsigned char RightTriangle	=	0x10;
const unsigned char UpTriangle		=	0x1E;
const unsigned char DownTriangle	=	0x1F;



/*****************************************************************************/
/*			     Module initialization			     */
/*****************************************************************************/



void InitWinAttr ();
// Initialize window attributes



void DoneWinAttr ();
// Clean up after use



// End of WINATTR.H

#endif


