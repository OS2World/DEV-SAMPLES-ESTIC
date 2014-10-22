/*****************************************************************************/
/*									     */
/*				  STREAMID.H				     */
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



#ifndef _STREAMID_H
#define _STREAMID_H



#include "machine.h"



/*****************************************************************************/
/*				Streamable ID's                              */
/*****************************************************************************/



const u16 ID_Collection			= 0x0010;
const u16 ID_SortedCollection		= 0x0011;
const u16 ID_StringCollection		= 0x0012;
const u16 ID_MsgCollection		= 0x0013;
const u16 ID_Palette			= 0x0014;

const u16 ID_ResourceIndex		= 0x0020;
const u16 ID_ResourceCollection		= 0x0021;

const u16 ID_Window			= 0x0030;
const u16 ID_RootWindow			= 0x0031;
const u16 ID_StatusLine			= 0x0032;
const u16 ID_ItemWindow			= 0x0033;
const u16 ID_MenueBar			= 0x0034;
const u16 ID_TopMenueBar		= 0x0035;
const u16 ID_Menue			= 0x0036;
const u16 ID_FileViewer			= 0x0037;

const u16 ID_BitSet			= 0x0040;
const u16 ID_CharSet			= 0x0041;

const u16 ID_String			= 0x0051;
const u16 ID_Msg			= 0x0052;

const u16 ID_WindowItem			= 0x0060;
const u16 ID_ItemLabel			= 0x0061;
const u16 ID_TextItem			= 0x0062;

const u16 ID_MenueItem			= 0x0070;
const u16 ID_SubMenueItem		= 0x0071;
const u16 ID_MenueBarItem		= 0x0072;
const u16 ID_MenueLine			= 0x0073;

const u16 ID_LongItem			= 0x0074;
const u16 ID_StringItem			= 0x0075;
const u16 ID_HexItem			= 0x0076;
const u16 ID_ToggleItem			= 0x0077;
const u16 ID_OffOnItem			= 0x0078;
const u16 ID_NoYesItem			= 0x0079;
const u16 ID_FloatItem			= 0x007A;
const u16 ID_TimeItem			= 0x007B;
const u16 ID_DateItem			= 0x007C;
const u16 ID_RStringItem		= 0x007D;

const u16 ID_EditLine			= 0x0090;
const u16 ID_FloatEdit			= 0x0091;
const u16 ID_LongEdit			= 0x0092;
const u16 ID_HexEdit			= 0x0093;
const u16 ID_TextEdit			= 0x0094;
const u16 ID_TimeEdit			= 0x0095;
const u16 ID_DateEdit			= 0x0096;
const u16 ID_PasswordEdit		= 0x0097;
const u16 ID_FileEdit			= 0x0098;
const u16 ID_FileNameEdit		= 0x0099;

const u16 ID_Container			= 0x00B0;

const u16 ID_Time			= 0x00C0;
const u16 ID_TimeDiff			= 0x00C1;

const u16 ID_ComPort			= 0x00D0;

const u16 ID_PasswordEntry		= 0x00E0;
const u16 ID_PasswordColl		= 0x00E1;
const u16 ID_PWLogEntry			= 0x00E2;

const u16 ID_StringPool			= 0x00F0;

const u16 ID_FileInfo			= 0x0100;
const u16 ID_FileInfoColl		= 0x0101;

const u16 ID_MemoryStream		= 0x0110;

const u16 ID_WindowManager		= 0x0120;
const u16 ID_WinColl			= 0x0121;

const u16 ID_RNG			= 0x0130;



// First user ID
const u16 ID_USER			= 0x1000;



// END of STREAMID.H

#endif
