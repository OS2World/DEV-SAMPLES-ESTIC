/*****************************************************************************/
/*                                                                           */
/*                                  KBD.CC                                   */
/*                                                                           */
/*                      (C) 1996 MU Softwareentwicklung                      */
/*                                                                           */
/*      Ullrich von Bassewitz                          Michael Peschel       */
/*      Wacholderweg 14                                   Ledergasse 3       */
/*      D-70597 Stuttgart                            D-72555 Metzingen       */
/*      uz@ibb.schwaben.com                      mipe@ibb.schwaben.com       */
/*                                                                           */
/*****************************************************************************/



#include <signal.h>
#include <windows.h>

#include "msgid.h"
#include "program.h"
#include "kbd.h"



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



// The one and only keyboard instance
Keyboard* Kbd;


// The keyboard handle
static HANDLE KbdHandle;



// Mapper table from virtual to extended keys. This table is fixed for DOS/OS2
static struct {
    Key EK;
    Key VK;
} VirtualMap [] = {
    {   kbEsc,          vkAbort         },
    {   kbF1,           vkHelp          },
    {   kbF10,          vkAccept        },
    {   kbPgUp,         vkPgUp          },
    {   kbPgDn,         vkPgDn          },
    {   kbCtrlPgUp,     vkCtrlPgUp      },
    {   kbCtrlPgDn,     vkCtrlPgDn      },
    {   kbUp,           vkUp            },
    {   kbDown,         vkDown          },
    {   kbLeft,         vkLeft          },
    {   kbRight,        vkRight         },
    {   kbIns,          vkIns           },
    {   kbDel,          vkDel           },
    {   kbHome,         vkHome          },
    {   kbEnd,          vkEnd           },
    {   kbCtrlUp,       vkCtrlUp        },
    {   kbCtrlDown,     vkCtrlDown      },
    {   kbCtrlLeft,     vkCtrlLeft      },
    {   kbCtrlRight,    vkCtrlRight     },
    {   kbCtrlIns,      vkCtrlIns       },
    {   kbCtrlDel,      vkCtrlDel       },
    {   kbCtrlHome,     vkCtrlHome      },
    {   kbCtrlEnd,      vkCtrlEnd       },
    {   kbF5,           vkZoom          },
    {   kbMetaF3,       vkClose         },
    {   kbF3,           vkOpen          },
    {   kbF2,           vkSave          },
    {   kbCtrlF5,       vkResize        },
    {   kbMetaX,        vkQuit          },

    // Secondary mappings follow
    {   kbCtrlR,        vkPgUp          },
    {   kbCtrlC,        vkPgDn          },
    {   kbCtrlE,        vkUp            },
    {   kbCtrlX,        vkDown          },
    {   kbCtrlS,        vkLeft          },
    {   kbCtrlD,        vkRight         },
    {   kbCtrlV,        vkIns           },
    {   kbCtrlG,        vkDel           },
    {   kbCtrlW,        vkCtrlUp        },
    {   kbCtrlZ,        vkCtrlDown      },
    {   kbCtrlA,        vkCtrlLeft      },
    {   kbCtrlF,        vkCtrlRight     },

};



// Tables to map the virtual keys

