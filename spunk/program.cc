/*****************************************************************************/
/*                                                                           */
/*                                  PROGRAM.CC                               */
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



#if defined(DOS) || defined(DOS32) || defined(OS2) || defined(NT) || defined(NETWARE)
#  include <io.h>
#  include <dos.h>
#  include <fcntl.h>
#  include <process.h>
#else
#  include <unistd.h>
#endif
#include <signal.h>

#include "msgid.h"
#include "eventid.h"
#include "filesys.h"
#include "kbd.h"
#include "screen.h"
#include "winattr.h"
#include "palette.h"
#include "environ.h"
#include "program.h"
#include "national.h"
#include "stdmsg.h"
#include "filepath.h"
#include "wildargs.h"
#include "progutil.h"



/*****************************************************************************/
/*                             Message constants                             */
/*****************************************************************************/



const u16 msNoHelpAvailable             = MSGBASE_PROGRAM +  0;
const u16 msNoHelpOnThisTopic           = MSGBASE_PROGRAM +  1;



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



// Global visible instance of class Program
Program* App;



/*****************************************************************************/
/*                               class Program                               */
/*****************************************************************************/



Program::Program (int argc, char** argv,
                  TopMenueBar* (*GetMenueBar) (),
                  BottomStatusLine* (*GetStatusLine) (),
                  const String& ProgBaseName):
    MainResource (NULL),
    MsgBase (NULL),
    AppMsgBase (NULL),
    ProgramName (ProgBaseName),
#ifdef NETWARE
    PID (0),
#else
    PID (getpid ()),
#endif
    GotSigWinCh (0),
    LastIdleTime (Now ()),
    StatusLine (NULL),
    MainMenue (NULL),
    ArgCount (argc),
    ArgVec (argv)
{
    // Initialize App pointer
    App = this;

    // Actions to be taken if we have an OS/2 or DOS system:
#if defined(DOS) || defined(DOS32) || defined(OS2) || defined(NETWARE)
    // Explode the argv vector
    ExpandArgs (ArgCount, ArgVec);

    // Use binary mode as default open mode for files
    _fmode = O_BINARY;
#endif

    // Define the search path for the resource file
#if defined(OS2)
    String SearchPath = GetEnvVar ("DPATH");
#else
    String SearchPath = GetEnvVar ("PATH");
#endif

    // Add the path of the executable to the search path
    if (ArgVec) {
        String Path, Name;
        FSplit (ArgVec [0], Path, Name);
        if (Path.Len () > 0) {
            Path = CleanPath (Path);    // Make the path absolute
            DelPathSep (Path);          // Remove the trailing path separator
            Path += FileSysListSep;     // Add a file list separator
            SearchPath = Path + SearchPath;
        }
    }

    // Get the current directory and remove the trailing separator
    String CurrentDir = GetCurrentDir ();
    DelPathSep (CurrentDir);

    // Expand the search path with the current directory
    SearchPath = CurrentDir + FileSysListSep + SearchPath;

    // Set up the support path if it is fixed by giving an environment
    // variable.
    // The environment variable is created from the program basename in
    // capital letters with "PATH" added.
    String EnvPath = ProgBaseName;
    EnvPath.ToUpper ();
    EnvPath += "PATH";
    SupportPath = GetEnvVar (EnvPath);
    if (!SupportPath.IsEmpty ()) {

        // An environment variable has been defined. Use this path as an
        // additional search path for the resource file
        SearchPath = SupportPath + FileSysListSep + SearchPath;
    }

#ifdef NETWARE
    SearchPath += "sys:system;";                // ##
#endif

    // Check if the resource file exists
    String ResName = ProgBaseName + ".res";
    String ResDir = FSearch (SearchPath, ResName, R_OK);

    // If the resource file exists, open the resource for reading only. If
    // there is no support path defined (by the environment variable), use the
    // path of the resource file as support path.
    if (!ResDir.IsEmpty ()) {

        // Try to open the file
        MainResource = new ResourceFile (new FileStream (ResDir + ResName, "rb"));
        if (MainResource->GetStatus () != reOk) {
            // Cannot use ErrorMsg here
            FAIL ("Cannot open resource file");
        }

        // Ok, resource file is open. Eventually use the path as support path
        if (SupportPath.IsEmpty ()) {
            SupportPath = ResDir;
        }

    } else {
        String Msg = FormatStr ("Error loading resource file %s", ResName.GetStr ());
        FAIL (Msg.GetStr ());
    }

    // The support path is no definitely non empty. Add a path separator to
    // the support path
    AddPathSep (SupportPath);

    // Search for the help file, but don't open it
    HelpFileName = SupportPath + ProgBaseName + ".hlp";
    if (!FExists (HelpFileName, R_OK)) {
        // File does not exist or is not accessible, clear the name
        HelpFileName.Clear ();
    }

    // Initialize the window attributes
    InitWinAttr ();

    // Initialize the national language system
    NLSInit ();

    // Create an instance of class Screen to handle screen output
    // Beware: This must be done before creating the keyboard object,
    // because in Linux, both interact, and Kbd expects Screen to read
    // in the termcap entry.
    TheScreen = new Screen;

    // Create an instance of class Keyboard to handle keyboard input
    Kbd = new Keyboard;

    // Create the root window
    (void) new RootWindow;

    // Ok, screen and kbd initialization is complete, grab and replace the
    // fail vector. Do use an intermediate function for that, since the
    // menue and statusline are not initialized - this will result in a
    // crash if some resources are not found and high level functions of
    // spunk are called.
    OldFailVec = CheckFailed;
    CheckFailed = _InitCheckFailed;

    // Before calling user supplied functions, make shure the programs
    // message base is loaded
    if (MsgBase == 0) {
        MsgBase = (MsgCollection*) LoadResource ("PROGRAM.Messages");
    }

    // Now create the menue bar and the status line
    if (GetMenueBar != NULL) {
        MainMenue = GetMenueBar ();
        if (MainMenue != NULL) {
            MainMenue->Show ();
        }
    }
    if (GetStatusLine != NULL) {
        StatusLine = GetStatusLine ();
    }

    // Ok, done. Now use the right CheckFailed function
    CheckFailed = _CheckFailed;
}



