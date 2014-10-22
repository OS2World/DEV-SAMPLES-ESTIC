/*****************************************************************************/
/*                                                                           */
/*                                   SKEL.CC                                 */
/*                                                                           */
/* (C) 1996     Ullrich von Bassewitz                                        */
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



#include "program.h"
#include "progutil.h"
#include "menuitem.h"
#include "menue.h"
#include "stdmenue.h"
#include "stdmsg.h"

#include "skelmsg.h"
#include "skel.h"



/*****************************************************************************/
/*                            Message constants                              */
/*****************************************************************************/



// These are constants for messages loaded from the resource file.
// The MSGBASE_XXX constants are defined in skelmsg.h. Usually I do
// reserve a range of 100 messages for each source file, so the step
// for the MSGBASE_XXX constants is 100. But this is just a convention.

const u16 msSkel                        = MSGBASE_SKEL + 0;
const u16 msAbout                       = MSGBASE_SKEL + 1;
const u16 msQuit                        = MSGBASE_SKEL + 2;
const u16 msAboutInfo                   = MSGBASE_SKEL + 3;



/*****************************************************************************/
/*                               class SkelApp                               */
/*****************************************************************************/



SkelApp::SkelApp (int argc, char* argv []):
    Program (argc, argv, CreateMenueBar, CreateStatusLine, "skel")
{
}



// The following code builds the main menu and the statusline at the bottom.
// The somewhat weird syntax is the result of my plan, to incorporate menu
// bars into the resource editor, so one would load (instead construct at
// runtime) the top menu bar. I thought about overloading the '+' key for
// menu items, but that lead to problems with submenues.



static MenueItem* GetMenueItem (u16 MsgID, i16 ItemID, Key AccelKey, WindowItem* NextItem)
{
    return (MenueItem*) SetAccelKey (new MenueItem (App->LoadAppMsg (MsgID),
                                                    ItemID,
                                                    NextItem),
                                     AccelKey);
}



static MenueLine* GetMenueLine (WindowItem* NextItem)
{
    return new MenueLine (miNone, NextItem);
}



static MenueBarItem* GetMenueBarItem (u16 MsgID, i16 ItemID,
                                      WindowItem* MenueList,
                                      WindowItem* NextItem)
{
    return new MenueBarItem (App->LoadAppMsg (MsgID), ItemID, MenueList, NextItem);
}



TopMenueBar* SkelApp::CreateMenueBar ()
{
    TopMenueBar* M = new TopMenueBar (
      GetMenueBarItem   (msSkel,            miSkel,
        GetMenueItem    (msAbout,           miAbout,            kbNoKey,
        GetMenueLine    (
        GetMenueItem    (msQuit,            miQuit,             vkQuit,
        NULL
      ))),
      NULL
    ));

    // Register the accel keys of the submenues
    App->RegisterKey (M->GetAccelKey (miSkel));

    // Register the accel keys
    App->RegisterKey (M->GetAccelKey (miQuit));

    // Return the result
    return M;
}



BottomStatusLine* SkelApp::CreateStatusLine ()
{
    return new BottomStatusLine (siExit);
}



int SkelApp::Run ()
// Run the application
{
    // These are the items for the quit menu
    const i16 miNo  = 1;
    const i16 miYes = 2;

    // Activate the main menue
    MainMenue->Activate ();

    // Main loop
    while (!Quitting ()) {

        // Switch according to the users choice
        switch (MainMenue->GetChoice ()) {

            case miAbout:
                InformationMsg (LoadAppMsg (msAboutInfo));
                break;

            case miQuit:
                // Close the windows
                if (MenueChoice ("@SKEL.QuitMenu") == miYes) {

                    // The end...
                    Quit = 1;

                }
                break;

        }

    }

    // Return the program exit code
    return 0;
}




int main (int argc, char* argv [])
{
    // Set the default language and country
//  DefaultLanguage = laGerman;
//  DefaultCountry = 49;

    // Declare an application object
    SkelApp MyApp (argc, argv);

    // Use it...
    return MyApp.Run ();
}




