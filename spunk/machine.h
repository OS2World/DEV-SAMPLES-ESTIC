/*****************************************************************************/
/*                                                                           */
/*                                 MACHINE.H                                 */
/*                                                                           */
/* (C) 1993-96  Ullrich von Bassewitz                                        */
/*              Wacholderweg 14                                              */
/*              D-70597 Stuttgart                                            */
/* EMail:       uz@ibb.schwaben.com                                          */
/*                                                                           */
/*****************************************************************************/



//
// $Id$
//
// $Log$
//
//



#ifndef _MACHINE_H
#define _MACHINE_H



/*****************************************************************************/
/*                                    DOS                                    */
/*****************************************************************************/



#ifdef DOS

// Endianess
#define CPU_LITTLE_ENDIAN

// File system supports drives
#define FILESYS_HAS_DRIVES

// data types
typedef int             i16;                    // int with 16 bits
typedef unsigned        u16;                    // unsigned with 16 bits
typedef long            i32;                    // int with 32 bits
typedef unsigned long   u32;                    // unsigned int with 32 bits

#endif



/*****************************************************************************/
/*                   DOS programs using a 32 bit extender                    */
/*****************************************************************************/



#ifdef DOS32

// Endianess
#define CPU_LITTLE_ENDIAN

// File system supports drives
#define FILESYS_HAS_DRIVES

// data types
typedef short int       i16;                    // int with 16 bits
typedef unsigned short  u16;                    // unsigned with 16 bits
typedef int             i32;                    // int with 32 bits
typedef unsigned        u32;                    // unsigned int with 32 bits

#endif



/*****************************************************************************/
/*                                   OS/2                                    */
/*****************************************************************************/



#ifdef OS2

// Endianess
#define CPU_LITTLE_ENDIAN

// File system supports drives
#define FILESYS_HAS_DRIVES

// data types
typedef short int       i16;                    // int with 16 bits
typedef unsigned short  u16;                    // unsigned with 16 bits
typedef int             i32;                    // int with 32 bits
typedef unsigned        u32;                    // unsigned int with 32 bits

#endif



/*****************************************************************************/
/*                                WINDOWS NT                                 */
/*****************************************************************************/



#ifdef NT

// Endianess
#define CPU_LITTLE_ENDIAN

// File system supports drives
#define FILESYS_HAS_DRIVES

// data types
typedef short int       i16;                    // int with 16 bits
typedef unsigned short  u16;                    // unsigned with 16 bits
typedef int             i32;                    // int with 32 bits
typedef unsigned        u32;                    // unsigned int with 32 bits

#endif



/*****************************************************************************/
/*                              Novell Netware                               */
/*****************************************************************************/



#ifdef NETWARE

// Endianess
#define CPU_LITTLE_ENDIAN

// File system supports volumes
#define FILESYS_HAS_VOLUMES

// data types
typedef short int       i16;                    // int with 16 bits
typedef unsigned short  u16;                    // unsigned with 16 bits
typedef int             i32;                    // int with 32 bits
typedef unsigned        u32;                    // unsigned int with 32 bits

#endif



/*****************************************************************************/
/*             Generic Unix - make your own entry after porting              */
/*****************************************************************************/



#ifdef GENERIC_UNIX

// Uncomment one of the following
#define CPU_LITTLE_ENDIAN
// #define CPU_BIG_ENDIAN

// If your machine has no usleep, uncomment the following
// #define DONT_HAS_USLEEP

// data types
typedef short int       i16;                    // int with 16 bits
typedef unsigned short  u16;                    // unsigned with 16 bits
typedef int             i32;                    // int with 32 bits
typedef unsigned        u32;                    // unsigned int with 32 bits

#endif



/*****************************************************************************/
/*                                   Linux                                   */
/*****************************************************************************/



#ifdef LINUX

// Endianess. The following code is not very clean as it accesses a reserved
// implementation identifier, but there is no other way to determine byte
// order under Linux (known to me).
#include <bytesex.h>
#if __BYTE_ORDER == 1234
#    define CPU_LITTLE_ENDIAN
#elif __BYTE_ORDER = 4321
#    define CPU_BIG_ENDIAN
#elif
#    error Byte order not defined!
#endif

// data types
typedef short int       i16;                    // int with 16 bits
typedef unsigned short  u16;                    // unsigned with 16 bits
typedef int             i32;                    // int with 32 bits
typedef unsigned        u32;                    // unsigned int with 32 bits

#endif



/*****************************************************************************/
/*                                    FreeBSD                                */
/*****************************************************************************/



#ifdef FREEBSD

#include <machine/endian.h>
#if (BYTE_ORDER == LITTLE_ENDIAN)
#    define CPU_LITTLE_ENDIAN
#elif (BYTE_ORDER == BIG_ENDIAN)
#    define CPU_BIG_ENDIAN
#elif
#    error Byte order not defined!
#endif

// data types
typedef short int       i16;                    // int with 16 bits
typedef unsigned short  u16;                    // unsigned with 16 bits
typedef int             i32;                    // int with 32 bits
typedef unsigned        u32;                    // unsigned int with 32 bits

#endif



/*****************************************************************************/
/*                                   HP/UX                                   */
/*****************************************************************************/



#ifdef HPUX

#include <machine/param.h>
#ifdef _BIG_ENDIAN
#    define CPU_BIG_ENDIAN
#else
#    define CPU_LITTLE_ENDIAN
#endif

// data types
typedef short int       i16;                    // int with 16 bits
typedef unsigned short  u16;                    // unsigned with 16 bits
typedef int             i32;                    // int with 32 bits
typedef unsigned        u32;                    // unsigned int with 32 bits

#endif



/*****************************************************************************/
/*                              NetBSD (Amiga)                               */
/*****************************************************************************/



#ifdef NETBSD

#include <machine/endian.h>
#if (BYTE_ORDER == LITTLE_ENDIAN)
#    define CPU_LITTLE_ENDIAN
#elif (BYTE_ORDER == BIG_ENDIAN)
#    define CPU_BIG_ENDIAN
#elif
#    error Byte order not defined!
#endif

// data types
typedef short int       i16;                    // int with 16 bits
typedef unsigned short  u16;                    // unsigned with 16 bits
typedef int             i32;                    // int with 32 bits
typedef unsigned        u32;                    // unsigned int with 32 bits

#endif



/*****************************************************************************/
/*                                  Solaris                                  */
/*****************************************************************************/



#ifdef SOLARIS

#include <sys/endian.h>
#ifdef _BIG_ENDIAN
#    define CPU_BIG_ENDIAN
#else
#    define CPU_LITTLE_ENDIAN
#endif

// data types
typedef short int       i16;                    // int with 16 bits
typedef unsigned short  u16;                    // unsigned with 16 bits
typedef int             i32;                    // int with 32 bits
typedef unsigned int    u32;                    // unsigned int with 32 bits

#endif



/*****************************************************************************/
/*               Some other defines that simplify things                     */
/*****************************************************************************/



#if defined (DOS) || defined (DOS32) || defined (OS2) || defined(NT)
#define DOSLIKE_OS
#elif !defined(NETWARE)
#define UNIXLIKE_OS
#endif



// End of MACHINE.H

#endif