Program::~Program ()
{
    // Clean up screen and other stuff
    Cleanup ();
}



void Program::Cleanup ()
// Clean up in the destructor or in a emergeny situation
{
    // Reset the _CheckFailed vector to catch errors in the shutdown phase
    CheckFailed = OldFailVec;

    // Delete program objects
    delete MainResource;        MainResource = NULL;
    delete StatusLine;          StatusLine = NULL;
    delete MainMenue;           MainMenue = NULL;
    delete Background;          Background = NULL;
    delete TheScreen;           TheScreen = NULL;
    delete Kbd;                 Kbd = NULL;

    // We do not need the message bases any longer
    FreeMsgBase ();
    FreeAppMsgBase ();

    // Free the window attributes
    DoneWinAttr ();
}



Streamable* Program::LoadResource (const String& Name, int MustHave)
// Load a resource from the resource file. If MustHave is true, this function
// never returns a NULL pointer, it will abort the program if the needed
// resource is not found
{
    // Check if a resource exists
    if (MainResource == NULL) {
        if (MustHave) {
            FAIL ("LoadResource called but resource file doesn't exist");
        } else {
            return NULL;
        }
    }

    // Try to find the language specific version of the resource. Build the
    // name of the language specific version
    String NewKey = FormatStr ("%03d.%s", NLSLanguage, Name.GetStr ());

    // First try to find the language specific version
    int Index;
    if ((Index = MainResource->FindKey (NewKey)) == -1) {

        // Not found. Now try the generic version
        if ((Index = MainResource->FindKey (Name)) == -1) {

            // Last resort: Try to find an english version
            NewKey = FormatStr ("%03d.%s", laEnglish, Name.GetStr ());
            Index = MainResource->FindKey (NewKey);

        }

    }

    // Load the resource if the key could be found
    Streamable* R = (Index != -1)? MainResource->Get (Index) : 0;

    // Check if we could load the resource
    if (R == NULL) {

        // Get a pointer to the resource stream
        const Stream& S = MainResource->GetStream ();

        //
        if (MustHave) {

            // Check the cause of the error
            switch (S.GetStatus ()) {

                case stGetError:
                    // ID not found
                    FAIL (FormatStr ("Resource %s: ID %d not registered",
                          Name.GetStr (), S.GetErrorInfo ()).GetStr ());

                default:
                    // Cannot use ErrorMsg here!
                    String Msg (String ("Missing resource: ") + Name);
                    FAIL (Msg.GetStr ());

            }

        } else {

            // Not found, check the stream status. If the stream status is ok
            // the resource was not found
            if (S.GetStatus () != stOk) {
                String Msg = FormatStr ("Resource %s: Stream error, status = %d",
                      Name.GetStr (), S.GetStatus ());
                FAIL (Msg.GetStr ());
            }

        }
    }

    // Return a pointer to the resource or NULL
    return R;
}



