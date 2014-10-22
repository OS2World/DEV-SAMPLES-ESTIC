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



#define INCL_NOPMAPI
#define INCL_DOS
#define INCL_KBD
#include <os2.h>
#undef KbdPeek				// We are using the name somewhere

#include "msgid.h"
#include "program.h"
#include "kbd.h"



/*****************************************************************************/
/*				     Data				     */
/*****************************************************************************/


// The one and only keyboard instance
Keyboard* Kbd;



// The keyboard handle
static HKBD	KbdHandle;



// Mapper table from virtual to extended keys. This table is fixed for DOS/OS2
static struct {
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
/*				     Code				     */
/*****************************************************************************/



static int KbdCharAvail ()
// Return true if a char is available from the os
{
    KBDKEYINFO Info;
    Kbd16Peek (&Info, KbdHandle);
    return (Info.fbStatus & KBDTRF_FINAL_CHAR_IN) ? 1 : 0;
}



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
    KBDINFO K;

    // Open the keyboard
    CHECK (KbdOpen (&KbdHandle) == 0);
    CHECK (KbdGetFocus (IO_WAIT, KbdHandle) == 0);
    K.cb     = 10;
    K.fsMask = 0x04;
    CHECK (KbdSetStatus (&K, 0) == 0);
}



Keyboard::~Keyboard ()
{
    CHECK (KbdFreeFocus (KbdHandle) == 0);
    CHECK (KbdClose (KbdHandle) == 0);
    delete [] TransTable;
}



Key Keyboard::RawKey ()
// Get a raw (unmapped) key from the os
{
    // Wait for a key calling repeatedly App->Idle ()
    int I = 0;
    while (KbdCharAvail () == 0) {
	DosSleep (20);
	if (I == 0) {
	    CurThread () -> Idle ();
	}
	if (++I == 5) {
	    I = 0;
	}
    }

    // Get the key
    KBDKEYINFO Info;
    KbdCharIn (&Info, IO_WAIT, KbdHandle);
    Key K = Key ((Info.chScan << 8) + Info.chChar);

    // Translate char/scancode to plain/extended key
    if ((K & 0x00FF) == 0xE0 || (K & 0x00FF) == 0x00) {
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


