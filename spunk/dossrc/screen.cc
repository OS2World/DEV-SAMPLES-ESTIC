/*****************************************************************************/
/*                                                                           */
/*                                  SCREEN.CC                                */
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



// This file is also used for the 32-bit version of the library. The define
// DOS32 is used to control the compile target. The functions in the first
// section (labeled "Target specific code") are the only ones that are
// target specific.
// Note: Because the Borland and Watcom compilers use different notations
// when accessing the word registers in a REGS strcuture, all assignments
// to those registers are splitted in two byte assignments. If you change
// this, the module will no longer compatible with all supported compilers.



#include <malloc.h>
#if defined (__WATCOMC__)
#include <i86.h>
#include <conio.h>
#elif defined (__BORLANDC__) || defined (__GO32__)
#include <dos.h>
#else
#error Unknown compiler!
#endif
#if defined (__GO32__)
#include <go32.h>
#include <sys\farptr.h>
#endif

#include "screen.h"


// Instance of the screen class to handle screen output. Must be initialized
// from outside (Application)
Screen* TheScreen;



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



// These are defined as constants instead using the numerical value. If you
// want to create a 16 bit dpmi version, change them to variables containing
// the corresponding selectors.
const u16 Seg0040       = 0x0040;
const u16 SegB000       = 0xB000;
const u16 SegB800       = 0xB800;



// Constants for ScrGetCard
static const u16 vcMono         = 0x0000;
static const u16 vcCGA          = 0x0001;
static const u16 vcEGA          = 0x0002;
static const u16 vcVGA          = 0x0003;



/*****************************************************************************/
/*                           Target specific code                            */
/*****************************************************************************/



#ifdef DOS
#define PTR(__s,__o)    MK_FP(__s,__o)
#endif
#ifdef DOS32
#define PTR(__s,__o)    ((((u32)(u16)(__s)) << 4) + ((u16)(__o)))
#endif

#define BYTEPTR(__s,__o)        ((unsigned char*) PTR (__s,__o))
#define WORDPTR(__s,__o)        ((u16*) PTR (__s,__o))



#if defined (__GO32__)

// Remapping of some djgpp function names
#define _disable()              disable ()
#define _enable()               enable ()
#define outp(__a,__v)           outportb (__a, __v)
#define outpw(__a, __v)         outportw (__a, __v)
#define inp(__a)                inportb (__a)
#define inpw(__a)               inportw (__a)

// DJGPP specific stuff for accessing low memory
#define DosSel                  _go32_conventional_mem_selector ()
#define PeekByte(__s, __o)      _farpeekb (DosSel, PTR (__s, __o))
#define PeekWord(__s, __o)      _farpeekw (DosSel, PTR (__s, __o))
#define PokeByte(__s, __o, __v) _farpokeb (DosSel, PTR (__s, __o), __v)
#define PokeWord(__s, __o, __v) _farpokew (DosSel, PTR (__s, __o), __v)

#else

// Stuff for accessing low memory
#define PeekByte(__s, __o)      *BYTEPTR (__s, __o)
#define PeekWord(__s, __o)      *WORDPTR (__s, __o)
#define PokeByte(__s, __o, __v) *BYTEPTR (__s, __o) = (__v)
#define PokeWord(__s, __o, __v) *WORDPTR (__s, __o) = (__v)

#endif



/*****************************************************************************/
/*                      Code for tweaking vga registers                      */
/*****************************************************************************/



inline void VideoInt (REGS& Regs)
// execute a software interrupt
{
#if defined (DOS32) && defined (__WATCOMC__)
    int386 (0x10, &Regs, &Regs);
#else
    int86 (0x10, &Regs, &Regs);
#endif
}



static void VGAOff ()
// Blank the vga display. Must be called with interrupts disabled!
{
    outpw (0x3C4, 0x100);
    outp  (0x3D4, 0x17);
    outp  (0x3D5, inp (0x3D5) & 0x7F);
    outp  (0x3D4, 0x11);
    outp  (0x3D5, inp (0x3D5) & 0x7F);
}



static void VGAOn ()
// re-enable the vga display.
{
    outp  (0x3D4, 0x11);
    outp  (0x3D5, inp (0x3D5) | 0x80);
    outp  (0x3D4, 0x17);
    outp  (0x3D5, inp (0x3D5) | 0x80);
    outpw (0x3C4, 0x300);
}