String Program::AppMsgBaseName ()
// This function builds the name for the application message resource.
// The default is to use ProgramName in uppercase, preceeded by a '@'
// and with '.Messages' added (e.g. "@RESED.Messages").
{
    String ResName = '@' + ProgramName;
    ResName.ToUpper ();
    return ResName + ".Messages";
}



const Msg& Program::LoadMsg (u16 MsgNum)
// Load a message from the library message base. Application functions
// should use LoadAppMsg instead. If the MsgBase is not already loaded,
// this function will load it.
{
    if (MsgBase == NULL) {
        // MsgBase not loaded, load it now
        MsgBase = (MsgCollection*) LoadResource ("PROGRAM.Messages");
    }

    // Retrieve the message
    return MsgBase->GetMsg (MsgNum);
}



const Msg& Program::LoadAppMsg (u16 MsgNum)
// Load a message from the application message base. Library functions
// should use LoadMsg instead. If AppMsgBase is not already loaded,
// this function will load it.
{
    if (AppMsgBase == NULL) {
        // AppMsgBase not loaded, load it now
        AppMsgBase = (MsgCollection*) LoadResource (AppMsgBaseName ());
    }

    // Retrieve the message
    return AppMsgBase->GetMsg (MsgNum);
}



void Program::FreeMsgBase ()
// The library message base will be deleted. This is useful if you are
// temporary short on memory. The next call to LoadMsg will reload the
// MsgBase, so repeatedly calling this function will slow down the
// program. Beware: After calling this function, references to messages
// are no longer valid!

{
    delete MsgBase;
    MsgBase = NULL;
}



void Program::FreeAppMsgBase ()
// The application message base will be deleted. This is useful if you are
// temporary short on memory. The next call to LoadAppMsg will reload the
// AppMsgBase, so repeatedly calling this function will slow down the
// program. Beware: After calling this function, references to messages
// are no longer valid!

{
    delete AppMsgBase;
    AppMsgBase = NULL;
}



void Program::_InitCheckFailed (const char* Msg, const char* Cond,
                                int Code, const char* File, int Line)
// Temporary function that is installed while initializing. It avoids
// calling other high level spunk functions since they may not be
// initialized.
{
    // Beware: To avoid an endless loop, reset the fail vector!
    CheckFailed = App->OldFailVec;

    // Post an appropriate event
    ::PostEvent (evAbort);

    // Use cleanup to clear the screen etc.
    App->Cleanup ();

    // Now call the old fail vector - this will print a message on stderr
    // and end the program
    CheckFailed (Msg, Cond, Code, File, Line);

    // Safety: Do not return!
    exit (EXIT_FAILURE);
}



void Program::_CheckFailed (const char* Msg, const char* Cond,
                            int Code, const char* File, int Line)
// Function that is called if a fatal error occurs. The function simply
// calls the virtual function AppError, which can be overridden by derived
// classes. After returning from CheckFailed, the screen is cleared and
// the program is aborted.
{
    // Beware: To avoid an endless loop, reset the fail vector!
    CheckFailed = App->OldFailVec;

    // Post an appropriate event
    ::PostEvent (evAbort);

    // Call the virtual function
    App->AppError (Msg, Cond, Code, File, Line);

    // Use cleanup to clear the screen etc
    App->Cleanup ();

    // Use exit instead of abort. This cleans up any global instances
    exit (EXIT_FAILURE);
}



void Program::AppError (const char* Msg, const char* Cond,
                        int Code, const char* File, int Line)
// Function that is called from _CheckFailed if a fatal error occurs.
{
    // Create the string
    String S = FormatStr ("%s%s (= %d)\nfile %s\nline %d", Msg, Cond, Code, File, Line);

    // place a msg window
    FatalErrorMsg (S);
}



void Program::CallHelp (const String& /*HelpTopic*/)
// Call the help for a specific topic
{
    if (!HasHelp ()) {
        ErrorMsg (msNoHelpAvailable);
    } else {
        // Temporary - this will hopefully change...
        ErrorMsg (msNoHelpOnThisTopic);
    }
}



