/*****************************************************************************/
/*                                                                           */
/*                                 KEYDEF.H                                  */
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



#ifndef __KEYDEF_H
#define __KEYDEF_H



#include "machine.h"



/*****************************************************************************/
/*                                 Keycodes                                  */
/*****************************************************************************/



// Type of a key
typedef u16     Key;



// "Special" key
static const Key        kbNoKey         = 0x0000;



/*****************************************************************************/
/*                             "Plain" Keycodes                              */
/*****************************************************************************/



// Keys without any modifiers
static const Key        kbEnter         = 0x000D;
static const Key        kbEsc           = 0x001B;
static const Key        kbBack          = 0x0008;

static const Key        kbTab           = 0x0009;



// Keys with ctrl modifier
static const Key        kbCtrlA         = 0x0001;
static const Key        kbCtrlB         = 0x0002;
static const Key        kbCtrlC         = 0x0003;
static const Key        kbCtrlD         = 0x0004;
static const Key        kbCtrlE         = 0x0005;
static const Key        kbCtrlF         = 0x0006;
static const Key        kbCtrlG         = 0x0007;
static const Key        kbCtrlH         = 0x0008;
static const Key        kbCtrlI         = 0x0009;
static const Key        kbCtrlJ         = 0x000A;
static const Key        kbCtrlK         = 0x000B;
static const Key        kbCtrlL         = 0x000C;
static const Key        kbCtrlM         = 0x000D;
static const Key        kbCtrlN         = 0x000E;
static const Key        kbCtrlO         = 0x000F;
static const Key        kbCtrlP         = 0x0010;
static const Key        kbCtrlQ         = 0x0011;
static const Key        kbCtrlR         = 0x0012;
static const Key        kbCtrlS         = 0x0013;
static const Key        kbCtrlT         = 0x0014;
static const Key        kbCtrlU         = 0x0015;
static const Key        kbCtrlV         = 0x0016;
static const Key        kbCtrlW         = 0x0017;
static const Key        kbCtrlX         = 0x0018;
static const Key        kbCtrlY         = 0x0019;
static const Key        kbCtrlZ         = 0x001A;



inline int IsPlainKey (Key K)
// Determine if K is a normal keycode
{
    return ((K & 0xFF00) == 0x0000);
}



/*****************************************************************************/
/*                             Extended Keycodes                             */
/*****************************************************************************/



static const Key        kbF1            = 0x013B;
static const Key        kbF2            = 0x013C;
static const Key        kbF3            = 0x013D;
static const Key        kbF4            = 0x013E;
static const Key        kbF5            = 0x013F;
static const Key        kbF6            = 0x0140;
static const Key        kbF7            = 0x0141;
static const Key        kbF8            = 0x0142;
static const Key        kbF9            = 0x0143;
static const Key        kbF10           = 0x0144;
static const Key        kbF11           = 0x0185;
static const Key        kbF12           = 0x0186;

static const Key        kbUp            = 0x0148;
static const Key        kbDown          = 0x0150;
static const Key        kbLeft          = 0x014B;
static const Key        kbRight         = 0x014D;
static const Key        kbPgDn          = 0x0151;
static const Key        kbPgUp          = 0x0149;
static const Key        kbIns           = 0x0152;
static const Key        kbDel           = 0x0153;
static const Key        kbHome          = 0x0147;
static const Key        kbEnd           = 0x014F;



static const Key        kbCtrlF1        = 0x015E;
static const Key        kbCtrlF2        = 0x015F;
static const Key        kbCtrlF3        = 0x0160;
static const Key        kbCtrlF4        = 0x0161;
static const Key        kbCtrlF5        = 0x0162;
static const Key        kbCtrlF6        = 0x0163;
static const Key        kbCtrlF7        = 0x0164;
static const Key        kbCtrlF8        = 0x0165;
static const Key        kbCtrlF9        = 0x0166;
static const Key        kbCtrlF10       = 0x0167;
static const Key        kbCtrlF11       = 0x0189;
static const Key        kbCtrlF12       = 0x018A;

static const Key        kbCtrlTab       = 0x0194;
static const Key        kbCtrlUp        = 0x018D;
static const Key        kbCtrlDown      = 0x0191;
static const Key        kbCtrlLeft      = 0x0173;
static const Key        kbCtrlRight     = 0x0174;
static const Key        kbCtrlPgDn      = 0x0176;
static const Key        kbCtrlPgUp      = 0x0184;

