/*****************************************************************************/
/*									     */
/*				    CPUCVT.CC				     */
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



// This file contains code to convert to and from little endian data types
// into the native format.



#include "cpucvt.h"



/*****************************************************************************/
/*				     Code				     */
/*****************************************************************************/



#ifdef CPU_BIG_ENDIAN



void ToLittleEndian (i16& X)
{
    X = ((X << 8) & 0xFF00) |
	((X >> 8) & 0x00FF);
}



void ToLittleEndian (i32& X)
{
    X = ((X >> 24) & 0x000000FF) |
	((X >>	8) & 0x0000FF00) |
	((X <<	8) & 0x00FF0000) |
	((X << 24) & 0xFF000000);
}



void ToLittleEndian (u16& X)
{
    X = ((X << 8) & 0xFF00) |
	((X >> 8) & 0x00FF);
}



void ToLittleEndian (u32& X)
{
    X = ((X >> 24) & 0x000000FF) |
	((X >>	8) & 0x0000FF00) |
	((X <<	8) & 0x00FF0000) |
	((X << 24) & 0xFF000000);
}



void ToLittleEndian (_double& X)
{
    // Swap the longs
    u32 Tmp = X.I [0];
    X.I [0] = X.I [1];
    X.I [1] = Tmp;

    // Swap the bytes inside the longs
    ToLittleEndian (X.I [0]);
    ToLittleEndian (X.I [1]);
}



void FromLittleEndian (i16& X)
{
    X = ((X << 8) & 0xFF00) |
	((X >> 8) & 0x00FF);
}



void FromLittleEndian (i32& X)
{
    X = ((X >> 24) & 0x000000FF) |
	((X >>	8) & 0x0000FF00) |
	((X <<	8) & 0x00FF0000) |
	((X << 24) & 0xFF000000);
}



void FromLittleEndian (u16& X)
{
    X = ((X << 8) & 0xFF00) |
	((X >> 8) & 0x00FF);
}



void FromLittleEndian (u32& X)
{
    X = ((X >> 24) & 0x000000FF) |
	((X >>	8) & 0x0000FF00) |
	((X <<	8) & 0x00FF0000) |
	((X << 24) & 0xFF000000);
}



void FromLittleEndian (_double& X)
{
    // Swap the longs
    u32 Tmp = X.I [0];
    X.I [0] = X.I [1];
    X.I [1] = Tmp;

    // Swap the bytes inside the longs
    FromLittleEndian (X.I [0]);
    FromLittleEndian (X.I [1]);
}



#endif