int Program::SigUsr3 ()
// Handle the SIGUSR3 signal. This is used as a replacement for
// SIGWINCH under NT. The function will do nothing if not running
// under NT.
{
#ifdef NT
    // Set the global flag
    GotSigWinCh = 1;

    // Signal has been handled...
    return 1;
#else
    return 0;
#endif
}



int Program::SigWinCh ()
{
    // Set the global flag
    GotSigWinCh = 1;

    // Signal has been handled...
    return 1;
}



void Program::Idle ()
// This is the idle function of the application class. Default is to
// do some housekeeping. This function may be called by anyone
// at anytime but it's not guaranteed to be called regularly.
{
    // Handle the SIGWINCH signal under linux
    if (GotSigWinCh) {

        // Reset the flag
        GotSigWinCh = 0;

        // Change the mode.
        TheScreen->SetMode (vmAsk);

        // Now tell all windows that the video resolution has changed
        Event* E = new Event (evScreenSizeChange);
        E->Info.O = new Rect (TheScreen->GetSize ());
        PostEvent (E);

    }

    // Get the system time and check if the time has changed
    Time Current = Now ();
    u32 Sec = Current.GetSec ();
    u32 LastSec = LastIdleTime.GetSec ();

    // Remember the last update time. Be shure to do this before delivering
    // evSecondChange and evMinuteChange events in case this function is
    // called recursively.
    LastIdleTime = Current;

    // Post an idle event
    Event* E = new Event (evIdle);
    E->Info.O = new Time (Current);
    PostEvent (E);

    // If the seconds have changed post another event
    if (Sec != LastSec) {

        E = new Event (evSecondChange);
        E->Info.O = new Time (Current);
        PostEvent (E);

        // If the minutes have changed, post a third event
        if ((Sec % 60) == 0) {

            E = new Event (evMinuteChange);
            E->Info.O = new Time (Current);
            PostEvent (E);

        }

    }
}



void Program::ChangeVideoMode (u16 NewMode)
// Change video mode to NewMode. Use the constants from screen.h
{
    // Change only if mode is different to current mode
    if (NewMode != TheScreen->GetMode ()) {

        // Change the mode
        TheScreen->SetMode (NewMode);

        // Now tell all windows that the video resolution has changed
        Rect NewSize (0, 0, TheScreen->GetXSize (), TheScreen->GetYSize ());
        Background->ChangeScreenSize (NewSize);

    }
}



void Program::RedrawScreen ()
// Redraw the complete screen in case it's garbled
{
    Background->RedrawScreen ();
}



void Program::PostEvent (Event* E, int ImmediateDelivery)
// Post an event to the event queue, then call DeliverEvents.
{
    // Put the event into the queue
    EvQueue.Put (E, 0);

    // Deliver events from the queue if requested
    if (ImmediateDelivery) {
        DeliverEvents ();
    }
}



void Program::DeliverEvents ()
// Deliver all events currently in the event queue.
{
    // Deliver until queue is empty
    while (EvQueue.IsEmpty () == 0) {

        // Retrieve the next event from the queue
        Event* E = EvQueue.Get ();

        // Deliver this event, then delete it
        SendEvent (E);

    }
}



void Program::SendEvent (Event* E)
// Send the event to all event handlers without going over the event
// queue. E is deleted after delivery. Use with care!
{
    // Prepare the event
    E->Handled = 0;

    // Get the first node
    ListNode<EventHandler>* Node = EventHandler::EventHandlerList;

    if (Node) {

        // Now loop through all event handlers until the root is reached again
        do {

            // Deliver the event
            Node->Contents () -> HandleEvent (*E);

            // Get the next node
            Node = Node->Next ();

        } while (Node != EventHandler::EventHandlerList && E->Handled == 0);

    }

    // Event has been delivered (or tried to), delete it
    delete E;
}



void Program::RemoveArg (int Index)
// Remove the program argument with the given index. The function will
// call FAIL if the index is invalid.
{
    // Assure that the index is valid
    PRECONDITION (Index >= 0 && Index < ArgCount);

    // Copy the other args down
    while (++Index <= ArgCount) {       // Include the NULL vector
        ArgVec [Index-1] = ArgVec [Index];
    }

    // We now have one argument less
    ArgCount--;
}