static const Key        kbCtrlIns       = 0x0104;
static const Key        kbCtrlDel       = 0x0106;
static const Key        kbCtrlHome      = 0x0177;
static const Key        kbCtrlEnd       = 0x0175;


// Keys with meta prefix
static const Key        kbMeta1         = 0x0178;
static const Key        kbMeta2         = 0x0179;
static const Key        kbMeta3         = 0x017A;
static const Key        kbMeta4         = 0x017B;
static const Key        kbMeta5         = 0x017C;
static const Key        kbMeta6         = 0x017D;
static const Key        kbMeta7         = 0x017E;
static const Key        kbMeta8         = 0x017F;
static const Key        kbMeta9         = 0x0180;
static const Key        kbMeta0         = 0x0181;

static const Key        kbMetaA         = 0x011E;
static const Key        kbMetaB         = 0x0130;
static const Key        kbMetaC         = 0x012E;
static const Key        kbMetaD         = 0x0120;
static const Key        kbMetaE         = 0x0112;
static const Key        kbMetaF         = 0x0121;
static const Key        kbMetaG         = 0x0122;
static const Key        kbMetaH         = 0x0123;
static const Key        kbMetaI         = 0x0117;
static const Key        kbMetaJ         = 0x0124;
static const Key        kbMetaK         = 0x0125;
static const Key        kbMetaL         = 0x0126;
static const Key        kbMetaM         = 0x0132;
static const Key        kbMetaN         = 0x0131;
static const Key        kbMetaO         = 0x0118;
static const Key        kbMetaP         = 0x0119;
static const Key        kbMetaQ         = 0x0110;
static const Key        kbMetaR         = 0x0113;
static const Key        kbMetaS         = 0x011F;
static const Key        kbMetaT         = 0x0114;
static const Key        kbMetaU         = 0x0116;
static const Key        kbMetaV         = 0x012F;
static const Key        kbMetaW         = 0x0111;
static const Key        kbMetaX         = 0x012D;
static const Key        kbMetaY         = 0x0115;
static const Key        kbMetaZ         = 0x012C;


static const Key        kbMetaF1        = 0x0168;
static const Key        kbMetaF2        = 0x0169;
static const Key        kbMetaF3        = 0x016A;
static const Key        kbMetaF4        = 0x016B;
static const Key        kbMetaF5        = 0x016C;
static const Key        kbMetaF6        = 0x016D;
static const Key        kbMetaF7        = 0x016E;
static const Key        kbMetaF8        = 0x016F;
static const Key        kbMetaF9        = 0x0170;
static const Key        kbMetaF10       = 0x0171;
static const Key        kbMetaF11       = 0x018B;
static const Key        kbMetaF12       = 0x018C;

static const Key        kbMetaEsc       = 0x0101;
static const Key        kbMetaSpace     = 0x0102;
static const Key        kbMetaTab       = 0x01A5;
static const Key        kbMetaPgDn      = 0x01A1;
static const Key        kbMetaPgUp      = 0x0199;

static const Key        kbMetaIns       = 0x01A2;
static const Key        kbMetaDel       = 0x01A3;
static const Key        kbMetaHome      = 0x0197;
static const Key        kbMetaEnd       = 0x019F;

static const Key        kbMetaLeft      = 0x019B;
static const Key        kbMetaRight     = 0x019D;
static const Key        kbMetaUp        = 0x0198;
static const Key        kbMetaDown      = 0x01A0;


// Keys with shift modifier
static const Key        kbShiftF1       = 0x0154;
static const Key        kbShiftF2       = 0x0155;
static const Key        kbShiftF3       = 0x0156;
static const Key        kbShiftF4       = 0x0157;
static const Key        kbShiftF5       = 0x0158;
static const Key        kbShiftF6       = 0x0159;
static const Key        kbShiftF7       = 0x015A;
static const Key        kbShiftF8       = 0x015B;
static const Key        kbShiftF9       = 0x015C;
static const Key        kbShiftF10      = 0x015D;
static const Key        kbShiftF11      = 0x0187;
static const Key        kbShiftF12      = 0x0188;

static const Key        kbShiftTab      = 0x010F;

static const Key        kbShiftIns      = 0x0105;
static const Key        kbShiftDel      = 0x0107;



// Some keys used in linux. Note: Those are not supported by the PC hardware
// but mapped by key sequences from the key mapper. They are used to define
// virtual keys when the corresponding extended keys are not available.
// Beware: Don't use numbers defined above!