static void VGA94Cols ()
// Switch the vga hardware to handle 846 pixels horizontally
{
    outp  (0x3C2, (inp (0x3CC) & 0xF3) | 0x04);
    outp  (0x3C4, 0x01);
    outp  (0x3C5, inp (0x3C5) | 0x01);
    outpw (0x3D4, 0x6C00);
    outpw (0x3D4, 0x5D01);
    outpw (0x3D4, 0x5E02);
    outpw (0x3D4, 0x8F03);
    outpw (0x3D4, 0x6204);
    outpw (0x3D4, 0x8E05);
    outpw (0x3D4, 0x2F13);
    inp   (0x3DA);
    outp  (0x3C0, 0x13);
    outp  (0x3C0, 0x00);
    outp  (0x3C0, 0x20);
    outp  (0x3C0, 0x20);
}



static void VGA480Scanlines ()
// Switch the vga to 480 lines
{
    outp  (0x3C2, inp (0x3CC) | 0xC0);
    outpw (0x3D4, 0x0B06);
    outpw (0x3D4, 0x3E07);
    outpw (0x3D4, 0x4F09);
    outpw (0x3D4, 0xEA10);
    outpw (0x3D4, 0x8C11);
    outpw (0x3D4, 0xDF12);
    outpw (0x3D4, 0xE715);
    outpw (0x3D4, 0x0416);
}



static void VGACharHeight (unsigned char H)
// Set the character height in scan lines
{
    // Set the bios value
    PokeByte (Seg0040, 0x0085, H);

    // program the vga
    outp (0x3D4, 0x09);
    outp (0x3D5, (inp (0x3D5) & 0xE0) + H - 1);
    outp (0x3D4, 0x0A);
    outp (0x3D5, H <= 12 ? H-2 : H-3);
    outp (0x3D4, 0x0B);
    outp (0x3D5, H <= 12 ? H-1 : H-2);
}



static void VGASetTweakedMode (unsigned Cols, unsigned Lines)
// Set one of the tweaked VGA modes. A vga must be present if this function is
// called!
{
    REGS Regs;

    // Switch to 350 or 400 scanlines. If we need 480, switch to 400 here
    Regs.h.ah = 0x12;
    Regs.h.bl = 0x30;
    Regs.h.al = (Lines == 43) ? 0x01 : 0x02;
    VideoInt (Regs);

    // Setup 80 cols by switching to mode 3 (CO80)
    Regs.h.ah = 0x00;
    Regs.h.al = 0x03;
    VideoInt (Regs);

    // Load the font for the choosen resolution
    Regs.h.ah = 0x11;
    Regs.h.bl = 0x00;
    unsigned Height;
    switch (Lines) {

        case 25:
        case 30:
            Regs.h.al = 0x04;           // 16x8 font
            Height = 16;
            break;

        case 34:
            Regs.h.al = 0x01;           // 14x8 font
            Height = 14;
            break;

        default:
            Regs.h.al = 0x02;           // 8x8 font
            Height = 8;
            break;

    }
    VideoInt (Regs);

    // Program the vga controller
    _disable ();
    VGAOff ();
    if (Cols == 94) {
        VGA94Cols ();
    }
    if (Lines == 30 || Lines == 34 || Lines == 60) {
        VGA480Scanlines ();
    }
    VGACharHeight (Height);
    VGAOn ();
    _enable ();

    // Set the BIOS variables for the selected screen size
    PokeByte (Seg0040, 0x004A, Cols);
    PokeByte (Seg0040, 0x0084, Lines-1);
}



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



inline u16 ScrGetMode ()
// return the current video mode
{
    return PeekByte (Seg0040, 0x0049);
}



inline u16 ScrGetCursor ()
// get the current cursor type
{
    return PeekWord (Seg0040, 0x0060);
}



inline u16 ScrGetColsPerLine ()
// get the columns per line from the BIOS segment
{
    return PeekByte (Seg0040, 0x004A);
}



static int ScrIsVGA ()
// Return 1 if the video card is VGA
{
    REGS Regs;
    Regs.h.ah = 0x1A;
    Regs.h.al = 0x00;
    VideoInt (Regs);
    return Regs.h.al == 0x1A;
}



