/*****************************************************************************/
/*                                                                           */
/*                                  CONSOLE.CC                               */
/*                                                                           */
/* (C) 1995-96  Ullrich von Bassewitz                                        */
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



// X Window backend for the spunk library. Because of the naming conventions
// of X Window there are name collisions between Xlib and spunk. To resolve
// them, we implement the functionality of class Screen here, but have an
// extra module for class Screen that calls functions from console.cc.
// Maybe namespaces are a better solution - but they are not yet supported
// by all compilers.



#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>

#include "../machine.h"
#include "../msgid.h"
#include "../object.h"
#include "../stack.h"
#include "../charset.h"
#include "../environ.h"
#include "../keymap.h"
#include "../progutil.h"
#include "../scrmodes.h"
#include "../kbd.h"
#include "../wincolor.h"

// Be shure to include all X11 stuff _after_ the spunk includes
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>



/*****************************************************************************/
/*                                 Constants                                 */
/*****************************************************************************/



// Window dimensions (in chars)
const int MinWidth              = 80;
const int MinHeight             = 25;
const int MaxWidth              = 100;
const int MaxHeight             = 50;

// Indices into the KeySymMap array
const unsigned kiPlane          = 0;
const unsigned kiShift          = 1;
const unsigned kiCtrl           = 2;
const unsigned kiMeta           = 3;
const unsigned kiCount          = 4;

// The Alt modifiers
const unsigned AltGrMask        = Mod3Mask;
const unsigned AltMask          = Mod4Mask;



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



// The one and only keyboard instance
Keyboard* Kbd;

// X variables
static Display*        SpunkDisplay;
static Window          SpunkWindow;
static GC              SpunkGC;
static Font            SpunkFont;

// Character dimensions (fixed font)
static unsigned CharHeight;
static unsigned CharWidth;
static unsigned CharDescent;
static unsigned long CharULPos;         // Underline position
static unsigned long CharULThickness;   // Underline thickness

// The screen dimensions
int ScreenHeight;
int ScreenWidth;

// Screen is using codepage 437
int ScreenCP437;

// Screen is using a color mode
int ScreenColor;

// Cursor
static enum { csOff, csOn, csFat } CursorType = csOff;
static unsigned CursorX = 0;
static unsigned CursorY = 0;

// Attribute and color management
static unsigned long Foreground = (unsigned long) -1;
static unsigned long Background = (unsigned long) -1;
static int LastAttr = -1;

enum _ColorMode { cmMono, cmBW, cmColor };
static _ColorMode ColorMode;            // Current colormode
static _ColorMode InitColorMode;        // Colormode at startup
static int Underline = 0;               // Attribute has underline set
static unsigned ColorDepth;             // Server color depth

static unsigned MonoBG;                 // Index of bg color in bw/mono mode
static unsigned MonoFG;                 // Index of fg color in bw/mono mode

// The application color map
static XColor Colors [coCount] = {
    { 0,   0*1024,   0*1024,   0*1024 },        // black
    { 0,   0*1024,   0*1024,  42*1024 },        // blue
    { 0,   0*1024,  42*1024,   0*1024 },        // green
    { 0,   0*1024,  42*1024,  42*1024 },        // cyan
    { 0,  42*1024,   0*1024,   0*1024 },        // red
    { 0,  42*1024,   0*1024,  42*1024 },        // magenta
    { 0,  42*1024,  21*1024,   0*1024 },        // brown
    { 0,  42*1024,  42*1024,  42*1024 },        // lightgray
    { 0,  21*1024,  21*1024,  21*1024 },        // darkgray
    { 0,  21*1024,  21*1024,  63*1024 },        // lightblue
    { 0,  21*1024,  63*1024,  21*1024 },        // lightgreen
    { 0,  21*1024,  63*1024,  63*1024 },        // lightcyan
    { 0,  63*1024,  21*1024,  21*1024 },        // lightred
    { 0,  63*1024,  21*1024,  63*1024 },        // lightmagenta
    { 0,  63*1024,  63*1024,  21*1024 },        // yellow
    { 0,  63*1024,  63*1024,  63*1024 }         // white
};

// The virtual screen we are writing to. Don't bother to resize the screen,
// use the maximum dimensions instead
static u16 ActualScreen [MaxHeight][MaxWidth];
static u16 VirtualScreen [MaxHeight][MaxWidth];

// Global keyboard buffer
static CircularBuffer<Key, 16>      KbdBuffer;

