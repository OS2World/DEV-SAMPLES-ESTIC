/*****************************************************************************/
/*                                                                           */
/*                                  SKEL.H                                   */
/*                                                                           */
/* (C) 1995-96  Ullrich von Bassewitz                                        */
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



#ifndef _SKEL_H
#define _SKEL_H



#include "program.h"



/*****************************************************************************/
/*                               Menue items                                 */
/*****************************************************************************/



const i16 miNone                        =    1;

const i16 miSkel                        = 1000;
const i16 miAbout                       = 1100;
const i16 miQuit                        = 1200;



/*****************************************************************************/
/*                               class SkelApp                               */
/*****************************************************************************/



class SkelApp: public Program {


private:
    static TopMenueBar* CreateMenueBar ();
    static BottomStatusLine* CreateStatusLine ();

public:
    SkelApp (int argc, char* argv []);
    // Construct an application object

    virtual int Run ();
    // Run the application

};



// End of SKEL.H

#endif