inline void ScrWriteBuf (u16* Buf, u16 X, u16 Y, unsigned Count)
// write a row of char/attribute pairs to the screen
{
#if defined (__GO32__)

    // Get a pointer to the screen base and add row/col offset
    u32 ScrPtr = ScrGetMode () == 0x07 ? 0xB0000 : 0xB8000;
    ScrPtr += (Y * ScrGetColsPerLine () + X) * sizeof (u16);

    // Copy the buffer to the screen
    dosmemput (Buf, Count * sizeof (u16), ScrPtr);

#else

    // Get a pointer to the screen base and add row/col offset
    u16* ScrPtr;
    if (ScrGetMode () == 0x07) {
        // mono
        ScrPtr = WORDPTR (SegB000, 0);
    } else {
        ScrPtr = WORDPTR (SegB800, 0);
    }
    ScrPtr += Y * ScrGetColsPerLine () + X;

    // Copy the buffer to the screen
    memcpy (ScrPtr, Buf, Count * sizeof (u16));

#endif
}



static void ScrSetCursorPos (unsigned char X, unsigned char Y)
{
    REGS Regs;

    Regs.h.dh = Y;
    Regs.h.dl = X;
    Regs.h.bh = 0x00;                   // Display page
    Regs.h.ah = 0x02;                   // Function code
    VideoInt (Regs);
}



static void ScrSetMode (u16 Mode)
// Set a video mode
{
    REGS Regs;

    switch (Mode) {

        case vmBW40:
        case vmCO40:
        case vmBW80:
        case vmCO80:
        case vmMono:
        case vmET4_100x40:
            Regs.h.al = Mode & 0x00FF;
            Regs.h.ah = 0x00;
            VideoInt (Regs);
            break;

        case vmVGA_80x30:
            VGASetTweakedMode (80, 30);
            break;

        case vmVGA_80x34:
            VGASetTweakedMode (80, 34);
            break;

        case vmVGA_80x43:
            VGASetTweakedMode (80, 43);
            break;

        case vmVGA_80x50:
            VGASetTweakedMode (80, 50);
            break;

        case vmVGA_80x60:
            VGASetTweakedMode (80, 60);
            break;

        case vmVGA_94x25:
            VGASetTweakedMode (94, 25);
            break;

        case vmVGA_94x30:
            VGASetTweakedMode (94, 30);
            break;

        case vmVGA_94x34:
            VGASetTweakedMode (94, 34);
            break;

        case vmVGA_94x43:
            VGASetTweakedMode (94, 43);
            break;

        case vmVGA_94x50:
            VGASetTweakedMode (94, 50);
            break;

        case vmVGA_94x60:
            VGASetTweakedMode (94, 60);
            break;

    }
}



static void ScrSetCursor (u16 Cursor)
// set a cursor
{
    REGS Regs;

    Regs.h.ch = Cursor >> 8;
    Regs.h.cl = Cursor & 0x00FF;
    Regs.h.ah = 0x01;
    VideoInt (Regs);
}



static int ScrColorMode ()
// return 0 in mono mode, 1 in color mode
{
    switch (ScrGetMode ()) {

        case 0: // BW40
        case 2: // BW80
        case 7: // Mono
            return 0;

        default:
            return 1;

    }
}



static void ScrScreenSize (u16& XSize, u16& YSize)
// Get screen dimensions
{
    REGS Regs;

    // Get the video mode and row count
    Regs.h.ah = 0x0F;
    VideoInt (Regs);
    XSize = Regs.h.ah;

    // Get line count
    Regs.h.ah = 0x11;
    Regs.h.al = 0x30;
    Regs.h.bh = 0x00;
    Regs.h.dl = 0x00;
    VideoInt (Regs);
    if (Regs.h.dl == 0x00) {
        // line count not set, assume 25 lines
        YSize = 25;
    } else {
        YSize = Regs.h.dl + 1;
    }
}



static u16 ScrGetCard ()
// Return the type of the video card installed. Used to set the
// cursor type correctly
{
    REGS Regs;

    if (ScrGetMode () == 0x07) {

        // Mono mode, check for a vga in monochrome mode
        return ScrIsVGA () ? vcVGA : vcMono;

    } else {

        // It is some sort of a color video card.
        // The following code is taken from the initialization part of
        // the crt unit as documented in Arne Schaepers, "Turbo Pascal 4.0"
        // The function call with ax = 1130h returns on EGA/VGA the number
        // of lines in dl, on a CGA dl is not changed.

        Regs.h.dl = 0x00;                       // Preset line count = 0
        Regs.h.bh = 0x00;
        Regs.h.ah = 0x11;
        Regs.h.al = 0x30;
        VideoInt (Regs);

        if (Regs.h.dl == 0x00) {
            // CGA card returns no line count
            return vcCGA;
        }

        // Remainig cards are EGA and VGA
        return ScrIsVGA () ? vcVGA : vcEGA;

    }
}