static const Key        kbEscCtrlA      = 0x01B0;
static const Key        kbEscCtrlB      = 0x01B1;
static const Key        kbEscCtrlC      = 0x01B2;
static const Key        kbEscCtrlD      = 0x01B3;
static const Key        kbEscCtrlE      = 0x01B4;
static const Key        kbEscCtrlF      = 0x01B5;
static const Key        kbEscCtrlG      = 0x01B6;
static const Key        kbEscCtrlH      = 0x01B7;
static const Key        kbEscCtrlI      = 0x01B8;
static const Key        kbEscCtrlJ      = 0x01B9;
static const Key        kbEscCtrlK      = 0x01BA;
static const Key        kbEscCtrlL      = 0x01BB;
static const Key        kbEscCtrlM      = 0x01BC;
static const Key        kbEscCtrlN      = 0x01BD;
static const Key        kbEscCtrlO      = 0x01BE;
static const Key        kbEscCtrlP      = 0x01BF;
static const Key        kbEscCtrlQ      = 0x01C0;
static const Key        kbEscCtrlR      = 0x01C1;
static const Key        kbEscCtrlS      = 0x01C2;
static const Key        kbEscCtrlT      = 0x01C3;
static const Key        kbEscCtrlU      = 0x01C4;
static const Key        kbEscCtrlV      = 0x01C5;
static const Key        kbEscCtrlW      = 0x01C6;
static const Key        kbEscCtrlX      = 0x01C7;
static const Key        kbEscCtrlY      = 0x01C8;
static const Key        kbEscCtrlZ      = 0x01C9;

static const Key        kbEscEsc        = 0x01CA;       // Mapped to vkAbort

static const Key        kbCtrlQS        = 0x01CB;       // Mapped to vkHome
static const Key        kbCtrlQD        = 0x01CC;       // Mapped to vkEnd
static const Key        kbCtrlQR        = 0x01CD;       // Mapped to vkCtrlPgUp
static const Key        kbCtrlQC        = 0x01CE;       // Mapped to vkCtrlPgDn
static const Key        kbCtrlQE        = 0x01CF;       // Mapped to vkCtrlHome
static const Key        kbCtrlQX        = 0x01D0;       // Mapped to vkCtrlEnd



inline int IsExtendedKey (Key K)
// Determine if K is a extended keycode
{
    return ((K & 0xFF00) == 0x0100);
}



/*****************************************************************************/
/*                             Virtual Keycodes                              */
/*****************************************************************************/



// The following keys are garantied to exist
static const Key        vkAbort         = 0x0201;
static const Key        vkHelp          = 0x0202;
static const Key        vkAccept        = 0x0203;

static const Key        vkPgUp          = 0x0210;
static const Key        vkPgDn          = 0x0211;
static const Key        vkCtrlPgUp      = 0x0212;
static const Key        vkCtrlPgDn      = 0x0213;

static const Key        vkUp            = 0x0214;
static const Key        vkDown          = 0x0215;
static const Key        vkLeft          = 0x0216;
static const Key        vkRight         = 0x0217;
static const Key        vkCtrlUp        = 0x0218;
static const Key        vkCtrlDown      = 0x0219;
static const Key        vkCtrlLeft      = 0x021A;
static const Key        vkCtrlRight     = 0x021B;

static const Key        vkIns           = 0x021C;
static const Key        vkDel           = 0x021D;
static const Key        vkHome          = 0x021E;
static const Key        vkEnd           = 0x021F;
static const Key        vkCtrlIns       = 0x0220;
static const Key        vkCtrlDel       = 0x0221;
static const Key        vkCtrlHome      = 0x0222;
static const Key        vkCtrlEnd       = 0x0223;

static const Key        vkZoom          = 0x0230;
static const Key        vkClose         = 0x0231;
static const Key        vkOpen          = 0x0232;
static const Key        vkResize        = 0x0233;
static const Key        vkQuit          = 0x0234;
static const Key        vkSave          = 0x0235;



inline int IsVirtualKey (Key K)
// Determine if K is a virtual keycode
{
    return ((K & 0xFF00) == 0x0200);
}



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



Key GetMetaCode (Key K);
// Return the "meta version" of the given key K or kbNoKey if none exists



Key GetMetaKey (Key K);
// Return the "normal key" of the meta key given key K or kbNoKey if none
// exists



// End of KEYDEF.H

#endif

