/*****************************************************************************/
/*									     */
/*				    MSGID.H				     */
/*									     */
/* (C) 1993-95	Ullrich von Bassewitz					     */
/*		Zwehrenbuehlstrasse 33					     */
/*		D-72070 Tuebingen					     */
/* EMail:	uz@ibb.schwaben.com					     */
/*									     */
/*****************************************************************************/



// $Id$
//
// $Log$
//
//



#ifndef __MSGID_H
#define __MSGID_H



#include "machine.h"



/*****************************************************************************/
/*			     Base ids for messages			     */
/*****************************************************************************/



const u16 MSGBASE_STATLINE		=	0;
const u16 MSGBASE_STDMENUE		=     100;
const u16 MSGBASE_STDMSG		=     200;
const u16 MSGBASE_MENUEDIT		=     300;
const u16 MSGBASE_MENUITEM		=     400;
const u16 MSGBASE_STRPARSE		=     500;
const u16 MSGBASE_INIFILE		=     600;
const u16 MSGBASE_PASSWORD		=     700;
const u16 MSGBASE_DATETIME		=     800;
const u16 MSGBASE_PROGRAM		=     900;
const u16 MSGBASE_KBD			=    1024;	// Needs 0x400 msgs!
const u16 MSGBASE_FILESEL		=    2100;
const u16 MSGBASE_WINSIZE		=    2200;
const u16 MSGBASE_SYSERROR		=    2300;	// Reserve 200 messages
const u16 MSGBASE_WINMGR		=    2500;



// END of MSGID.H

#endif

