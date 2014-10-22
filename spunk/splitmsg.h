/*****************************************************************************/
/*                                                                           */
/*                                SPLITMSG.H                                 */
/*                                                                           */
/* (C) 1993,94  Ullrich von Bassewitz                                        */
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



#ifndef __SPLITMSG_H
#define __SPLITMSG_H


#include "listnode.h"
#include "rect.h"
#include "str.h"



/*****************************************************************************/
/*                                                                           */
/*****************************************************************************/



ListNode<String> * SplitLine (String Text, Rect &Bounds, unsigned MinLen = 0);
// Split the given string into a list of lines. MinLen is used if any of the
// lines is requested to be centered. In this case, MinLen is the minimum
// length of the centered line (used in windows with header strings).

void ReleaseLines (ListNode<String> *Node);
// Release a line list build from SplitLine



// End of SPLITMSG.H

#endif

