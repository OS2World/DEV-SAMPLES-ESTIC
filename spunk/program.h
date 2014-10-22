/*****************************************************************************/
/*                                                                           */
/*                                  PROGRAM.H                                */
/*                                                                           */
/* (C) 1993-96  Ullrich von Bassewitz                                        */
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



#ifndef _PROGRAM_H
#define _PROGRAM_H



#include "statline.h"
#include "event.h"
#include "thread.h"
#include "menue.h"
#include "msgcoll.h"
#include "resource.h"
#include "datetime.h"



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



// Instance of class Program
extern class Program *App;



/*****************************************************************************/
/*                               class Program                               */
/*****************************************************************************/



class Program: public EventHandler, public Thread {

private:
    void (*OldFailVec) (const char*, const char*, int, const char*, int);
    // Old fail vector

    static void _CheckFailed (const char* Msg, const char* Cond,
                              int Code, const char* File, int Line);
    // Function that is called if a fatal error occurs. The function simply
    // calls the virtual function AppError, which can be overridden by derived
    // classes. After returning from CheckFailed, the screen is cleared and
    // the program is aborted.

    static void _InitCheckFailed (const char* Msg, const char* Cond,
                                  int Code, const char* File, int Line);
    // Temporary function that is installed while initializing. It avoids
    // calling other high level spunk functions since they may not be
    // initialized.

protected:
    ResourceFile*       MainResource;
    MsgCollection*      MsgBase;                // Messages for the library
    MsgCollection*      AppMsgBase;             // Messages for the application
    String              ProgramName;            // Base name for the program
    String              SupportPath;            // Path for support files
    String              HelpFileName;           // Name+path of the help file
    unsigned            PID;                    // Process ID
    int                 GotSigWinCh;            // true if a SIGWINCH occured
    EventQueue          EvQueue;                // EventQueue
    Time                LastIdleTime;           // Time of last update


    virtual int SigUsr3 ();
    // Handle the SIGUSR3 signal. This is used as a replacement for
    // SIGWINCH under NT. The function will do nothing if not running
    // under NT.

    virtual int SigWinCh ();
    // Handle the SIGWINCH signal (Unices only)

    virtual void Cleanup ();
    // Clean up in the destructor or in a emergeny situation

    virtual void AppError (const char* Msg, const char* Cond,
                           int Code, const char* File, int Line);
    // Function that is called in case of errors. The function is usually
    // called from _CheckFailed. It should not be called directly because
    // it does no cleanup and simply returns to the caller after displaying
    // an error message.

    virtual String AppMsgBaseName ();
    // This function builds the name for the application message resource.
    // The default is to use ProgramName in uppercase, preceeded by a '@'
    // and with '.Messages' added (e.g. "@RESED.Messages").

    virtual void DeliverEvents ();
    // Deliver all events currently in the event queue.

    virtual void SendEvent (Event* E);
    // Send the event to all event handlers without going over the event
    // queue. E is deleted after delivery. Use with care!


public:
    // Public accessible variables
    BottomStatusLine*   StatusLine;
    TopMenueBar*        MainMenue;
    int                 ArgCount;               // Expanded argc from main()
    char**              ArgVec;                 // Expanded argv from main()

public:
    Program (int argc, char** argv,
             TopMenueBar* (*GetMenueBar) (),
             BottomStatusLine* (*GetStatusLine) (),
             const String& ProgBaseName);
    // Create a program object. argc/argv are the arguments from main. On
    // OS/2 and DOS systems, they will be expanded, on all systems they end up
    // in the global accessible variables ArgCount and ArgVec.
    // GetMenueBar ist a function pointer that is called to create the menue
    // bar. GetStatusLine is a function pointer that is called to create the
    // status line. Both may be NULL if no menue bar and/or status line is
    // needed. Beware: Many functions of the library assume that a statusline
    // exists!
    // ProgBaseName is the base name of the program. It is used as basename
    // for resource and help files and for window titles (if needed).

    virtual ~Program ();
    // Destruct a program object

    Streamable* LoadResource (const String& Name, int MustHave = 1);
    // Load a program resource from the resource file. The usual language
    // resolving algorithm is applied. If MustHave is set to 1, the function
    // will abort the program (via FAIL) if the resource is not available

    virtual const Msg& LoadMsg (u16 MsgNum);
    // Load a message from the library message base. Application functions
    // should use LoadAppMsg instead. If the MsgBase is not already loaded,
    // this function will load it.

    virtual const Msg& LoadAppMsg (u16 MsgNum);
    // Load a message from the application message base. Library functions
    // should use LoadMsg instead. If AppMsgBase is not already loaded,
    // this function will load it.

    virtual void FreeMsgBase ();
    // The library message base will be deleted. This is useful if you are
    // temporary short on memory. The next call to LoadMsg will reload the
    // MsgBase, so repeatedly calling this function will slow down the
    // program. Beware: After calling this function, references to messages
    // are no longer valid!

    virtual void FreeAppMsgBase ();
    // The application message base will be deleted. This is useful if you are
    // temporary short on memory. The next call to LoadAppMsg will reload the
    // AppMsgBase, so repeatedly calling this function will slow down the
    // program. Beware: After calling this function, references to messages
    // are no longer valid!

    virtual void Idle ();
    // This is the idle function of the application class. Default is to
    // do some housekeeping. This function may be called by anyone
    // at anytime but it's not guaranteed to be called regularly.

    void ChangeVideoMode (u16 NewMode);
    // Change video mode to NewMode. Use the constants from screen.h

    void RedrawScreen ();
    // Redraw the complete screen in case it's garbled

    unsigned GetPID () const;
    // Get process ID

    const String& GetSupportPath () const;
    // Access the protected variable

    const String& GetProgName () const;
    // Return the program base name

    int HasHelp () const;
    // Return true if we have help available. This is assumed to be true if
    // the help file exists and is readable.

    void CallHelp (const String& HelpTopic);
    // Call the help with the specified topic

    virtual void PostEvent (Event* E, int ImmediateDelivery = 1);
    // Post an event to the event queue, eventually call DeliverEvents.
    // The event will be deleted after successful delivery.

    void RemoveArg (int Index);
    // Remove the program argument with the given index. The function will
    // call FAIL if the index is invalid.

};



inline unsigned Program::GetPID () const
{
    return PID;
}



inline const String& Program::GetSupportPath () const
// Access the protected variable
{
    return SupportPath;
}



inline const String& Program::GetProgName () const
// Access the protected variable
{
    return ProgramName;
}



inline int Program::HasHelp () const
// Return true if we have help available. This is assumed to be true if the
// help file exists and is readable.
{
    return HelpFileName.IsEmpty () == 0;
}



// End of PROGRAM.H

#endif


