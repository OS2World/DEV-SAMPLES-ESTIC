/*****************************************************************************/
/*									     */
/*				  CHARTYPE.CC				     */
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



#include "charset.h"



/*****************************************************************************/
/*				     Data				     */
/*****************************************************************************/



// This is character set that contains all whitespace chars (as defined by
// IsSpace (c) != 0). The current definition assumes ASCII. This is not a
// problem but must be changed when porting to non-ASCII systems (probably
// by a #define). Beware: '\0' is classified _not_ as space to make coding
// easier.
extern const CharSet WhiteSpace ("\x01\x02\x03\x04\x05\x06\x07"
				 "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
				 "\x10\x11\x12\x13\x14\x15\x16\x17"
				 "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
				 " ");



