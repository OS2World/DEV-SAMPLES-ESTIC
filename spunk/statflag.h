/*****************************************************************************/
/*									     */
/*				  STATFLAG.H				     */
/*									     */
/* (C) 1995	Ullrich von Bassewitz					     */
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



// Flags for use with CreateStatusLine/PushStatusLine



#ifndef __STATFLAG_H
#define __STATFLAG_H



/*****************************************************************************/
/*		   Flags for creating standard status lines		     */
/*****************************************************************************/



// !!! Old names, do not use for future development !!!
const u32 siF1_Help		= 0x00000001;
const u32 siEsc_Abort		= 0x00000002;
const u32 siEsc_End		= 0x00000004;
const u32 siEsc_Proceed		= 0x00000008;
const u32 siF10_Accept		= 0x00000010;
const u32 siCR_Accept		= 0x00000020;
const u32 siUpDn_Select		= 0x00000040;
const u32 siUpDnCR_Select	= 0x00000080;
const u32 siIns_Insert		= 0x00000100;
const u32 siDel_Delete		= 0x00000200;
const u32 siCR_Change		= 0x00000400;
const u32 siCtrlD_Print		= 0x00000800;
const u32 siCtrlG_Graphics	= 0x00001000;
const u32 siAltX_Exit		= 0x00002000;
const u32 siCR_Confirm		= 0x00004000;
const u32 siCursPgKeys_Move	= 0x00008000;
const u32 siF3_Open		= 0x00010000;
const u32 siAltF3_Close		= 0x00020000;
const u32 siF5_Zoom		= 0x00040000;
const u32 siCtrlF5_Resize	= 0x00080000;
const u32 siAltI_Login		= 0x00100000;
const u32 siAltO_Logout		= 0x00200000;



// New codes					-> Example
const u32 siHelp		= 0x00000001;	// F1 Help
const u32 siAbort		= 0x00000002;	// ESC Abort
const u32 siEnd			= 0x00000004;	// ESC Done
const u32 siProceed		= 0x00000008;	// ESC Proceed
const u32 siAccept		= 0x00000010;	// F10 Accept
const u32 siEnter		= 0x00000020;	// <ды Accept
const u32 siSelectKeys		= 0x00000040;	//  Select
const u32 siSelectChooseKeys	= 0x00000080;	// <ды Select
const u32 siInsert		= 0x00000100;	// Ins Insert
const u32 siDelete		= 0x00000200;	// Del Delete
const u32 siChange		= 0x00000400;	// <ды Change
const u32 siPrint		= 0x00000800;	// Alt-D Print
const u32 siGraphics		= 0x00001000;	// Alt-G Graphics
const u32 siExit		= 0x00002000;	// Alt-X Exit
const u32 siConfirm		= 0x00004000;	// Enter Confirm
const u32 siPageKeys		= 0x00008000;	//  PgUp PgDn Move
const u32 siOpen		= 0x00010000;	// F3 Open
const u32 siSave		= 0x00020000;	// F2 Save
const u32 siClose		= 0x00040000;	// Alt-F3 Close
const u32 siZoom		= 0x00080000;	// F5 Zoom
const u32 siResize		= 0x00100000;	// Ctrl-F5 Resize
const u32 siLogin		= 0x00200000;	// Alt-I Login
const u32 siLogout		= 0x00400000;	// Alt-O Logout
const u32 siMoveKeys		= 0x10000000;	// <-> Move
const u32 siResizeKeys		= 0x20000000;	// Shift-<-> Resize



// End of STATFLAG.H

#endif

