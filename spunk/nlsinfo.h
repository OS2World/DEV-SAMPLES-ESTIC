/*****************************************************************************/
/*                                                                           */
/*                                 NLSINFO.H                                 */
/*                                                                           */
/* (C) 1995     Ullrich von Bassewitz                                        */
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



// Information in this file is used internal by the library.



#ifndef _NLSINFO_H
#define _NLSINFO_H



#include "national.h"



/*****************************************************************************/
/*                              struct _NLSInfo                              */
/*****************************************************************************/



struct _NLSInfo {
    _NLSData            Data;
    _NLSTransTable      UpCaseMap;
    _NLSTransTable      LoCaseMap;
    _NLSTransTable      CollMap;
};



// End of NLSINFO.H

#endif

