/*****************************************************************************/
/*									     */
/*				      KBD.CC				     */
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



// This file is also used for the 32-bit version of the library. The define
// DOS32 is used to control the compile target. The functions in the first
// section (labeled "Target specific code") are the only ones that are
// target specific.
// Note: Because the Borland and Watcom compilers use different notations
// when accessing the word registers in a REGS strcuture, all assignments
// to those registers are splitted in two byte assignments. If you change
// this, the module will no longer compatible with all supported compilers.



#if defined (__WATCOMC__)
#include <i86.h>
#include <conio.h>
#elif defined (__BORLANDC__) || defined (__GO32__)
#include <dos.h>
#endif

#include "msgid.h"
#include "program.h"
#include "kbd.h"



/*****************************************************************************/
/*				     Data				     */
/*****************************************************************************/



// The one and only keyboard instance
Keyboard* Kbd;



// Mapper table from virtual to extended keys. This table is fixed for DOS/OS2
struct {
    Key EK;
    Key VK;
} VirtualMap [] = {
    {	kbEsc,		vkAbort		},
    {	kbF1,		vkHelp		},
    {	kbF10,		vkAccept	},
    {	kbPgUp,		vkPgUp		},
    {	kbPgDn,		vkPgDn		},
    {	kbCtrlPgUp,	vkCtrlPgUp	},
    {	kbCtrlPgDn,	vkCtrlPgDn	},
    {	kbUp,		vkUp		},
    {	kbDown,		vkDown		},
    {	kbLeft,		vkLeft		},
    {	kbRight,	vkRight		},
    {	kbIns,		vkIns		},
    {	kbDel,		vkDel		},
    {	kbHome,		vkHome		},
    {	kbEnd,		vkEnd		},
    {	kbCtrlUp,	vkCtrlUp	},
    {	kbCtrlDown,	vkCtrlDown	},
    {	kbCtrlLeft,	vkCtrlLeft	},
    {	kbCtrlRight,	vkCtrlRight	},
    {	kbCtrlIns,	vkCtrlIns	},
    {	kbCtrlDel,	vkCtrlDel	},
    {	kbCtrlHome,	vkCtrlHome	},
    {	kbCtrlEnd,	vkCtrlEnd	},
    {	kbF5,		vkZoom		},
    {	kbMetaF3,	vkClose		},
    {	kbF3,		vkOpen		},
    {	kbF2,		vkSave		},
    {	kbCtrlF5,	vkResize	},
    {	kbMetaX,	vkQuit		},

    // Secondary mappings follow
    {	kbCtrlR,	vkPgUp		},
    {	kbCtrlC,	vkPgDn		},
    {	kbCtrlE,	vkUp		},
    {	kbCtrlX,	vkDown		},
    {	kbCtrlS,	vkLeft		},
    {	kbCtrlD,	vkRight		},
    {	kbCtrlV,	vkIns		},
    {	kbCtrlG,	vkDel		},
    {	kbCtrlW,	vkCtrlUp	},
    {	kbCtrlZ,	vkCtrlDown	},
    {	kbCtrlA,	vkCtrlLeft	},
    {	kbCtrlF,	vkCtrlRight	},

};



/*****************************************************************************/
/*			     Target specific code			     */
/*****************************************************************************/



inline void KbdInt (REGS& Regs)
// execute a software interrupt
{
#if defined (DOS32) && defined (__WATCOMC__)
    int386 (0x16, &Regs, &Regs);
#else
    int86 (0x16, &Regs, &Regs);
#endif
}



#ifdef __WATCOMC__
// Watcom-C does not allow to access the flags register via int86(), so use
// inline assembly

extern short KbdCharAvail ();
// Return 1 if a key is available
#pragma aux KbdCharAvail =	\
    "   mov     ah, 11h"	\
    "   int     16h"		\
    "   mov     ax, 0"		\
    "   jz      L1"		\
    "   inc     ax"		\
    "L1:"			\
    value [ax]

#else

static int KbdCharAvail ()
// Return 1 if a key is available
{
    REGS Regs;
    Regs.h.ah = 0x11;
    KbdInt (Regs);

    // A key is available if the zero flag is not set
    return (Regs.x.flags & 0x40) == 0;
}

#endif



static void GiveUpTimeslice ()
// Give up the remainder of the current timeslice. Multiplex interrupt works
// under Windows and OS/2
{
    REGS Regs;
#if defined (DOS32) && defined (__WATCOMC__)
    Regs.w.ax = 0x1680;
    int386 (0x2F, &Regs, &Regs);
#else
    Regs.x.ax = 0x1680;
    int86 (0x2F, &Regs, &Regs);
#endif
}



/*****************************************************************************/
/*				     Code				     */
/*****************************************************************************/



static Key KbdMapExtended (Key K)
// Map an extended key to a virtual key. Return the virtual key if a map
// exists, otherwise return the key unchanged.
{
    for (int I = 0; I < sizeof (VirtualMap) / sizeof (VirtualMap [0]); I++) {
	if (VirtualMap [I].EK == K) {
	    return VirtualMap [I].VK;
	}
    }
    return K;
}



static Key KbdMapVirtual (Key K)
// Map a virtual key to an extended key. Return the extended key if a map
// exists, otherwise return the key unchanged.
{
    for (int I = 0; I < sizeof (VirtualMap) / sizeof (VirtualMap [0]); I++) {
	if (VirtualMap [I].VK == K) {
	    return VirtualMap [I].EK;
	}
    }
    return K;
}



/*****************************************************************************/
/*				Class Keyboard				     */
/*****************************************************************************/



Keyboard::Keyboard ():
    Console (1),
    TransTable (NULL)
{
}



Keyboard::~Keyboard ()
{
    delete [] TransTable;
}



Key Keyboard::RawKey ()
// Get a raw (unmapped) key from the pc bios
{
    REGS Regs;

    // Wait for a key calling App->Idle
    while (KbdCharAvail () == 0) {
	GiveUpTimeslice ();
	CurThread () -> Idle ();
    }

    // Retrieve the key
    Regs.h.ah = 0x10;
    KbdInt (Regs);

    // Remap some keys
    Key K = (unsigned (Regs.h.ah) << 8) | Regs.h.al;

    // Translate char/scancode to plain/extended key
    if ( (K & 0x00FF) == 0xE0 || (K & 0x00FF) == 0x00) {
	// Extended character
	return Key (0x0100 | (K >> 8));
    } else {
	return Key (K & 0x00FF);
    }
}



void Keyboard::GetMappedKey (int Wait)
// Read keys until the needed key is not found in the mapper or until
// a valid sequence is found. If Wait is zero, return immediately if
// no match is found and no more keys are available.
{
    // If waiting is not wanted, check if there are keys available before
    // grabbing them
    if (Wait == 0 && KbdCharAvail () == 0) {
	// No keys available, bail out
	return;
    }

    // Get a key, remap it and put it into the buffer
    KeyBuf.Put (KbdMapExtended (Translate (RawKey ())));
}



String Keyboard::GetKeyName (Key K)
// Return a string describing the give key
{
    if (IsVirtualKey (K)) {
	// It is a virtual key, remap it to it's extended form
	K = KbdMapVirtual (K);
    }

    // Now return the key description
    return App->LoadMsg (MSGBASE_KBD + K);
}


