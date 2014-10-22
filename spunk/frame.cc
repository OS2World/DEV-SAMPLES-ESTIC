/*****************************************************************************/
/*									     */
/*				   FRAME.CC				     */
/*									     */
/* (C) 1995-96	Ullrich von Bassewitz					     */
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



/*****************************************************************************/
/*		      Definition of the frame characters		     */
/*****************************************************************************/



// These are some predefined arrays with characters that are used in window
// frames, horizontal lines and more

// Codepage 850 (works also for CP 437)
unsigned char CP850_InactiveFrame [12] = {
    0xda, 0xbf, 0xc0, 0xd9, 0xc4, 0xb3, 0xc3, 0xb4, 0xc2, 0xc1, 0xc5, 0x00
//   Ú	   ¿	  À	Ù     Ä     ³	  Ã    ´      Â     Á	 Å

};
unsigned char CP850_ActiveFrame [12] = {
    0xc9, 0xbb, 0xc8, 0xbc, 0xcd, 0xba, 0xcc, 0xb9, 0xcb, 0xca, 0xca, 0x00
};

// KOI-8r according to Michael A. Rodiono (marod@piglet.cnit.nsk.su)
unsigned char KOI8r_InactiveFrame [12] = {
    130, 131, 132, 133, 128, 129, 134, 135, 136, 137, 129, 0
};
unsigned char KOI8r_ActiveFrame [12] = {
    165, 168, 171, 174, 000, 000, 177, 181, 184, 187, 190, 0
};

// No linedrawing support
unsigned char SimpleFrame [12] = "++++-|+++++";

// This are the variables that are used to access the arrays above. They are
// initialized to the CP 437 specific frame arrays
unsigned char* InactiveFrame = CP850_InactiveFrame;
unsigned char* ActiveFrame   = CP850_ActiveFrame;