// Mapper table from virtual to extended keys. This table is fixed.
struct {
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

// Mapping table keysym --> Key
struct {
    KeySym Sym;
    Key    Keys [kiCount];
} KeySymMap [] = {
    // KeySym           Key             Shift-Key       Ctrl-Ctrl       Meta-Key
    { XK_Left,          { kbLeft,       kbNoKey,      kbCtrlLeft,   kbMetaLeft    } },
    { XK_Up,            { kbUp,         kbNoKey,      kbCtrlUp,     kbMetaUp      } },
    { XK_Right,         { kbRight,      kbNoKey,      kbCtrlRight,  kbMetaRight   } },
    { XK_Down,          { kbDown,       kbNoKey,      kbCtrlDown,   kbMetaDown    } },
    { XK_Page_Up,       { kbPgUp,       kbNoKey,      kbCtrlPgUp,   kbMetaPgUp    } },
    { XK_Page_Down,     { kbPgDn,       kbNoKey,      kbCtrlPgDn,   kbMetaPgDn    } },
    { XK_End,           { kbEnd,        kbNoKey,      kbCtrlEnd,    kbMetaEnd     } },
    { XK_Home,          { kbHome,       kbNoKey,      kbCtrlHome,   kbMetaHome    } },
    { XK_Begin,         { kbHome,       kbNoKey,      kbCtrlHome,   kbMetaHome    } },
    { XK_F1,            { kbF1,         kbShiftF1,    kbCtrlF1,     kbMetaF1      } },
    { XK_F2,            { kbF2,         kbShiftF2,    kbCtrlF2,     kbMetaF2      } },
    { XK_F3,            { kbF3,         kbShiftF3,    kbCtrlF3,     kbMetaF3      } },
    { XK_F4,            { kbF4,         kbShiftF4,    kbCtrlF4,     kbMetaF4      } },
    { XK_F5,            { kbF5,         kbShiftF5,    kbCtrlF5,     kbMetaF5      } },
    { XK_F6,            { kbF6,         kbShiftF6,    kbCtrlF6,     kbMetaF6      } },
    { XK_F7,            { kbF7,         kbShiftF7,    kbCtrlF7,     kbMetaF7      } },
    { XK_F8,            { kbF8,         kbShiftF8,    kbCtrlF8,     kbMetaF8      } },
    { XK_F9,            { kbF9,         kbShiftF9,    kbCtrlF9,     kbMetaF9      } },
    { XK_F10,           { kbF10,        kbShiftF10,   kbCtrlF10,    kbMetaF10     } },
    { XK_F11,           { kbF11,        kbShiftF11,   kbCtrlF11,    kbMetaF11     } },
    { XK_F12,           { kbF12,        kbShiftF12,   kbCtrlF12,    kbMetaF12     } },
    { XK_BackSpace,     { kbBack,       kbNoKey,      kbNoKey,      kbNoKey       } },
    { XK_Tab,           { kbTab,        kbShiftTab,   kbCtrlTab,    kbMetaTab     } },
    { XK_KP_Enter,      { kbEnter,      kbNoKey,      kbNoKey,      kbNoKey       } },
    { XK_Return,        { kbEnter,      kbNoKey,      kbNoKey,      kbNoKey       } },
    { XK_Escape,        { kbEsc,        kbNoKey,      kbNoKey,      kbMetaEsc     } },
    { XK_Delete,        { kbDel,        kbShiftDel,   kbCtrlDel,    kbMetaDel     } },
    { XK_Insert,        { kbIns,        kbShiftIns,   kbCtrlIns,    kbMetaIns     } },
    { XK_KP_Enter,      { kbEnter,      kbNoKey,      kbNoKey,      kbNoKey       } },
    { XK_KP_Left,       { kbLeft,       kbNoKey,      kbCtrlLeft,   kbMetaLeft    } },
    { XK_KP_Up,         { kbUp,         kbNoKey,      kbCtrlUp,     kbMetaUp      } },
    { XK_KP_Right,      { kbRight,      kbNoKey,      kbCtrlRight,  kbMetaRight   } },
    { XK_KP_Down,       { kbDown,       kbNoKey,      kbCtrlDown,   kbMetaDown    } },
    { XK_KP_Page_Up,    { kbPgUp,       kbNoKey,      kbCtrlPgUp,   kbMetaPgUp    } },
    { XK_KP_Page_Down,  { kbPgDn,       kbNoKey,      kbCtrlPgDn,   kbMetaPgDn    } },
    { XK_KP_End,        { kbEnd,        kbNoKey,      kbCtrlEnd,    kbMetaEnd     } },
    { XK_KP_Home,       { kbHome,       kbNoKey,      kbCtrlHome,   kbMetaHome    } },
    { XK_KP_Begin,      { kbHome,       kbNoKey,      kbCtrlHome,   kbMetaHome    } },
    { XK_KP_Delete,     { kbDel,        kbShiftDel,   kbCtrlDel,    kbMetaDel     } },
    { XK_KP_Insert,     { kbIns,        kbShiftIns,   kbCtrlIns,    kbMetaIns     } },
    { XK_KP_F1,         { kbF1,         kbShiftF1,    kbCtrlF1,     kbMetaF1      } },
    { XK_KP_F2,         { kbF2,         kbShiftF2,    kbCtrlF2,     kbMetaF2      } },
    { XK_KP_F3,         { kbF3,         kbShiftF3,    kbCtrlF3,     kbMetaF3      } },
    { XK_KP_F4,         { kbF4,         kbShiftF4,    kbCtrlF4,     kbMetaF4      } },
    { XK_space,         { ' ',          ' ',          kbNoKey,      kbMetaSpace   } },
    { XK_exclam,        { '!',          '!',          kbNoKey,      kbNoKey       } },
    { XK_quotedbl,      { '\"',         '\"',         kbNoKey,      kbNoKey       } },
    { XK_numbersign,    { '#',          '#',          kbNoKey,      kbNoKey       } },
    { XK_dollar,        { '$',          '$',          kbNoKey,      kbNoKey       } },
    { XK_percent,       { '%',          '%',          kbNoKey,      kbNoKey       } },
    { XK_ampersand,     { '&',          '&',          kbNoKey,      kbNoKey       } },
    { XK_apostrophe,    { '\'',         '\'',         kbNoKey,      kbNoKey       } },
    { XK_parenleft,     { '(',          '(',          kbNoKey,      kbNoKey       } },
    { XK_parenright,    { ')',          ')',          kbNoKey,      kbNoKey       } },
    { XK_asterisk,      { '*',          '*',          kbNoKey,      kbNoKey       } },
    { XK_plus,          { '+',          '+',          kbNoKey,      kbNoKey       } },
    { XK_comma,         { ',',          ',',          kbNoKey,      kbNoKey       } },
    { XK_minus,         { '-',          '-',          kbNoKey,      kbNoKey       } },
    { XK_period,        { '.',          '.',          kbNoKey,      kbNoKey       } },
    { XK_slash,         { '/',          '/',          kbNoKey,      kbNoKey       } },
    { XK_0,             { '0',          kbNoKey,      kbNoKey,      kbMeta0       } },
    { XK_1,             { '1',          kbNoKey,      kbNoKey,      kbMeta1       } },
    { XK_2,             { '2',          kbNoKey,      kbNoKey,      kbMeta2       } },
    { XK_3,             { '3',          kbNoKey,      kbNoKey,      kbMeta3       } },
    { XK_4,             { '4',          kbNoKey,      kbNoKey,      kbMeta4       } },
    { XK_5,             { '5',          kbNoKey,      kbNoKey,      kbMeta5       } },
    { XK_6,             { '6',          kbNoKey,      kbNoKey,      kbMeta6       } },
    { XK_7,             { '7',          kbNoKey,      kbNoKey,      kbMeta7       } },
    { XK_8,             { '8',          kbNoKey,      kbNoKey,      kbMeta8       } },
    { XK_9,             { '9',          kbNoKey,      kbNoKey,      kbMeta9       } },
    { XK_colon,         { ':',          ':',          kbNoKey,      kbNoKey       } },
    { XK_semicolon,     { ';',          ';',          kbNoKey,      kbNoKey       } },
    { XK_less,          { '<',          '<',          kbNoKey,      kbNoKey       } },
    { XK_equal,         { '=',          '=',          kbNoKey,      kbNoKey       } },
    { XK_greater,       { '>',          '>',          kbNoKey,      kbNoKey       } },
    { XK_question,      { '?',          '?',          kbNoKey,      kbNoKey       } },
    { XK_at,            { '@',          '@',          kbNoKey,      kbNoKey       } },
    { XK_A,             { 'A',          'A',          kbCtrlA,      kbMetaA       } },
    { XK_B,             { 'B',          'B',          kbCtrlB,      kbMetaB       } },
    { XK_C,             { 'C',          'C',          kbCtrlC,      kbMetaC       } },
    { XK_D,             { 'D',          'D',          kbCtrlD,      kbMetaD       } },
    { XK_E,             { 'E',          'E',          kbCtrlE,      kbMetaE       } },
    { XK_F,             { 'F',          'F',          kbCtrlF,      kbMetaF       } },
    { XK_G,             { 'G',          'G',          kbCtrlG,      kbMetaG       } },
    { XK_H,             { 'H',          'H',          kbCtrlH,      kbMetaH       } },
    { XK_I,             { 'I',          'I',          kbCtrlI,      kbMetaI       } },
    { XK_J,             { 'J',          'J',          kbCtrlJ,      kbMetaJ       } },
    { XK_K,             { 'K',          'K',          kbCtrlK,      kbMetaK       } },
    { XK_L,             { 'L',          'L',          kbCtrlL,      kbMetaL       } },
    { XK_M,             { 'M',          'M',          kbCtrlM,      kbMetaM       } },
    { XK_N,             { 'N',          'N',          kbCtrlN,      kbMetaN       } },
    { XK_O,             { 'O',          'O',          kbCtrlO,      kbMetaO       } },
    { XK_P,             { 'P',          'P',          kbCtrlP,      kbMetaP       } },
    { XK_Q,             { 'Q',          'Q',          kbCtrlQ,      kbMetaQ       } },
    { XK_R,             { 'R',          'R',          kbCtrlR,      kbMetaR       } },
    { XK_S,             { 'S',          'S',          kbCtrlS,      kbMetaS       } },
    { XK_T,             { 'T',          'T',          kbCtrlT,      kbMetaT       } },
    { XK_U,             { 'U',          'U',          kbCtrlU,      kbMetaU       } },
    { XK_V,             { 'V',          'V',          kbCtrlV,      kbMetaV       } },
    { XK_W,             { 'W',          'W',          kbCtrlW,      kbMetaW       } },
    { XK_X,             { 'X',          'X',          kbCtrlX,      kbMetaX       } },
    { XK_Y,             { 'Y',          'Y',          kbCtrlY,      kbMetaY       } },
    { XK_Z,             { 'Z',          'Z',          kbCtrlZ,      kbMetaZ       } },
    { XK_bracketleft,   { '[',          kbNoKey,      kbNoKey,      kbNoKey       } },
    { XK_backslash,     { '\\',         kbNoKey,      kbNoKey,      kbNoKey       } },
    { XK_bracketright,  { ']',          kbNoKey,      kbNoKey,      kbNoKey       } },
    { XK_asciicircum,   { '^',          kbNoKey,      kbNoKey,      kbNoKey       } },
    { XK_underscore,    { '_',          kbNoKey,      kbNoKey,      kbNoKey       } },
    { XK_grave,         { '`',          kbNoKey,      kbNoKey,      kbNoKey       } },
    { XK_a,             { 'a',          'A',          kbCtrlA,      kbMetaA       } },
    { XK_b,             { 'b',          'B',          kbCtrlB,      kbMetaB       } },
    { XK_c,             { 'c',          'C',          kbCtrlC,      kbMetaC       } },
    { XK_d,             { 'd',          'D',          kbCtrlD,      kbMetaD       } },
    { XK_e,             { 'e',          'E',          kbCtrlE,      kbMetaE       } },
    { XK_f,             { 'f',          'F',          kbCtrlF,      kbMetaF       } },
    { XK_g,             { 'g',          'G',          kbCtrlG,      kbMetaG       } },
    { XK_h,             { 'h',          'H',          kbCtrlH,      kbMetaH       } },
    { XK_i,             { 'i',          'I',          kbCtrlI,      kbMetaI       } },
    { XK_j,             { 'j',          'J',          kbCtrlJ,      kbMetaJ       } },
    { XK_k,             { 'k',          'K',          kbCtrlK,      kbMetaK       } },
    { XK_l,             { 'l',          'L',          kbCtrlL,      kbMetaL       } },
    { XK_m,             { 'm',          'M',          kbCtrlM,      kbMetaM       } },
    { XK_n,             { 'n',          'N',          kbCtrlN,      kbMetaN       } },
    { XK_o,             { 'o',          'O',          kbCtrlO,      kbMetaO       } },
    { XK_p,             { 'p',          'P',          kbCtrlP,      kbMetaP       } },
    { XK_q,             { 'q',          'Q',          kbCtrlQ,      kbMetaQ       } },
    { XK_r,             { 'r',          'R',          kbCtrlR,      kbMetaR       } },
    { XK_s,             { 's',          'S',          kbCtrlS,      kbMetaS       } },
    { XK_t,             { 't',          'T',          kbCtrlT,      kbMetaT       } },
    { XK_u,             { 'u',          'U',          kbCtrlU,      kbMetaU       } },
    { XK_v,             { 'v',          'V',          kbCtrlV,      kbMetaV       } },
    { XK_w,             { 'w',          'W',          kbCtrlW,      kbMetaW       } },
    { XK_x,             { 'x',          'X',          kbCtrlX,      kbMetaX       } },
    { XK_y,             { 'y',          'Y',          kbCtrlY,      kbMetaY       } },
    { XK_z,             { 'z',          'Z',          kbCtrlZ,      kbMetaZ       } },
    { XK_braceleft,     { '{',          kbNoKey,      kbNoKey,      kbNoKey       } },
    { XK_bar,           { '|',          kbNoKey,      kbNoKey,      kbNoKey       } },
    { XK_braceright,    { '}',          kbNoKey,      kbNoKey,      kbNoKey       } },
    { XK_asciitilde,    { '~',          kbNoKey,      kbNoKey,      kbNoKey       } },
    { XK_ssharp,        { 0xE1,         0xE1,         kbNoKey,      kbNoKey       } },
    { XK_adiaeresis,    { 0x84,         0x8E,         kbNoKey,      kbNoKey       } },
    { XK_odiaeresis,    { 0x94,         0x99,         kbNoKey,      kbNoKey       } },
    { XK_udiaeresis,    { 0x81,         0x9A,         kbNoKey,      kbNoKey       } }
};



/*****************************************************************************/
/*                          Argument handling stuff                          */
/*****************************************************************************/



static String GetStringArg (const char* ArgName)
// Try to find a program parameter with the given name. If we find one, return
// the parameter following this name and remove both from the argument list.
{
    // Get the argument vector
    char** Args = GetArgVec ();

    // search for the given argument
    int I = 1;
    while (Args [I] != 0) {
        if (strcmp (Args [I], ArgName) == 0) {
            // found
            if (Args [I+1] != 0) {
                // Remember the argument value
                String ArgVal = Args [I+1];
                // Remove the argument name and the argument value
                RemoveArg (I+1);
                RemoveArg (I);
                // Return the value read
                return ArgVal;
            }
        }
        I++;
    }

    // Not found
    return "";
}



/*****************************************************************************/
/*                External visible stuff for class Screen                    */
/*****************************************************************************/



static inline unsigned ScrXPos (unsigned X)
// Calculate a graphics X position from a character X position
{
    return X * CharWidth;
}



static inline unsigned ScrYPos (unsigned Y)
// Calculate a graphics Y position from a character Y position
{
    return (Y + 1) * CharHeight - CharDescent;
}



static void ScrSetBackground (unsigned Index)
// Set the background color to the color with the given index
{
    unsigned long Pixel = Colors [Index].pixel;
    if (Pixel != Background) {
        Background = Pixel;
        XSetBackground (SpunkDisplay, SpunkGC, Pixel);
    }
}



static void ScrSetForeground (unsigned Index)
// Set the foreground color to the color with the given index
{
    unsigned long Pixel = Colors [Index].pixel;
    if (Pixel != Foreground) {
        Foreground = Pixel;
        XSetForeground (SpunkDisplay, SpunkGC, Pixel);
    }
}



static void ScrSetupAttr (unsigned char Attr)
// Set up the colors for the new attribute and remember the new attribute
{
    // Remove the blink attribute - it's not used
    Attr &= 0x7F;

    // Check if something has changed
    if (Attr != LastAttr) {

        // Assume no underlining
        Underline = 0;

        // Set the colors according to the color model used
        switch (ColorMode) {

            case cmMono:
                // Set the foreground color
                switch (Attr & 0x0F) {

                    case 0x00:
                        // Black
                        ScrSetForeground (MonoBG);
                        break;

                    case 0x01:
                        // Black/underline
                        ScrSetForeground (MonoBG);
                        Underline = 1;
                        break;

                    case 0x07:
                        // Black
                        ScrSetForeground (MonoFG);
                        break;

                    case 0x09:
                        // White/underline
                        ScrSetForeground (MonoFG);
                        Underline = 1;
                        break;

                    case 0x0F:
                        // White
                        ScrSetForeground (MonoFG);
                        break;

                    default:
                        // Handle as normal text
                        ScrSetForeground (MonoFG);
                        break;

                }

                // Set the background color
                switch (Attr & 0x70) {

                    case 0x00:
                        // Black
                        ScrSetBackground (MonoBG);
                        break;

                    case 0x70:
                        ScrSetBackground (MonoFG);
                        break;

                    default:
                        // Handle as normal text
                        ScrSetBackground (MonoBG);
                        break;
                }
                break;

            case cmBW:
                // Set the foreground color
                switch (Attr & 0x0F) {

                    case 0x00:
                        // Black
                        ScrSetForeground (MonoBG);
                        break;

                    case 0x01:
                        // Black/underline
                        ScrSetForeground (MonoBG);
                        Underline = 1;
                        break;

                    case 0x07:
                        // Lightgray
                        ScrSetForeground (coLightGray);
                        break;

                    case 0x09:
                        // White/underline
                        ScrSetForeground (MonoFG);
                        Underline = 1;
                        break;

                    case 0x0F:
                        // White
                        ScrSetForeground (MonoFG);
                        break;

                    default:
                        // Handle as normal text
                        ScrSetForeground (coLightGray);
                        break;

                }

                // Set the background color
                switch (Attr & 0x70) {

                    case 0x00:
                        // Black
                        ScrSetBackground (MonoBG);
                        break;

                    case 0x70:
                        ScrSetBackground (coLightGray);
                        break;

                    default:
                        // Handle as normal text
                        ScrSetBackground (MonoBG);
                        break;
                }
                break;

            case cmColor:
                ScrSetBackground ((Attr >> 4) & 0x0F);
                ScrSetForeground (Attr & 0x0F);
                break;

            default:
                FAIL ("ScrSetupAttr: Unknown color mode");
                break;
        }

        // Remember the new attribute
        LastAttr = Attr;
    }
}



static void ScrDrawText (unsigned X, unsigned Y, const char* Buf, unsigned Count)
// Draw text using XDrawImageString. Underline the text if needed.
{
    // Draw the string
    XDrawImageString (SpunkDisplay, SpunkWindow, SpunkGC,
                      ScrXPos (X), ScrYPos (Y),
                      Buf, Count);

    // If the underline attribute is set, underline the text
    if (Underline) {
        XFillRectangle (SpunkDisplay, SpunkWindow, SpunkGC,
                        ScrXPos (X), ScrYPos (Y) + CharULPos,
                        CharWidth * Count, CharULThickness);
    }
}



static void ScrDrawCell (unsigned X, unsigned Y, u16 Cell)
// Draw a single cell unconditionally
{
    // Set the colors for the new attribute
    ScrSetupAttr (Cell >> 8);

    // Draw the cell
    char Buf [1];
    Buf [0] = Cell & 0xFF;
    ScrDrawText (X, Y, Buf, 1);
}



static void ScrDrawCursor ()
{
    static const char CursorOnString [] = "_";

    u16 Cell;

    // Draw the cursor according to the current cursor state
    switch (CursorType) {

        case csOn:
            // Use an underline character as cursor
            Cell = ActualScreen [CursorY][CursorX];
            ScrSetupAttr (Cell >> 8);
            XDrawString (SpunkDisplay, SpunkWindow, SpunkGC,
                         ScrXPos (CursorX), ScrYPos (CursorY),
                         CursorOnString, 1);
            break;

        case csFat:
            // Redraw the character with the attribute inverted
            Cell = ActualScreen [CursorY][CursorX];
            Cell = (Cell & 0x00FF) | ((Cell & 0x0F00) << 4) | ((Cell & 0xF000) >> 4);
            ScrDrawCell (CursorX, CursorY, Cell);
            break;

        case csOff:
            break;

        default:
            FAIL ("ScrDrawCursor: Invalid cursor type");
            break;
    }
}



void ScrCursorOn ()
// Set the cursor state to on
{
    // If we had a fat cursor before, we need to redraw the cell to remove the
    // old cursor
    if (CursorType == csFat) {
        ScrDrawCell (CursorX, CursorY, ActualScreen [CursorY][CursorX]);
    }

    // Remember the new cursor
    CursorType = csOn;

    // Now draw the actual cursor
    ScrDrawCursor ();
}



void ScrCursorOff ()
// Set the cursor state to off
{
    // Just redraw the cell
    ScrDrawCell (CursorX, CursorY, ActualScreen [CursorY][CursorX]);

    // Remember the new cursor
    CursorType = csOff;
}



void ScrCursorFat ()
// Set the cursor state to fat
{
    // Remember the new state
    CursorType = csFat;

    // Draw the new cursor
    ScrDrawCursor ();
}



void ScrSetCursorPos (unsigned X, unsigned Y)
// Set the cursor position
{
    // Check if the cursor position has changed
    if (X != CursorX || Y != CursorY) {
        // Cursor position has changed. If we had a visible cursor before,
        // remove it at the old cursor position
        if (CursorType != csOff) {
            ScrDrawCell (CursorX, CursorY, ActualScreen [CursorY][CursorX]);
        }

        // Remember the new position
        CursorX = X;
        CursorY = Y;

        // Draw the new cursor
        ScrDrawCursor ();
    }
}



void ScrSetMode (unsigned Mode)
// Set a screen mode
{
    unsigned XSize, YSize;
    _ColorMode NewColorMode = ColorMode;

    // Check the new dimensions
    switch (Mode) {

        case vmAsk:
            // Request the current window size. Assume, color depth
            // and other window parameters have not changed.
            XWindowAttributes WA;
            XGetWindowAttributes (SpunkDisplay, SpunkWindow, &WA);
            XSize = WA.width / CharWidth;
            YSize = WA.height / CharHeight;
            break;

        case vmBW40:
            XSize = 40;
            YSize = 25;
            NewColorMode = (ColorDepth == 1)? cmMono : cmBW;
            break;

        case vmMono:
        case vmBW80:
            XSize = 80;
            YSize = 25;
            NewColorMode = (ColorDepth == 1)? cmMono : cmBW;
            break;

        case vmCO40:
            XSize = 40;
            YSize = 25;
            NewColorMode = cmColor;
            break;

        case vmCO80:
            XSize = 80;
            YSize = 25;
            NewColorMode = cmColor;
            break;

        case vmVGA_80x30:
            XSize = 80;
            YSize = 30;
            NewColorMode = cmColor;
            break;

        case vmVGA_80x34:
            XSize = 80;
            YSize = 34;
            NewColorMode = cmColor;
            break;

        case vmVGA_80x43:
            XSize = 80;
            YSize = 43;
            NewColorMode = cmColor;
            break;

        case vmVGA_80x50:
            XSize = 80;
            YSize = 50;
            NewColorMode = cmColor;
            break;

        case vmVGA_80x60:
            XSize = 80;
            YSize = 60;
            NewColorMode = cmColor;
            break;

        case vmVGA_94x25:
            XSize = 94;
            YSize = 25;
            NewColorMode = cmColor;
            break;

        case vmVGA_94x30:
            XSize = 94;
            YSize = 30;
            NewColorMode = cmColor;
            break;

        case vmVGA_94x34:
            XSize = 94;
            YSize = 34;
            NewColorMode = cmColor;
            break;

        case vmVGA_94x43:
            XSize = 94;
            YSize = 43;
            NewColorMode = cmColor;
            break;

        case vmVGA_94x50:
            XSize = 94;
            YSize = 50;
            NewColorMode = cmColor;
            break;

        case vmVGA_94x60:
            XSize = 94;
            YSize = 60;
            NewColorMode = cmColor;
            break;

        case vmET4_100x40:
            XSize = 100;
            YSize = 40;
            NewColorMode = cmColor;
            break;

        default:
            // Ignore modes we do not know
            return;

    }

    // Check if we support the color setting. If we cannot accept the color
    // setting, use the maximum color setting instead (honor the size)
    if (NewColorMode > InitColorMode) {
        NewColorMode = InitColorMode;
    }

    // Check if we can resize the screen, bail out if not
    if (XSize < MinWidth || XSize > MaxWidth ||
        YSize < MinHeight || YSize > MaxHeight) {
        return;
    }

    // Set the global variables
    ScreenWidth  = XSize;
    ScreenHeight = YSize;
    ColorMode    = NewColorMode;
    ScreenColor  = ColorMode == cmColor;

    // Switch the cursor off
    CursorType = csOff;

    // Resize the window
    XResizeWindow (SpunkDisplay, SpunkWindow,
                   ScreenWidth * CharWidth,
                   ScreenHeight * CharHeight);

    // Clear the shadow screen to force an update on the next writes
    memset (ActualScreen, 0, sizeof (ActualScreen));

    // Reset the attribute and color settings
    LastAttr = -1;
    Foreground = (unsigned long) -1;
    Background = (unsigned long) -1;
}



void ScrInit (unsigned XSize, unsigned YSize)
// Initialize the screen
{
    // Set the screen size
    ScreenHeight = YSize;
    ScreenWidth  = XSize;

    // Initialize some other stuff
    CursorType = csOff;
    memset (VirtualScreen, ' ', sizeof (VirtualScreen));
    memset (ActualScreen, 0, sizeof (ActualScreen));

    // Open the X display. If a display is given as argument use it, otherwise
    // use the DISPLAY environment variable (the latter should be automatic,
    // But it seems to fail on some operating systems).
    String DisplayName = GetStringArg ("-display");
    if (DisplayName.IsEmpty ()) {
        DisplayName = GetEnvVar ("DISPLAY");
    }
    SpunkDisplay = XOpenDisplay (DisplayName.GetStr ());
    if (SpunkDisplay == NULL) {
        FAIL ("ScrInit: Cannot open display");
    }

    // Get a screen
    int SpunkScreen = DefaultScreen (SpunkDisplay);

    // Decide which color model to use. If we have more than 16 colors, use
    // the color model. If we have more than two colors, but less or equal
    // to 16, use the black/white model. If we have two colors, use the
    // monochrome model.
    // All this may be overridden by the SPUNK_COLOR environment variable.
    ColorDepth = XDefaultDepth (SpunkDisplay, SpunkScreen);
    if (ColorDepth == 1) {
        // One plane - pure monochrome
        ColorMode = cmMono;
    } else if (ColorDepth <= 4) {
        // More than one but less than five - black/white
        ColorMode = cmBW;
    } else {
        // 16 color mode
        ColorMode = cmColor;
    }

    // Check for an explicit override
    if (!GetEnvVar ("SPUNK_COLOR").IsEmpty ()) {
        // There is an explicit override
        if (GetEnvBool ("SPUNK_COLOR", ColorMode == cmColor)) {
            ColorMode = cmColor;
        } else {
            ColorMode = (ColorDepth > 1)? cmBW : cmMono;
        }
    }

    // Get all needed colors
    Colormap CM;
    switch (ColorMode) {

        case cmMono:
            Colors [coBlack].pixel = XBlackPixel (SpunkDisplay, SpunkScreen);
            Colors [coWhite].pixel = XWhitePixel (SpunkDisplay, SpunkScreen);
            break;

        case cmBW:
            CM = DefaultColormap (SpunkDisplay, SpunkScreen);
            if (XAllocColor (SpunkDisplay, CM, &Colors [coBlack]) == 0) {
                FAIL ("ScrInit: Cannot allocate color");
            }
            if (XAllocColor (SpunkDisplay, CM, &Colors [coLightGray]) == 0) {
                FAIL ("ScrInit: Cannot allocate color");
            }
            if (XAllocColor (SpunkDisplay, CM, &Colors [coWhite]) == 0) {
                FAIL ("ScrInit: Cannot allocate color");
            }
            break;

        case cmColor:
            CM = DefaultColormap (SpunkDisplay, SpunkScreen);
            for (unsigned C = 0; C < coCount; C++) {
                if (XAllocColor (SpunkDisplay, CM, &Colors [C]) == 0) {
                    FAIL ("ScrInit: Cannot allocate color");
                }
            }
            break;

        default:
            FAIL ("ScrInit: Invalid color mode");
            break;
    }

    // Check for inverse video (mono and bw only)
    if (GetEnvBool ("SPUNK_XINVERTMONO", 0)) {
        MonoBG = coWhite;
        MonoFG = coBlack;
    } else {
        MonoBG = coBlack;
        MonoFG = coWhite;
    }

    // Remember the startup color mode for later mode switches
    InitColorMode = ColorMode;

    // Set the palette color flag
    ScreenColor = (ColorMode == cmColor);

    // Load the font. If the font is specified by the user, use the given
    // font. If the font is not spcified, try "vga", then some other defaults.
    XFontStruct* FontInfo = 0;
    String Font = GetEnvVar ("SPUNK_XFONT");
    if (Font.IsEmpty ()) {

        // A list of fonts to try
        static const char* Fonts [] = { "vga", "fixed", "8x13" };

        // Try to load the fonts in the given order
        for (unsigned I = 0; I < sizeof (Fonts) / sizeof (Fonts [0]); I++) {

            // Try to load the font
            FontInfo = XLoadQueryFont (SpunkDisplay, Fonts [I]);

            // End the search if we got the font
            if (FontInfo) {
                // We got the font
                Font = Fonts [I];
                break;
            }
        }

        // If we could not load the font, bail out
        if (FontInfo == 0) {
            FAIL ("ScrInit: Could not load X font");
        }

    } else {

        // A user font has been given. Try to load that font
        FontInfo = XLoadQueryFont (SpunkDisplay, Font.GetStr ());

    }

    // If we could not load the font, bail out
    if (FontInfo == 0) {
        FAIL ("ScrInit: Could not load X font");
    }

    // Remember the font identifier
    SpunkFont = FontInfo->fid;

    // Get the width and height of the font
    int Direction, Ascent, Descent;
    XCharStruct Overall;
    XTextExtents (FontInfo, "", 0, &Direction, &Ascent, &Descent, &Overall);
    CharWidth   = XTextWidth (FontInfo, "M", 1);
    CharHeight  = Ascent + Descent;
    CharDescent = Descent;

    // Get the font parameters needed for the underlining of text
    if (XGetFontProperty (FontInfo, XA_UNDERLINE_POSITION, &CharULPos) == 0) {
        // Not defined, use default
        CharULPos = 2;
    }
    if (XGetFontProperty (FontInfo, XA_UNDERLINE_THICKNESS, &CharULThickness) == 0) {
        // Not defined, use default
        CharULThickness = 1;
    }

    // Determine the character set and the corresponding translation table
    // The default is to assume codepage 437 with any font that's name starts
    // with "vga", if not overridden by environment.
    Font.ToUpper ();
    ScreenCP437 = GetEnvBool ("SPUNK_CP437", Font.Match ("VGA*"));

    // Set up the window size
    XSizeHints SizeHints;
    XWMHints WMHints;
    SizeHints.x          = 100;
    SizeHints.y          = 100;
    SizeHints.flags      = 0;

    // Search for a -geometry command argument
    String Geometry = GetStringArg ("-geometry");
    if (!Geometry.IsEmpty ()) {

        // Set up the default string
        char DefaultGeometry [100];
        sprintf (DefaultGeometry, "%ux%u+%d+%d",
                 ScreenWidth, ScreenHeight, SizeHints.x, SizeHints.y);

        // We got something, parse it.
        int Flag = XGeometry (SpunkDisplay,
                              SpunkScreen,
                              Geometry.GetStr (),
                              DefaultGeometry,
                              5,
                              CharWidth,
                              CharHeight,
                              0, 0,
                              &SizeHints.x, &SizeHints.y,
                              &ScreenWidth, &ScreenHeight);

        if (Flag & (XValue | YValue)) {
            SizeHints.flags |= USPosition;
        } else {
            SizeHints.flags |= PPosition;
        }

        if (Flag & (WidthValue | HeightValue)) {
            // Be shure, we have valid values
            if (ScreenWidth < MinWidth) {
                ScreenWidth = MinWidth;
            } else if (ScreenWidth > MaxWidth) {
                ScreenWidth = MaxWidth;
            }
            if (ScreenHeight < MinHeight) {
                ScreenHeight = MinHeight;
            } else if (ScreenHeight > MaxHeight) {
                ScreenHeight = MaxHeight;
            }
            SizeHints.flags |= USSize;
        } else {
            SizeHints.flags |= PSize;
        }

    } else {

        // No geometry argument given
        SizeHints.flags |= PPosition | PSize;

    }

    // Set up the remaining fields of the SizeHints structure
    SizeHints.width      = ScreenWidth * CharWidth;
    SizeHints.height     = ScreenHeight * CharHeight;
    SizeHints.min_width  = MinWidth * CharWidth;
    SizeHints.min_height = MinHeight * CharHeight;
    SizeHints.max_width  = MaxWidth * CharWidth;
    SizeHints.max_height = MaxHeight * CharHeight;
    SizeHints.width_inc  = CharWidth;
    SizeHints.height_inc = CharHeight;
    SizeHints.flags     |= PMinSize | PMaxSize | PResizeInc;
    WMHints.flags        = InputHint;
    WMHints.input        = True;

    // Create the window
    SpunkWindow = XCreateSimpleWindow (SpunkDisplay,
                                       DefaultRootWindow (SpunkDisplay),
                                       SizeHints.x,
                                       SizeHints.y,
                                       SizeHints.width,
                                       SizeHints.height,
                                       5,
                                       Colors [coWhite].pixel,
                                       Colors [coBlack].pixel);

    XSetStandardProperties (SpunkDisplay,               // Display
                            SpunkWindow,                // Window
                            GetProgName ().GetStr (),   // Window name
                            GetProgName ().GetStr (),   // Icon name
                            None,                       // Icon Pixmap
                            GetArgVec (),               // argv
                            GetArgCount (),             // argc
                            &SizeHints);                // Hints
    XSetWMHints (SpunkDisplay, SpunkWindow, &WMHints);

    // GC creation and initialization
    SpunkGC = XCreateGC (SpunkDisplay, SpunkWindow, 0, 0);

    // Load the font to use
    XSetFont (SpunkDisplay, SpunkGC, SpunkFont);

    // Set the cursor to show over the spunk window
    Cursor C = XCreateFontCursor (SpunkDisplay, XC_pirate);
    XDefineCursor (SpunkDisplay, SpunkWindow, C);

    // Select input events
    XSelectInput (SpunkDisplay, SpunkWindow,
                  KeyPressMask | ExposureMask | StructureNotifyMask);

    // Show the window
    XMapRaised (SpunkDisplay, SpunkWindow);
}



void ScrExit ()
// Destroy the window
{
    XUndefineCursor (SpunkDisplay, SpunkWindow);
    XFreeGC (SpunkDisplay, SpunkGC);
    XDestroyWindow (SpunkDisplay, SpunkWindow);
    XCloseDisplay (SpunkDisplay);
}



static void ScrUpdateRow (unsigned X0, unsigned Y0, unsigned Width,
                          int MustUpdate = 0)
// Update a screen row
{
    // Buffer management
    char Buf [MaxWidth];
    unsigned Count = 0;
    unsigned StartX = 0;    // initialize to avoid gcc warning...

    // Loop over a row
    for (unsigned X = X0; X < X0 + Width; X++) {

        // Get the screen cell
        u16 Cell = VirtualScreen [Y0][X];

        // Check if the screen cell needs an update.
        if (MustUpdate == 0 && Cell == ActualScreen [Y0][X]) {
            // We do not need an update and the cell is already ok.
            // If we have characters in the output buffer, draw them,
            // then start over with the next char.
            if (Count) {

                // Draw the text
                ScrDrawText (StartX, Y0, Buf, Count);

                // Buffer is empty now
                Count = 0;
            }

            // Next char
            continue;
        }

        // If the new attribute is different from the last one, there's
        // something to do
        unsigned char Attr = (Cell >> 8) & 0x7F;
        if (Attr != LastAttr) {

            // New attribute. Check if there are characters in the buffer
            // that have the old attribute. If so, write them out
            if (Count) {

                // Draw the text
                ScrDrawText (StartX, Y0, Buf, Count);

                // Buffer is empty now
                Count = 0;
            }

            // Put the new character in the buffer and remember the starting
            // position
            Buf [Count++] = char (Cell & 0x00FF);
            StartX = X;

            // Setup the colors for the new attribute
            ScrSetupAttr (Attr);

        } else {

            // No attribute change. If this is the first char in the buffer,
            // remember the position
            if (Count == 0) {
                StartX = X;
            }

            // Add the character to the buffer
            Buf [Count++] = char (Cell & 0x00FF);

        }

        // Ok, we did the update. Remember the actual screen contents
        ActualScreen [Y0][X] = Cell;

    }

    // If there are characters in the buffer, write them out
    if (Count) {
        ScrDrawText (StartX, Y0, Buf, Count);
    }

    // If we overwrote the cursor, redraw it
    if (CursorY == Y0 && CursorX >= X0 && CursorX < X0 + Width) {
        ScrDrawCursor ();
    }
}



void ScrWriteBuf (unsigned X, unsigned Y, const u16* Buf, unsigned Count)
// Write a buffer line to the screen, clipping right if needed
{
    if (Y >= (unsigned) ScreenHeight || X >= (unsigned) ScreenWidth) {
        // Completely outside
        return;
    }

    // Clip to the right
    if (X + Count > (unsigned) ScreenWidth) {
        Count = ScreenWidth - X;
    }

    // Copy the buffer in place, clearing the high bit of the attribute
    // (the blink bit). Blinking is not supported anyway, so we use the
    // high bit of the attribute to mark the cursor position.
    unsigned RunX = X;
    unsigned C    = Count;
    while (C--) {
        VirtualScreen [Y][RunX++] = *Buf++ & 0x7FFF;
    }

    // Update the buffer row on the screen
    ScrUpdateRow (X, Y, Count);
}



static void ScrUpdateArea (unsigned X0, unsigned Y0, unsigned Width, unsigned Height,
                    int MustUpdate = 0)
// Update a window area from the virtual screen. R must be already clipped!
{
    // Loop over all rows
    for (unsigned Y = Y0; Y < Y0 + Height; Y++) {

        // Update each row in turn
        ScrUpdateRow (X0, Y, Width, MustUpdate);

    }
}



void ScrFlush ()
// Flush the output command queue
{
    XFlush (SpunkDisplay);
}



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



static void KbdPress (XEvent& Event)
// Handle a keypress event
{
    // Remember the state
    unsigned State = Event.xkey.state;

    // Make a keysym
    KeySym Sym;
    XLookupString (&Event.xkey, NULL, 0, &Sym, NULL);

    // Determine the modifier state
    unsigned Table = kiPlane;
    if (State & ShiftMask) {
        Table = kiShift;
    } else if (State & ControlMask) {
        Table = kiCtrl;
    } else if (State & AltMask) {
        Table = kiMeta;
    }

    // Look for the symbol (### should be binary search - but needs sorting)
    for (unsigned I = 0; I < sizeof (KeySymMap) / sizeof (KeySymMap [0]); I++) {
        if (KeySymMap [I].Sym == Sym) {
            // Found
            Key K = KeySymMap [I].Keys [Table];
            if (K != kbNoKey) {
                KbdBuffer.Put (K);
            }
            return;
        }
    }

    // Not found - ignore the key
}



static void EventLoop ()
// Get all waiting events and handle them
{
    // Read input events
    while (XEventsQueued (SpunkDisplay, QueuedAfterFlush) != 0) {

        // Read an event
        XEvent Event;
        XNextEvent (SpunkDisplay, &Event);

        //
        switch (Event.type) {

            case Expose:
                if (Event.xexpose.count == 0) {
                    // Calculate the area to redraw
                    unsigned X1 = Event.xexpose.x / CharWidth;
                    unsigned Y1 = Event.xexpose.y / CharHeight;
                    unsigned X2 = (Event.xexpose.x + Event.xexpose.width - 1)
                                  / CharWidth;
                    unsigned Y2 = (Event.xexpose.y + Event.xexpose.height - 1)
                                  / CharHeight;
                    ScrUpdateArea (X1, Y1, X2 - X1 + 1, Y2 - Y1 + 1, 1);
                }
                break;

            case MappingNotify:
                XRefreshKeyboardMapping (&Event.xmapping);
                break;

            case KeyPress:
                KbdPress (Event);
                break;

            case ConfigureNotify:
                if (Event.xconfigure.width != int (ScreenWidth * CharWidth) ||
                    Event.xconfigure.height != int (ScreenHeight * CharHeight)) {
                    // Size has changed, notify the application
                    raise (SIGWINCH);
                }
                break;
        }
    }

    // Flush the outgoing event queue
    XFlush (SpunkDisplay);
}



static Key KbdMapExtended (Key K)
// Map an extended key to a virtual key. Return the virtual key if a map
// exists, otherwise return the key unchanged.
{
    for (unsigned I = 0; I < sizeof (VirtualMap) / sizeof (VirtualMap [0]); I++) {
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
    for (unsigned I = 0; I < sizeof (VirtualMap) / sizeof (VirtualMap [0]); I++) {
        if (VirtualMap [I].VK == K) {
            return VirtualMap [I].EK;
        }
    }
    return K;
}



/*****************************************************************************/
/*                                class Keyboard                             */
/*****************************************************************************/



Keyboard::Keyboard ():
    Console (1),
    TransTable (NULL)           // Don't need a translation table, translation
                                // is done via table in KbdPress
{
}



Keyboard::~Keyboard ()
{
    delete [] TransTable;
}



Key Keyboard::RawKey ()
// Get a raw (unmapped) key
{
    while (1) {

        // Get all waiting events
        EventLoop ();

        // End the loop if we got an input key
        if (!KbdBuffer.IsEmpty ()) {
            break;
        }

        // Get the file handle of the window connection
        int Handle = ConnectionNumber (SpunkDisplay);
        int Result;

        do {

            // No waiting events - call the applications idle function before
            // waiting for more
            Idle ();

            // Use select() with a timeout of 100ms to wait for new events
            timeval Timeout;
            Timeout.tv_usec = 100000;
            Timeout.tv_sec  = 0;

            // Set the file descriptor
            fd_set Desc;
            FD_ZERO (&Desc);
            FD_SET (Handle, &Desc);

            // Check the connection status
            Result = select (Handle+1, &Desc, NULL, NULL, &Timeout);
            CHECK (Result >= 0);

        } while (Result == 0);

    }

    // Return the key from the buffer
    return KbdBuffer.Get ();
}



void Keyboard::GetMappedKey (int Wait)
// Read keys until the needed key is not found in the mapper or until
// a valid sequence is found. If Wait is zero, return immediately if
// no match is found and no more keys are available.
{
    // Handling incoming events
    EventLoop ();

    // If we should not wait, check if keys are available before grabbing them
    if (Wait == 0 && KbdBuffer.IsEmpty ()) {
        // No keys available, bail out
        return;
    }

    // Get a key and remap it
    KeyBuf.Put (KbdMapExtended (RawKey ()));
}



String Keyboard::GetKeyName (Key K)
// Return a string describing the give key
{
    if (IsVirtualKey (K)) {
        // It is a virtual key, remap it to it's extended form
        K = KbdMapVirtual (K);
    }

    // Now return the key description
    return LoadMsg (MSGBASE_KBD + K);
}