// keys with ALT-modifier
static const u16 MetaVKeyTab [0x80] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, kbMetaTab, 0, 0, 0, 0, 0, 0,     // 0x00 - 0x0F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, kbMetaEsc, 0, 0, 0, 0,     // 0x10 - 0x1F

    kbMetaSpace, kbMetaPgUp, kbMetaPgDn, kbMetaEnd,             // 0x20 - 0x2F
    kbMetaHome,  kbMetaLeft, kbMetaUp,   kbMetaRight,
    kbMetaDown,  0,          0,          0,
    0,           kbMetaIns,  kbMetaDel,  0,

    kbMeta0, kbMeta1, kbMeta2, kbMeta3,                         // 0x30 - 0x3F
    kbMeta4, kbMeta5, kbMeta6, kbMeta7,
    kbMeta8, kbMeta9, 0,       0,
    0,       0,       0,       0,

    0,       kbMetaA, kbMetaB, kbMetaC,                         // 0x40 - 0x4F
    kbMetaD, kbMetaE, kbMetaF, kbMetaG,
    kbMetaH, kbMetaI, kbMetaJ, kbMetaK,
    kbMetaL, kbMetaM, kbMetaN, kbMetaO,

    kbMetaP, kbMetaQ, kbMetaR, kbMetaS,                         // 0x60 - 0x5F
    kbMetaT, kbMetaU, kbMetaV, kbMetaW,
    kbMetaX, kbMetaY, kbMetaZ, 0,
    0,       0,       0,       0,

    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,             // 0x60 - 0x6F

    kbMetaF1, kbMetaF2,  kbMetaF3,  kbMetaF4,                   // 0x70 - 0x7F
    kbMetaF5, kbMetaF6,  kbMetaF7,  kbMetaF8,
    kbMetaF9, kbMetaF10, kbMetaF11, kbMetaF12,
    0,        0,         0,         0,
};


// keys with Ctrl-modifier
static const u16 CtrlVKeyTab [0x80] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, kbCtrlTab, 0, 0, 0, 0, 0, 0,     // 0x00 - 0x0F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,         0, 0, 0, 0, 0, 0,     // 0x10 - 0x1F

    0,          kbCtrlPgUp, kbCtrlPgDn, kbCtrlEnd,              // 0x20 - 0x2F
    kbCtrlHome, kbCtrlLeft, kbCtrlUp,   kbCtrlRight,
    kbCtrlDown, 0,          0,          0,
    0,          kbCtrlIns,  kbCtrlDel,  0,

    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,             // 0x30 - 0x3F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,             // 0x40 - 0x4F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,             // 0x50 - 0x5F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,             // 0x60 - 0x6F

    kbCtrlF1, kbCtrlF2,  kbCtrlF3,  kbCtrlF4,                   // 0x70 - 0x7F
    kbCtrlF5, kbCtrlF6,  kbCtrlF7,  kbCtrlF8,
    kbCtrlF9, kbCtrlF10, kbCtrlF11, kbCtrlF12,
    0,        0,         0,         0,
};


// keys with Shift-modifier
static const u16 ShiftVKeyTab [0x80] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, kbShiftTab, 0, 0, 0, 0, 0, 0,    // 0x00 - 0x0F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,             // 0x10 - 0x1F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, kbShiftIns, kbShiftDel, 0, // 0x20 - 0x2F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,             // 0x30 - 0x3F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,             // 0x40 - 0x4F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,             // 0x50 - 0x5F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,             // 0x60 - 0x6F
    kbShiftF1, kbShiftF2,  kbShiftF3,  kbShiftF4,               // 0x70 - 0x7F
    kbShiftF5, kbShiftF6,  kbShiftF7,  kbShiftF8,
    kbShiftF9, kbShiftF10, kbShiftF11, kbShiftF12,
    0,         0,          0,          0,
};


// normal keys
static const u16 NormalVKeyTab [0x80] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, kbTab, 0, 0, 0, 0, 0, 0,         // 0x00 - 0x0F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0,         // 0x10 - 0x1F

    0,      kbPgUp, kbPgDn, kbEnd,                              // 0x20 - 0x2F
    kbHome, kbLeft, kbUp,   kbRight,
    kbDown, 0,      0,      0,
    0,      kbIns,  kbDel,  0,

    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,             // 0x30 - 0x3F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,             // 0x40 - 0x4F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,             // 0x50 - 0x5F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,             // 0x60 - 0x6F

    kbF1, kbF2,  kbF3,  kbF4,                                   // 0x70 - 0x7F
    kbF5, kbF6,  kbF7,  kbF8,
    kbF9, kbF10, kbF11, kbF12,
    0,    0,     0,     0,
};



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



static inline int IsMetaKey (u32 Flags)
{
    return (Flags & LEFT_ALT_PRESSED) != 0;
}



static inline int IsCtrlKey (u32 Flags)
{
    return ((Flags & LEFT_CTRL_PRESSED) || (Flags & RIGHT_CTRL_PRESSED));
}



