/*****************************************************************************/
/*									     */
/*				  CHARTYPE.H				     */
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



#ifndef __CHARTYPE_H
#define __CHARTYPE_H



#include <ctype.h>



/*****************************************************************************/
/*				     Data				     */
/*****************************************************************************/



// This is character set that contains all whitespace chars (as defined by
// IsSpace (c) != 0). The current definition assumes ASCII. This is not a
// problem but must be changed when porting to non-ASCII systems (probably
// by a #define). Beware: '\0' is classified _not_ as space to make coding
// easier.
extern const class CharSet WhiteSpace;



/*****************************************************************************/
/*		      Character classification functions		     */
/*****************************************************************************/



inline int IsAlpha (int C)
{
    return isascii (C) && isalpha (C);
}



inline int IsAlNum (int C)
{
    return isascii (C) && isalnum (C);
}



inline int IsAscii (int C)
{
    return isascii (C);
}



inline int IsCntrl (int C)
{
    return isascii (C) && iscntrl (C);
}



inline int IsDigit (int C)
{
    return isascii (C) && isdigit (C);
}



inline int IsGraph (int C)
{
    return isascii (C) && isgraph (C);
}



inline int IsPrint (int C)
{
    return isascii (C) && isprint (C);
}



inline int IsPunct (int C)
{
    return isascii (C) && ispunct (C);
}



inline int IsSpace (int C)
{
    return isascii (C) && isspace (C);
}



inline int IsXDigit (int C)
{
    return isascii (C) && isxdigit (C);
}



// End of CHARTYPE.H

#endif

