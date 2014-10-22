/*****************************************************************************/
/*									     */
/*				      KBD.CC				     */
/*									     */
/* (C) 1996	Ullrich von Bassewitz					     */
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



#include <process.h>
#include <conio.h>

#include "msgid.h"
#include "program.h"
#include "kbd.h"



/*****************************************************************************/
/*				     Data				     */
/*****************************************************************************/



// The one and only keyboard instance
Keyboard* Kbd;



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
    return kbhit ();
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
}



Keyboard::~Keyboard ()
{
    delete [] TransTable;
}



Key Keyboard::RawKey ()
// Get a raw (unmapped) key from the os
{
    // Wait for a key calling repeatedly App->Idle ()
    while (KbdCharAvail () == 0) {

	// Allow idle processing
	CurThread () -> Idle ();

	// Allow other threads to run
	ThreadSwitch ();
    }

    // Get the key
    Key K = getch ();
    if (K == 0) {
	// Extended key
	return (0x0100 | (Key) getch ());
    } else {
	return (K & 0x00FF);
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