static inline int IsShiftKey (u32 Flags)
{
    return (Flags & SHIFT_PRESSED) != 0;
}



u16 TranslateVKey (u16 Key, u32 Flags)
{
    // only keys below 0x80 are supported
    if (Key > 0x7f) {
        return 0;
    }

    if (IsMetaKey (Flags)) {
        return MetaVKeyTab [Key];
    }

    if (IsCtrlKey (Flags)) {
        return CtrlVKeyTab [Key];
    }

    if (IsShiftKey (Flags)) {
        return ShiftVKeyTab [Key];
    }

    return NormalVKeyTab [Key];
}



static int IsSpecialKey (u16 Code)
// returns true if the specified key has a special meaning
{
    return ((Code >= VK_F1)    && (Code <= VK_F12)) ||    // function keys
           ((Code >= VK_PRIOR) && (Code <= VK_DOWN)) ||   // cursor keys
            (Code == VK_INSERT) ||
            (Code == VK_DELETE) ||
            (Code == VK_TAB);
}



// the nect available key
static u16 InKey    = 0;



static int KbdCharAvail ()
// Return true if a char is available from the os
{
    // if there is a key currently not read, return immediatly
    if (InKey != 0) {
        return 1;
    }

    // examine input queue, until a key is available or the queue is empty
    while (InKey == 0) {
        // check input queue
        DWORD NumRead;
        INPUT_RECORD Info;
        PeekConsoleInput (KbdHandle, &Info, 1, &NumRead);

        // queue is empty
        if (NumRead == 0) {
            break;
        }

        // get event
        ReadConsoleInput (KbdHandle, &Info, 1, &NumRead);

        // first check if screen resizing
        if (Info.EventType == WINDOW_BUFFER_SIZE_EVENT) {

           // raise signal
           raise (SIGUSR3);
           continue;

        }

        if (Info.EventType != KEY_EVENT) {

           continue;
        }

        // check if key is released
        if (!Info.Event.KeyEvent.bKeyDown) {
            continue;
        }

        // get the flags
        u32 Flags = Info.Event.KeyEvent.dwControlKeyState;

        // convert key code
        if (IsMetaKey (Flags)) {

            InKey = TranslateVKey (Info.Event.KeyEvent.wVirtualKeyCode, Flags);

        } else if (IsSpecialKey (Info.Event.KeyEvent.wVirtualKeyCode)) {

            InKey = TranslateVKey (Info.Event.KeyEvent.wVirtualKeyCode, Flags);

        } else if (Info.Event.KeyEvent.uChar.AsciiChar != 0) {

            InKey = Info.Event.KeyEvent.uChar.AsciiChar;

        }

    }

    return (InKey != 0);
}



static Key KbdGetKey ()
// return the next available key and set this key to 0 (NoKey)
{
    Key K = InKey;
    InKey = 0;

    return K;
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
/*                              Class Keyboard                               */
/*****************************************************************************/



Keyboard::Keyboard ():
    Console (1),
    TransTable (NULL)
{
    KbdHandle = GetStdHandle (STD_INPUT_HANDLE);
    CHECK (KbdHandle != INVALID_HANDLE_VALUE);

    CHECK (SetConsoleMode (KbdHandle, ENABLE_WINDOW_INPUT) != 0);
}



Keyboard::~Keyboard ()
{
    delete [] TransTable;
}



Key Keyboard::RawKey ()
// Get a raw (unmapped) key from the os
{
    // Wait for a key calling repeatedly App->Idle ()
    int I = 0;
    while (KbdCharAvail () == 0) {
        Sleep (20);
        if (I == 0) {
            CurThread () -> Idle ();
        }
        if (++I == 5) {
            I = 0;
        }
    }

    // Get the key
    return KbdGetKey ();
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
// Return a string describing the given key
{
    if (IsVirtualKey (K)) {
        // It is a virtual key, remap it to it's extended form
        K = KbdMapVirtual (K);
    }

    // Now return the key description
    return App->LoadMsg (MSGBASE_KBD + K);
}


