/*****************************************************************************/
/*                                                                           */
/*                                  CPUCVT.H                                 */
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



// This file contains code to convert to and from little endian data types
// into the native format.



#ifndef __CPUCVT_H
#define __CPUCVT_H



#include "machine.h"



/*****************************************************************************/
/*                                   Types                                   */
/*****************************************************************************/



union _double {
    double      F;
    u32         I [2];
};



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



#ifdef CPU_LITTLE_ENDIAN

// We already have little endian format, inline no-op functions
inline void ToLittleEndian (i16&) { }
inline void ToLittleEndian (i32&) { }
inline void ToLittleEndian (u16&) { }
inline void ToLittleEndian (u32&) { }
inline void ToLittleEndian (_double&) { }

inline void FromLittleEndian (i16&) { }
inline void FromLittleEndian (i32&) { }
inline void FromLittleEndian (u16&) { }
inline void FromLittleEndian (u32&) { }
inline void FromLittleEndian (_double&) { }

#else

// We have to convert
void ToLittleEndian (i16& X);
void ToLittleEndian (i32& X);
void ToLittleEndian (u16& X);
void ToLittleEndian (u32& X);
void ToLittleEndian (_double& X);

void FromLittleEndian (i16& X);
void FromLittleEndian (i32& X);
void FromLittleEndian (u16& X);
void FromLittleEndian (u32& X);
void FromLittleEndian (_double& X);

#endif



// End of CPUCVT.H

#endif