/*****************************************************************************/
/*                               class Screen                                */
/*****************************************************************************/



Screen::Screen ():
    Console (1),
    CP437 (1),
    TransTable (NULL)
{
    // Remember old video mode and cursor form
    StartupMode = ScrGetMode ();
    StartupCursor = ScrGetCursor ();

    // Remember the current mode
    CurrentMode = ScrGetMode ();

    // Get size of screen
    ScrScreenSize (XSize, YSize);

    // Check if color mode
    Color = ScrColorMode ();

    // Switch cursor off
    SetCursorOff ();
}



Screen::~Screen ()
{
    // Reset old video mode and cursor
    SetMode (StartupMode);
    ScrSetCursor (StartupCursor);

    // Delete the translation table
    delete [] TransTable;
}



u16* Screen::Translate (u16* Target, u16* Source, unsigned Len)
// Translate a complete buffer via the translation table
{
    if (TransTable) {
        unsigned char* S = (unsigned char*) Target;
        unsigned char* T = (unsigned char*) Source;
        while (Len--) {
            *T++ = TransTable [*S++];           // Translate character
            *T++ = *S++;                        // Copy attribute
        }
        return Target;
    } else {
        // No translation table, return the source buffer
        return Source;
    }
}



void Screen::SetMode (u16 Mode)
{
    // Remember mode
    CurrentMode = Mode;

    // set mode
    ScrSetMode (Mode);

    // Get size of screen
    ScrScreenSize (XSize, YSize);

    // Check if color mode
    Color = ScrColorMode ();

    // Switch cursor off
    SetCursorOff ();
}



void Screen::SetCursorOn ()
{
    static u16 CursorTab [4] = {
        0x0B0C,                 // Mono
        0x0607,                 // CGA
        0x0506,                 // EGA
        0x0506                  // VGA
    };

    // Set the cursor according to the video card type
    ScrSetCursor (CursorTab [ScrGetCard ()]);
}



void Screen::SetCursorOff ()
{
    static u16 CursorTab [4] = {
        0x1000,                 // Mono
        0x8F8F,                 // CGA
        0x8F8F,                 // EGA
        0x2000                  // VGA
    };

    // Set the cursor according to the video card type
    ScrSetCursor (CursorTab [ScrGetCard ()]);
}



void Screen::SetCursorFat ()
{
    static u16 CursorTab [4] = {
        0x020B,                 // Mono
        0x040B,                 // CGA
        0x0206,                 // EGA
        0x0206                  // VGA
    };

    // Set the cursor according to the video card type
    ScrSetCursor (CursorTab [ScrGetCard ()]);
}



void Screen::SetCursorPos (const Point &Pos)
{
    ScrSetCursorPos ((unsigned) Pos.X, (unsigned) Pos.Y);
}



void Screen::DisplayBuffer (const Rect& R, u16* Buf)
{
    int XtoDo, YtoDo;
    int XCount = R.XSize ();
    int YCount = R.YSize ();

    // Check if there is anything to do
    if (XCount == 0 || YCount == 0 || R.A.Y > YSize || R.A.X > XSize) {
        // Done
        return;
    }

    // Calculate the size of the output rectangle
    XtoDo = (R.B.X > XSize) ? XSize - R.A.X : XCount;
    YtoDo = (R.B.Y > YSize) ? YSize - R.A.Y : YCount;

    // If we have to translate the output, get buffer memory. This operation
    // is cheap, so do it, even if we don't need the buffer to avoid warnings
    u16* Buf2 = (u16*) alloca (XtoDo * sizeof (u16));

    // Write the stuff to the screen
    int Y = R.A.Y;
    while (YtoDo--) {

        if (TransTable) {
            // Translate the buffer line and write it to the screen
            ScrWriteBuf (Translate (Buf2, Buf, XtoDo), R.A.X, Y, XtoDo);
        } else {
            // No translation, just write out the buffer
            ScrWriteBuf (Buf, R.A.X, Y, XtoDo);
        }
        Buf += XCount;
        Y++;
    }

}



unsigned Screen::TerminalSpeed ()
// Get some information on the terminal speed. This will return a value
// between 0..10, where 10 is a very fast (direct access) screen and
// 0 is a very slow (300 baud) serial line.
// This may be used to change the amount of screen output, a program
// produces.
{
    // We have always direct access to the screen
    return 10;
}



