/*****************************************************************************/
/*									     */
/*				     ESTIC.CC				     */
/*									     */
/* (C) 1995-97	Ullrich von Bassewitz					     */
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



#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "eventid.h"
#include "delay.h"
#include "screen.h"
#include "environ.h"
#include "syserror.h"
#include "filepath.h"
#include "filesel.h"
#include "fviewer.h"
#include "program.h"
#include "progutil.h"
#include "menuitem.h"
#include "menue.h"
#include "strcvt.h"
#include "stdmenue.h"
#include "stdmsg.h"
#include "memstrm.h"
#include "datetime.h"
#include "inifile.h"
#include "settings.h"
#include "winmgr.h"

#include "icmsg.h"
#include "icevents.h"
#include "icerror.h"
#include "icconfig.h"
#include "icdlog.h"
#include "iccom.h"
#include "iclog.h"
#include "icalias.h"
#include "iccron.h"
#include "icfile.h"
#include "devstate.h"
#include "icdevs.h"
#include "icident.h"
#include "icbaseed.h"
#include "icdiag.h"
#include "callwin.h"
#include "icac.h"
#include "iccli.h"
#include "icshort.h"
#include "cliwin.h"
#include "chargwin.h"
#ifdef LINUX
#include "imon.h"
#endif
#include "estic.h"



/*****************************************************************************/
/*				   Constants				     */
/*****************************************************************************/



// Diag mode update
static const duOff		= 0;
static const duOn		= 1;
static const duAuto		= 2;	// Update if version <= 1.93

static const char VersionStr [] = "1.50";
static const char VersionID []	= "ESTIC-Version";



/*****************************************************************************/
/*			      Message constants				     */
/*****************************************************************************/



const u16 msAboutInfo			= MSGBASE_ISTEC + 60;
const u16 msConnMehrgeraete		= MSGBASE_ISTEC + 61;
const u16 msConnAnlagen			= MSGBASE_ISTEC + 62;
const u16 msConnUnknown			= MSGBASE_ISTEC + 63;
const u16 msIstecConfig			= MSGBASE_ISTEC + 64;
const u16 msLoadFileHeader		= MSGBASE_ISTEC + 65;
const u16 msSaveFileHeader		= MSGBASE_ISTEC + 66;
const u16 msComPortNotOpen		= MSGBASE_ISTEC + 67;
const u16 msIstecTimeout		= MSGBASE_ISTEC + 68;
const u16 msWriteChanges		= MSGBASE_ISTEC + 69;
const u16 msPrintHeader			= MSGBASE_ISTEC + 71;
const u16 msPrintHeader2		= MSGBASE_ISTEC + 72;
const u16 msChargeTableHeader		= MSGBASE_ISTEC + 73;
const u16 msChargeLine			= MSGBASE_ISTEC + 74;
const u16 msErrorComPortOpen		= MSGBASE_ISTEC + 75;
const u16 msRecBufOverflow		= MSGBASE_ISTEC + 76;
const u16 msRecBufUnderflow		= MSGBASE_ISTEC + 77;
const u16 msInvalidReply		= MSGBASE_ISTEC + 78;
const u16 msWrongDevice			= MSGBASE_ISTEC + 79;
const u16 msPrintFileSelHeader		= MSGBASE_ISTEC + 80;
const u16 msViewLogSelHeader		= MSGBASE_ISTEC + 81;
const u16 msSettingsFileError		= MSGBASE_ISTEC + 82;
const u16 msSettingsVersionError	= MSGBASE_ISTEC + 83;
const u16 msReadCronFile		= MSGBASE_ISTEC + 84;
const u16 msWriteShortNumbers		= MSGBASE_ISTEC + 85;



/*****************************************************************************/
/*			       class IstecApp				     */
/*****************************************************************************/



IstecApp::IstecApp (int argc, char* argv []):
    Program (argc, argv, CreateMenueBar, CreateStatusLine, "estic"),
    StatusFlags (0),
    ComPortName ("COM2"),
    SettingsFile ("estic.rc"),
    IstecPresent (0),
    Changes (0),
    ShortNumberChanges (0),
    Currency (NLSData.CurrStr),
    VideoMode (vmAsk),
    ShowDateTime (1),
    ShowInfoOnStartup (1),
    DiagModeUpdate (duAuto),
    LastUpdate (Now ()),
    DiagModeUpdateCounter (450),		// 7.5 min
    ChargesUpdateCounter (900)			// 15 min
{
    // Assume that we don't have an Istec
    int HaveIstec = 1;

    // Read the default values of some variables from the ini file
    ReadIniFile ();

    // Parse the command line
    int I = 1;
    while (I < ArgCount) {

	char* Item = ArgVec [I];
	if (*Item == '-') {

	    Item++;
	    switch (*Item) {

		case 'a':
		    PortBase = atoi (++Item);
		    break;

		case 'i':
		    PortIRQ = atoi (++Item);
		    break;

		case 'n':
		    HaveIstec = 0;
		    break;

		case 'p':
		    ComPortName = ++Item;
		    break;

	    }

	}

	// Next argument
	I++;
    }

    // Now switch the video mode
    if (VideoMode != vmAsk) {
	ChangeVideoMode (VideoMode);
    }

    // Display date and Time in the upper right corner of the main menu
    if (ShowDateTime > 0) {
	DisplayDateTime (Now ());
    }

    // Try to open the debug logfile
    InitDebugLog (DebugLog);

    // Open the program option file
    if (!SettingsFile.IsEmpty ()) {
	// If we cannot open the settings file, print an error message.
	// If we could open the file, check for the version string to
	// make shure, the guys out there don't use old versions of the
	// file.
	if (StgOpen (MakeAbsolute (SettingsFile)) != 0) {
	    IstecError (msSettingsFileError);
	} else {
	    if (!StgEmpty ()) {
		String Version = StgGetString (VersionID, "");
		if (Version != VersionStr) {
		    // Incorrect version, close the file (don't use it)
		    IstecError (msSettingsVersionError);
		    StgClose ();
		}
	    } else {
		// This is a new resource - write the version string
		StgPutString (VersionStr, VersionID);
	    }
	}
    }

    // Try to initialize the com port if this was not prohibited
    if (HaveIstec) {

	// Try to open the com port
	if (OpenComPort (ComPortName) != 0) {
	    // Port could not be opened
	    IstecError (msErrorComPortOpen);
	} else {
	    // Com port could be opened, check for the istec
	    if (EvalCmd (IstecReady ())) {
		// Istec is there
		IstecPresent = 1;
	    }

	}
    }

    // If the istec is online, read the cron file
    ReadCronFile ();

    // Read the complete configuration from the istec if possible, else reset
    // it to a known state
    InitIstecConfig ();

    // Load the window manager from the settings file
    WinMgr = (WindowManager*) StgGet ("WinMgr");
    if (WinMgr == NULL) {
	// Does not exist, create default
	WinMgr = new WindowManager;
    }

    // If the istec is not present, disable some of the menue choices, if
    // the istec is present, switch to debug mode if requested
    if (IstecPresent) {

	// Enable the istec diagnostic messages
	IstecDiagOn ();

	// If we have a firmware version < 2.00, disable the short number
	// access
	if (FirmwareVersion < 2.00) {
	    DisableCommand (miFileLoadShort);
	    DisableCommand (miFileSaveShort);
	    DisableCommand (miIstecLoadShort);
	    DisableCommand (miIstecSaveShort);
	    DisableCommand (miEditShort);
	}

	// Set the country code string for the areacode resolver from the
	// Istec setting if it is not defined until now.
	if (CountryCode.IsEmpty ()) {
	    CountryCode = U32Str (Config.GetCountryCode ());
	}

    } else {
	// Disable some commands
	DisableCommand (miIstecLoadConfig);
	DisableCommand (miIstecSaveConfig);
	DisableCommand (miIstecLoadShort);
	DisableCommand (miIstecSaveShort);
	DisableCommand (miReadCronFile);
	DisableCommand (miMatrixWin);
	DisableCommand (miCallWin);
	DisableCommand (miCLIWin);
	DisableCommand (miCharges);
	DisableCommand (miChargeWin2);
    }

    // Ok, initialization is complete, post an apropriate event
    ::PostEvent (evInit);

    // Display the istec configuration
    if (ShowInfoOnStartup) {
	ShowIstecConfig ();
    }

}



IstecApp::~IstecApp ()
// Destruct an application object
{
    // Post an event before shutting down
    ::PostEvent (evExit);

    // Close the com port
    CloseComPort ();

    // Delete the window manager
    delete WinMgr;

    // Close the settings file
    StgClose ();
}



void IstecApp::BackgroundWork (const Time& Current)
// Idle function. Is used to check for debug messages in the receive queue.
// This function contains some hacks. No spunk program should rely on calls
// to App::Idle in a regular or even time based manner. But there is no
// other way to implement the needed background functions without using
// too much CPU resources. So I will do something dirty and use my knowledge
// about the internals of the KbdGet() and Delay() functions here...
// However, there are other solutions, but none of them is portable between
// the supported operating systems.
{
    // Get the system time and check if the time has changed
    TimeDiff Period = Current - LastUpdate;

    // Remember the last update time
    LastUpdate = Current;

    // Check if we have to talk with the istec
    if (IstecPresent) {

	// Check if must update the diag mode
	if (DiagModeUpdate == duOn ||
	    (DiagModeUpdate == duAuto && FirmwareVersion <= 1.93)) {
	    if ((DiagModeUpdateCounter -= Period.GetSec ()) <= 0) {
		DiagModeUpdateCounter += 15 * 60;
		IstecDiagOn ();
	    }
	}

	// Check if we must request the charges
	if ((ChargesUpdateCounter -= Period.GetSec ()) <= 0) {
	    ChargesUpdateCounter += 15 * 60;
	    IstecRequestCharges ();
	}

    }

}



void IstecApp::CronHandler (const Time& T)
// Is called from idle every minute, checks periodic events
{
    HandleCronEvent (T);
}



void IstecApp::DisplayDateTime (const Time& T)
// Display the time in the upper right corner
{
    String S = T.DateTimeStr (ShowDateTime > 1);
    MainMenue->Write (MainMenue->MaxX () - S.Len (), 0, S);
}



void IstecApp::HandleEvent (Event& E)
// Handle incoming events. Calls Update() if the application is idle
{
    // Call the derived function and return if the event is handled
    Program::HandleEvent (E);
    if (E.Handled) {
	return;
    }

    // Now look at the event code
    switch (E.What) {

	case evIdle:
	    // Poll the istec
	    IstecPoll ();
	    break;

	case evSecondChange:
	    // Update date&time
	    if (ShowDateTime == 2) {
		DisplayDateTime (*(Time*)E.Info.O);
	    }
	    // Do some background work
	    BackgroundWork (*(Time*)E.Info.O);
	    break;

	case evMinuteChange:
	    // Update date&time
	    if (ShowDateTime == 1) {
		DisplayDateTime (*(Time*)E.Info.O);
	    }
	    // Call the cron handler
	    CronHandler (*(Time*)E.Info.O);
	    break;

	case evScreenSizeChange:
	    // Will clear the screen - update date&time
	    DisplayDateTime (Now ());
	    break;

	case evWinMgrNoWindows:
	    // No more open windows
	    DisableCommand (miClose);
	    DisableCommand (miZoom);
	    DisableCommand (miResize);
	    DisableCommand (miTile);
	    DisableCommand (miCascade);
	    DisableCommand (miCloseAll);
	    break;

	case evWinMgrFirstOpen:
	    // One open window
	    EnableCommand (miClose);
	    EnableCommand (miZoom);
	    EnableCommand (miResize);
	    EnableCommand (miTile);
	    EnableCommand (miCascade);
	    EnableCommand (miCloseAll);
	    break;

	case evWinMgrLastClose:
	    // Max count - 1 reached
	    EnableCommand (miOpen);
	    break;

	case evWinMgrMaxWindows:
	    // Max count of windows reached
	    DisableCommand (miOpen);
	    break;

	case evEnableCommand:
	    EnableCommand ((i16) E.Info.U);
	    break;

	case evDisableCommand:
	    DisableCommand ((i16) E.Info.U);
	    break;

	case evMatrixWinChange:
	    // New count of active matrix windows
	    if (E.Info.U == 0) {
		// No windows, enable open
		EnableCommand (miMatrixWin);
	    } else {
		// Open window, disable more windows
		DisableCommand (miMatrixWin);
	    }
	    break;

	case evCallWinChange:
	    // New count of active call windows
	    if (E.Info.U == 0) {
		// No windows, enable open
		EnableCommand (miCallWin);
	    } else {
		// Open window, disable more windows
		DisableCommand (miCallWin);
	    }
	    break;

	case evCLIWinChange:
	    // New count of active CLI windows
	    if (E.Info.U == 0) {
		// No windows, enable open
		EnableCommand (miCLIWin);
	    } else {
		// Open window, disable more windows
		DisableCommand (miCLIWin);
	    }
	    break;

	case evChargeWinChange:
	    // New count of active matrix windows
	    if (E.Info.U == 0) {
		// No windows, enable open
		EnableCommand (miChargeWin1);
		EnableCommand (miChargeWin2);
	    } else {
		// Open window, disable more windows
		DisableCommand (miChargeWin1);
		DisableCommand (miChargeWin2);
	    }
	    break;

	case evIMonWinChange:
	    // New count of active imon windows
	    if (E.Info.U == 0) {
		// No windows, enable open
		EnableCommand (miIMonWin);
	    } else {
		// Open window, disable more windows
		DisableCommand (miIMonWin);
	    }
	    break;
    }
}



void IstecApp::ReadIniFile ()
// Read default settings from an ini file
{
    // Build the name of the ini file
    String IniName = GetProgName () + ".ini";

    // Search for the ini file in the following directories:
    //	-> the current dir
    //	-> the home directory if $HOME is defined
    //	-> the support path
    IniFile* F = new IniFile (IniName);
    if (F->GetStatus () != stOk) {
	// Not found, try the home dir
	delete F;
	String HomeDir = GetEnvVar ("HOME");
	AddPathSep (HomeDir);
	F = new IniFile (HomeDir + IniName);
	if (F->GetStatus () != stOk) {
	    // Ok, last resort: try the support path
	    delete F;
	    F = new IniFile (GetSupportPath () + IniName);
	    if (F->GetStatus () != stOk) {
		// No ini file
		delete F;
		return;
	    }
	}
    }

    // F points now to a valid ini file. Read the variables
    static const char EsticSection []	  = "ESTIC";
    static const char PortSection []	  = "Port";
    static const char PrintSection []	  = "Printing";
    static const char WindowSection []	  = "Windows";
    static const char AreaCodeSection []  = "AreaCode";
    static const char LogSection []	  = "Call-Logs";
    static const char AliasSection []	  = "Alias";
    static const char CronSection []	  = "Cron";
    static const char DebugSection []	  = "Debug";
    static const char FirmwareSection []  = "Firmware";

    static const char* ShowDateTimeKeys = "1^1|2^2|0^0|0^NONE|1^MINUTES|2^SECONDS|";
    static const char* DiagModeUpdateKeys = "2^2|1^1|0^0|2^AUTO|1^ON|0^OFF|";

    String AreaCodeFile;
    String DialPrefixStr;

    SettingsFile       = F->ReadString	(EsticSection,	  "SettingsFile",	SettingsFile);
    ComPortName        = F->ReadString	(PortSection,	  "PortName",		ComPortName);
    PortBase	       = F->ReadInt	(PortSection,	  "PortBase",		PortBase);
    PortIRQ	       = F->ReadInt	(PortSection,	  "PortIRQ",		PortIRQ);
    Headline	       = F->ReadString	(PrintSection,	  "Headline",		Headline);
    Currency	       = F->ReadString	(PrintSection,	  "Currency",		Currency);
    PricePerUnit       = F->ReadFloat	(PrintSection,	  "PricePerUnit",	PricePerUnit);
    VideoMode	       = F->ReadInt	(WindowSection,   "VideoMode",		VideoMode);
    ShowDateTime       = F->ReadKeyword (WindowSection,   "ShowDateTime",	ShowDateTimeKeys);
    ShowInfoOnStartup  = F->ReadBool	(WindowSection,   "ShowInfoOnStartup",	ShowInfoOnStartup);
    AreaCodeFile       = F->ReadString	(AreaCodeSection, "AreaCodeFile",	AreaCodeFile);
    CountryCode        = F->ReadString	(AreaCodeSection, "CountryCode",	CountryCode);
    AreaCode	       = F->ReadString	(AreaCodeSection, "AreaCode",		AreaCode);
    DialPrefixStr      = F->ReadString	(AreaCodeSection, "DialPrefix",		DialPrefixStr);
    OutgoingLog1       = F->ReadString	(LogSection,	  "OutgoingLog1",	OutgoingLog1);
    OutgoingLog2       = F->ReadString	(LogSection,	  "OutgoingLog2",	OutgoingLog2);
    OutgoingLog3       = F->ReadString	(LogSection,	  "OutgoingLog3",	OutgoingLog3);
    IncomingLog1       = F->ReadString	(LogSection,	  "IncomingLog1",	IncomingLog1);
    IncomingLog2       = F->ReadString	(LogSection,	  "IncomingLog2",	IncomingLog2);
    IncomingLog3       = F->ReadString	(LogSection,	  "IncomingLog3",	IncomingLog3);
    LogZeroCostCalls   = F->ReadBool	(LogSection,	  "LogZeroCostCalls",	LogZeroCostCalls);
    XDigits	       = F->ReadInt	(LogSection,	  "XDigits",		XDigits);
    AliasFile	       = F->ReadString	(AliasSection,	  "AliasFile",		AliasFile);
    AutoReadAliases    = F->ReadBool	(AliasSection,	  "AutoReadAliases",	AutoReadAliases);
    CronFile	       = F->ReadString	(CronSection,	  "CronFile",		CronFile);
    DebugWaitAfterCall = F->ReadInt	(DebugSection,	  "WaitAfterCall",	DebugWaitAfterCall);
    ShortWaitAfterMsg  = F->ReadBool	(DebugSection,	  "ShortWaitAfterMsg",	ShortWaitAfterMsg);
    DebugLog	       = F->ReadString	(DebugSection,	  "DebugLog",		DebugLog);
    ConfigVersionHigh  = F->ReadInt	(DebugSection,	  "ConfigVersionHigh",	ConfigVersionHigh);
    ConfigVersionLow   = F->ReadInt	(DebugSection,	  "ConfigVersionLow",	ConfigVersionLow);
    DiagModeUpdate     = F->ReadKeyword (FirmwareSection, "DiagModeUpdate",	DiagModeUpdateKeys);
    FirmwareVersion    = F->ReadFloat	(FirmwareSection, "FirmwareVersion",	FirmwareVersion);
    AllowDiagMode      = F->ReadBool	(FirmwareSection, "AllowDiagMode",	AllowDiagMode);

    // If an aliasfile is defined, read it. Otherwise try to read the aliases
    // from the ini file.
    if (AliasFile.IsEmpty ()) {
	// No aliasfile defined, read the device aliases from the ini file
	for (unsigned Dev = 21; Dev < 99; Dev++) {
	    String Alias = F->ReadString (AliasSection, U32Str (Dev), "");
	    if (!Alias.IsEmpty ()) {
		NewAlias (Dev, Alias);
	    }
	}
    } else {
	// Make the path name absolute
	AliasFile = MakeAbsolute (AliasFile);

	// Read the aliasfile
	ReadAliasFile ();

	// Enable the "Reread aliases" menu entry
	MainMenue->ActivateItem (miReadAliases);
    }

    // Set some other variables
    SetAreaCodeFilename (AreaCodeFile);
    if (DialPrefixStr.Len () > 0) {
	DialPrefix = DialPrefixStr [0];
    }

    // Now close the ini file
    delete F;
}



int IstecApp::LoadConfig ()
// Calls IstecGetConfig an returns the same codes but displays a message as
// this can last some time
{
    // Pop up a window
    Window* Win = PleaseWaitWindow ();

    // Load the stuff from the istec
    int Result = IstecGetConfig (Config);

    // If we could get the configuration, read also the charges
    if (Result == ieDone) {
	Result = IstecGetCharges ();
    }

    // Delete the window
    delete Win;

    // Return the result
    return Result;
}



int IstecApp::StoreConfig ()
// Calls IstecPutConfig an returns the same codes but displays a message as
// this can last some time
{
    // Pop up a window
    Window* Win = PleaseWaitWindow ();

    // Load the stuff from the istec
    int Result = IstecPutConfig (Config);

    // Delete the window
    delete Win;

    if (Result == ieDone) {

	// Remember that we wrote a configuration
	Changes = 0;

    }

    // Return the result
    return Result;
}



int IstecApp::LoadShortNumbers ()
// Calls IstecGetShortNumbers and returns the same codes but displays
// a message as this can last some time
{
    // Pop up a window
    Window* Win = PleaseWaitWindow ();

    // Load the stuff from the istec
    int Result = IstecGetShortNumbers (ShortNumbers);

    // Delete the window
    delete Win;

    // Return the result
    return Result;
}



int IstecApp::StoreShortNumbers ()
// Calls IstecPutShortNumbers and returns the same codes but displays
// a message as this can last some time
{
    // Pop up a window
    Window* Win = PleaseWaitWindow ();

    // Load the stuff from the istec
    int Result = IstecPutShortNumbers (ShortNumbers);

    // Delete the window
    delete Win;

    if (Result == ieDone) {

	// Remember that we wrote the short numbers
	ShortNumberChanges = 0;

    }

    // Return the result
    return Result;
}



void IstecApp::InitIstecConfig ()
// Try to connect to the istec and download the istec configuration.
// If this is impossible, initialize the configuration data to known
// values.
{
    // Initialize the configuration in any case since the following
    // LoadConfig will not load all device configurations
    LoadConfigDefault (Config, 1008);

    // If we could connect to the istec, try to download the configuration.
    if (IstecPresent == 0 || EvalCmd (LoadConfig ()) == 0) {

	// No istec or read error, use defaults
	IstecPresent = 0;
	Charges.Clear ();

    } else {

	// We did not have an error, load the short number info if the
	// firmware has such stuff
	if (FirmwareVersion >= 2.00) {
	    // Put 30 short numbers into the collection
	    for (unsigned I = 1; I < 60; I++) {
		ShortNumbers.NewShortNumber (I);
	    }
	    EvalCmd (LoadShortNumbers ());
	}
    }
}



String IstecApp::GetIstecName ()
// Return the name of the istec, determined by the parameters of the base
// configuration.
{
    return ::GetIstecName (Config.IstecID ());
}



const char* IstecApp::GetProtocolName (unsigned Prot)
// Return the protocol name used by the istec
{
    switch (Prot) {

	case pr1TR6:
	    return "1TR6";

	case prDSS1:
	    return "DSS1";

	default:
	    return "";
    }
}



const String& IstecApp::GetConnectionName (unsigned Conn)
// Return the connection type of the istec
{
    unsigned MsgNum;
    switch (Conn) {

	case coPointToMulti:
	    MsgNum = msConnMehrgeraete;
	    break;

	case coPointToPoint:
	    MsgNum = msConnAnlagen;
	    break;

	default:
	    MsgNum = msConnUnknown;
	    break;

    }

    return LoadAppMsg (MsgNum);
}



void IstecApp::ShowIstecConfig ()
// Show the istec configuration
{
    // Get some strings now to work around a gcc bug
    String IstecName = GetIstecName ();
    String ConnectionName = GetConnectionName (Config.GetConnection ());

    // Set up the message to display
    String Msg =  FormatStr (
		    LoadAppMsg (msIstecConfig).GetStr (),
		    IstecName.GetStr (),
		    Config.BaseConfig.VersionHigh,
		    Config.BaseConfig.VersionLow,
		    Config.GetExtS0 (),
		    Config.GetIntS0 (),
		    Config.GetDevCount (),
		    GetProtocolName (Config.GetProtocol ()),
		    ConnectionName.GetStr ()
		  );

    InformationMsg (Msg);

}



void IstecApp::DisableCommand (i16 ID)
// Disable the command bound to the menue item with the given ID
{
    // Gray the item
    MainMenue->GrayItem (ID);

    // Get the accel key of the item
    Key AccelKey = MainMenue->GetAccelKey (ID);

    // If this key is registered, unregister it
    if (AccelKey != kbNoKey && KeyIsRegistered (AccelKey)) {
	UnregisterKey (AccelKey);
    }

}



void IstecApp::EnableCommand (i16 ID)
// Enable the command bound to the menue item with the given ID
{
    // Enable the item
    MainMenue->ActivateItem (ID);

    // Get the accel key of the item
    Key AccelKey = MainMenue->GetAccelKey (ID);

    // If this key is not registered, do it
    if (AccelKey != kbNoKey && KeyIsRegistered (AccelKey) == 0) {
	RegisterKey (AccelKey);
    }
}



TopMenueBar* IstecApp::CreateMenueBar ()
{
    TopMenueBar* M = (TopMenueBar*) App->LoadResource ("@ESTIC.MainMenue");

    // Register the accel keys of the submenues
    App->RegisterKey (M->GetAccelKey (miEstic));
    App->RegisterKey (M->GetAccelKey (miFile));
    App->RegisterKey (M->GetAccelKey (miIstec));
    App->RegisterKey (M->GetAccelKey (miCharges));
    App->RegisterKey (M->GetAccelKey (miWindow));
    App->RegisterKey (M->GetAccelKey (miOpen));
    App->RegisterKey (M->GetAccelKey (miClose));

    // Register the accel keys
    App->RegisterKey (M->GetAccelKey (miQuit));
    App->RegisterKey (M->GetAccelKey (miWindowList));

    // Gray unused items
    M->GrayItem (miReadAliases);
    M->GrayItem (miTile);
    M->GrayItem (miCascade);
    M->GrayItem (miCloseAll);
    M->GrayItem (miZoom);
    M->GrayItem (miResize);
    M->GrayItem (miClose);

#ifndef LINUX
    M->GrayItem (miIMonWin);
#endif

    // Return the result
    return M;
}



BottomStatusLine* IstecApp::CreateStatusLine ()
{
    const u32 Flags = siAltX_Exit;
    ((IstecApp*) App)->StatusFlags = Flags;
    return new BottomStatusLine (Flags);
}



void IstecApp::AskWriteChanges ()
// If there are differnces between the configuration stored in the istec and
// the configuration in memory, ask to store the config data into the istec.
{
    if (IstecPresent) {

	if (Changes) {

	    // We have changes
	    if (AskYesNo (LoadAppMsg (msWriteChanges)) == 2) {

		// We should write the changes...
		if (EvalCmd (StoreConfig ())) {
		    // Done
		    Changes = 0;
		}

	    }
	}

    } else {

	// No istec present, assume changes are written
	Changes = 0;

    }
}



void IstecApp::AskWriteShortNumbers ()
// If there are differnces between the short numbers stored in the istec and
// the short numbers in memory, ask to store the numbers into the istec.
{
    if (IstecPresent) {

	if (ShortNumberChanges) {

	    // We have changes
	    if (AskYesNo (LoadAppMsg (msWriteShortNumbers)) == 2) {

		// We should write the changes...
		if (EvalCmd (StoreShortNumbers ())) {
		    // Done
		    ShortNumberChanges = 0;
		}

	    }
	}

    } else {

	// No istec present, assume changes are written
	ShortNumberChanges = 0;

    }
}



int IstecApp::EvalCmd (int RetCode)
// Evaluate the return code of an istec command function. If the return
// code denotes an error, an error message is poped up. The function
// return 1 if the command has been error free, 0 on errors.
{
    switch (RetCode) {

	case ieRecBufOverflow:
	    // Receive buffer overflow
	    IstecError (msRecBufOverflow);
	    IstecErrorSync ();
	    return 0;

	case ieRecBufUnderflow:
	    // Receive buffer underflow
	    IstecError (msRecBufUnderflow);
	    IstecErrorSync ();
	    return 0;

	case ieInvalidReply:
	    // Invalid replay
	    IstecError (msInvalidReply);
	    IstecErrorSync ();
	    return 0;

	case ieWrongDevice:
	    // Wrong device number in reply
	    IstecError (msWrongDevice);
	    IstecErrorSync ();
	    return 0;

	case iePortNotOpen:
	    // COM port not open
	    IstecError (msComPortNotOpen);
	    return 0;

	case ieTimeout:
	    // Timeout
	    IstecError (msIstecTimeout);
	    return 0;

	case ieDone:
	    // Success
	    return 1;

	default:
	    FAIL ("IstecApp::EvalCmd: Unexpected return code");

    }

    // Never reached
    return 0;
}



void IstecApp::SetSaveDir (const String& Path)
// Strips the name part from the given filename and stores the remaining part
// (the directory) including the trailing path separator into SaveDir.
{
    String Name;
    FSplit (Path, SaveDir, Name);
}



void IstecApp::LoadFile ()
// Load the configuration from a file
{
    // Choose the file name
    FileSelector FS (LoadAppMsg (msLoadFileHeader), ".ic");
    String Sel = FS.GetChoice (SaveDir + "*.ic");
    if (!Sel.IsEmpty ()) {

	// Load the config from a file
	String Msg = IstecLoadFile (Sel, Config);

	// Check for errors
	if (!Msg.IsEmpty ()) {

	    // Error
	    ErrorMsg (Msg);

	} else {

	    // Remember the directory used
	    SetSaveDir (Sel);

	    // Show the configuration just read
	    ShowIstecConfig ();

	    // Data has been changed
	    Changes = 1;

	    // Ask to write the changes to the istec
	    AskWriteChanges ();
	}
    }
}



void IstecApp::SaveFile ()
// Save the current configuration to a file
{
    // Choose the file name
    FileSelector FS (LoadAppMsg (msSaveFileHeader), ".ic", fsFileMayNotExist);
    String Sel = FS.GetChoice (SaveDir + "*.ic");
    if (!Sel.IsEmpty ()) {

	// Save the config to a file
	String Msg = IstecSaveFile (Sel, Config);

	// Check for errors
	if (!Msg.IsEmpty ()) {
	    // Error
	    ErrorMsg (Msg);
	} else {
	    // Remember the directory used
	    SetSaveDir (Sel);
	}
    }
}



void IstecApp::LoadShort ()
// Load the short numbers from a file
{
    // Choose the file name
    FileSelector FS (LoadAppMsg (msLoadFileHeader), ".sn");
    String Sel = FS.GetChoice (SaveDir + "*.sn");
    if (!Sel.IsEmpty ()) {

	// Load the config from a file
	String Msg = FileLoadShort (Sel, ShortNumbers);

	// Check for errors
	if (!Msg.IsEmpty ()) {

	    // Error
	    ErrorMsg (Msg);

	} else {

	    // Remember the directory used
	    SetSaveDir (Sel);

	    // Data has been changed
	    ShortNumberChanges = 1;

	    // Ask to write the changes to the istec
	    AskWriteShortNumbers ();
	}
    }
}



void IstecApp::SaveShort ()
// Save the current short numbers to a file
{
    // Choose the file name
    FileSelector FS (LoadAppMsg (msSaveFileHeader), ".sn", fsFileMayNotExist);
    String Sel = FS.GetChoice (SaveDir + "*.sn");
    if (!Sel.IsEmpty ()) {

	// Save the config to a file
	String Msg = FileSaveShort (Sel, ShortNumbers);

	// Check for errors
	if (!Msg.IsEmpty ()) {
	    // Error
	    ErrorMsg (Msg);
	} else {
	    // Remember the directory used
	    SetSaveDir (Sel);
	}
    }
}



void IstecApp::ViewLog ()
// View a logfile
{
    // Ask for the file name
    FileSelector FS (LoadAppMsg (msViewLogSelHeader), ".log");
    String LogFile = FS.GetChoice ();
    if (LogFile.IsEmpty ()) {
	// User abort
	return;
    }

    // Open the file viewer
    FileViewer* Viewer = new FileViewer (LogFile,
					 Background->GetDesktop (),
					 wfFramed | wfCanMove | wfCanResize,
					 paBlue);

    // Check for errors, browse the file
    if (Viewer->GetStatus () != stOk) {
	// OOPS, error opening the file or something like that
	ErrorMsg (GetSysErrorMsg (Viewer->GetErrorInfo ()));
	delete Viewer;
    } else {
	// Ok, insert the file into the window manager
	WinMgr->AddWindow (Viewer);
	WinMgr->Browse (Viewer);
    }
}



void IstecApp::LoadIstec ()
// Load the configuration from the istec
{
    if (EvalCmd (IstecReady ())) {
	// Well, we _can_ talk to the istec, try to read the config
	if (EvalCmd (LoadConfig ())) {
	    // Error free, display the configuration
	    ShowIstecConfig ();
	}
    }
}



void IstecApp::SaveIstec ()
// Save the current config to the istec
{
    EvalCmd (StoreConfig ());
}



void IstecApp::SysParams ()
// Edit system parameters
{
    // Edit the parameters
    int NewChanges = 0;
    EditBaseConfig (Config, IstecPresent, NewChanges);

    // Remember if we had changes
    Changes |= NewChanges;

    // Ask for writing the changes back if we have new changes
    if (NewChanges) {
	AskWriteChanges ();
    }
}



void IstecApp::EditShortNumbers ()
// Edit shortcut numbers
{
    // Edit the parameters
    int NewChanges = 0;
    ShortNumberList (ShortNumbers, NewChanges);

    // Remember if we had changes
    ShortNumberChanges |= NewChanges;

    // Ask for writing the changes back if we have new changes
    if (NewChanges) {
	AskWriteShortNumbers ();
    }
}



void IstecApp::DevParams ()
// Set the device parameters
{
    // Edit the parameters
    int NewChanges = 0;
    DeviceList (Config, NewChanges);

    // Remember if we had changes
    Changes |= Changes || NewChanges;

    // Ask for writing the changes back if we have new changes
    if (NewChanges) {
	AskWriteChanges ();
    }
}



void IstecApp::Reset ()
// Reset the istec
{
    if (AskAreYouShure () == 2) {

	// Reset the device params
	LoadConfigDefault (Config, Config.IstecID ());

	// We have changes now
	Changes = 1;

	// Ask to write those changes
	AskWriteChanges ();

    }
}



void IstecApp::LoadCharges ()
// Reload the charges from the istec
{
    // Pop up a window
    Window* Win = PleaseWaitWindow ();

    // Load the stuff from the istec
    int Result = IstecGetCharges ();

    // Delete the window
    delete Win;

    // Print an error message if needed
    EvalCmd (Result);
}



void IstecApp::PrintSettings ()
// Edit settings for printing charges
{
    // Name of the settings resource
    static const String StgPosName = "PrintSettings.PrintSettingsMenue.Position";

    // Load the menue
    Menue* M = (Menue*) LoadResource ("@ISTEC.PrintSettingsMenue");

    // If there is a stored window position, move the window to that position
    Point Pos = StgGetPoint (StgPosName, M->OuterBounds ().A);
    M->MoveAbs (Pos);

    // Remember the old values
    String OldHeadline	   = Headline;
    double OldPricePerUnit = PricePerUnit;

    // Transfer the current values to the menue
    M->SetStringValue (1, Headline);
    M->SetFloatValue (2, PricePerUnit);

    // Create a new status line
    PushStatusLine (siAbort | siSelectKeys | siAccept);

    // Activate the menue
    M->Activate ();

    // Accept user input
    int Done = 0;
    while (!Done) {

	// Get a selection
	int Sel = M->GetChoice ();

	// Evaluate the selection
	switch (Sel) {

	    case 1:
		Headline = M->GetStringValue (1);
		break;

	    case 2:
		PricePerUnit = M->GetFloatValue (2);
		break;

	    case 0:
		if (M->GetAbortKey () == vkAbort) {
		    // Abort - ask if we have changes
		    if (PricePerUnit != OldPricePerUnit || Headline != OldHeadline) {
			// We have changes
			if (AskDiscardChanges () == 2) {
			    // Discard changes
			    PricePerUnit = OldPricePerUnit;
			    Headline = OldHeadline;
			    Done = 1;
			}
		    } else {
			// No changes
			Done = 1;
		    }
		} else if (M->GetAbortKey () == vkAccept) {
		    // Accept the changes
		    Done = 1;
		}
		break;

	}

    }

    // Restore the old status line
    PopStatusLine ();

    // Save the current window position
    StgPutPoint (M->OuterBounds ().A, StgPosName);

    // Delete the menue
    delete M;
}



void IstecApp::PrintCharges ()
// Print the charges
{
    // Choose the file name
    FileSelector FS (LoadAppMsg (msPrintFileSelHeader), ".geb", fsFileMayNotExist);
    String Sel = FS.GetChoice (SaveDir + "*.geb");
    if (Sel.IsEmpty ()) {
	// Empty selection means abort
	return;
    }

    // Open the file
    FILE* F = fopen (Sel.GetStr (), "wt");
    if (F == NULL) {
	ErrorMsg (GetSysErrorMsg (errno));
	return;
    }

    // Some space
    fprintf (F, "\n\n");

    // Headline
    String Line = Headline;
    fprintf (F, "%s\n", Line.OutputCvt ().GetStr ());

    // Internal headline
    String TimeStr = Time ().DateTimeStr ();
    String IstecName = GetIstecName ();
    Line = FormatStr (LoadAppMsg (msPrintHeader).GetStr (),
		      IstecName.GetStr (),
		      TimeStr.GetStr ());
    fprintf (F, "%s\n", Line.OutputCvt ().GetStr ());

    // Divider
    Line = LoadAppMsg (msPrintHeader2).GetStr ();
    fprintf (F, "%s\n", Line.OutputCvt ().GetStr ());

    // Empty line followed by the table header
    Line = LoadAppMsg (msChargeTableHeader).GetStr ();
    fprintf (F, "\n%s\n\n", Line.OutputCvt ().GetStr ());

    // Ok, loop through the devices...
    for (unsigned Dev = 0; Dev < Config.GetDevCount (); Dev++) {
	Line = LoadAppMsg (msChargeLine);
	Line = FormatStr (Line.GetStr (), Dev + 21, Charges [Dev]);
	String Price = FloatStr (Charges [Dev] * PricePerUnit, 4, 2);
	Price.Pad (String::Left, 7);
	Line += Price;
	fprintf (F, "%s\n", Line.OutputCvt ().GetStr ());
    }

    // Close the file
    fclose (F);

    // Remember the directory used
    SetSaveDir (Sel);

}



void IstecApp::ResetCharges ()
// Reset all charges
{
    if (AskAreYouShure () == 2) {

	// Create empty charges
	IstecCharges Charges;

	if (IstecPresent) {

	    // Pop up a window cause this could last some time
	    Window* WaitWin = PleaseWaitWindow ();

	    // Send the istec command
	    IstecPutCharges (Charges);

	    // Delete the window
	    delete WaitWin;

	}
    }
}



void IstecApp::CloseAll ()
// Close all windows
{
    WinMgr->CloseAll ();
}



void IstecApp::Resize (ItemWindow* Win)
// Resize a window
{
    if (Win) {
	Win->MoveResize ();
    }
}



void IstecApp::Zoom (ItemWindow* Win)
// Zoom a window
{
    if (Win && Win->CanResize ()) {
	Win->Zoom ();
    }
}



void IstecApp::Close (ItemWindow* Win)
// Close a window
{
    if (Win) {
	WinMgr->DeleteWindow (Win);
    }
}



int IstecApp::Run ()
{
    // Activate the main menue
    MainMenue->Activate ();

    // Main loop
    while (!Quitting ()) {

	Key K;

	// Switch according to the users choice
	switch (MainMenue->GetChoice ()) {

	    case miAbout:
		InformationMsg (LoadAppMsg (msAboutInfo));
		break;

	    case miFileLoadConfig:
		LoadFile ();
		break;

	    case miFileSaveConfig:
		SaveFile ();
		break;

	    case miFileLoadShort:
		LoadShort ();
		break;

	    case miFileSaveShort:
		SaveShort ();
		break;

	    case miViewLog:
		ViewLog ();
		break;

	    case miReadAliases:
		ReadAliasFile ();
		break;

	    case miReadCronFile:
		ReadCronFile ();
		break;

	    case miQuit:
		// Close the windows
		if (WinMgr->CanClose ()) {

		    // Save the window manager, then close all windows
		    StgPut (WinMgr, "WinMgr");
		    WinMgr->CloseAll ();

		    // Switch the istec out of diag mode (only if istec present)
		    if (IstecPresent) {
			IstecDiagOff ();
		    }

		    // End the program. First ask to write any changes back
		    AskWriteChanges ();
		    AskWriteShortNumbers ();

		    // The end...
		    Quit = 1;
		}
		break;

	    case miIstecLoadConfig:
		LoadIstec ();
		break;

	    case miIstecSaveConfig:
		SaveIstec ();
		break;

	    case miIstecLoadShort:
		LoadShortNumbers ();
		break;

	    case miIstecSaveShort:
		StoreShortNumbers ();
		break;

	    case miVersion:
		ShowIstecConfig ();
		break;

	    case miEditSysParams:
		SysParams ();
		break;

	    case miEditDevParams:
		DevParams ();
		break;

	    case miEditShort:
		EditShortNumbers ();
		break;

	    case miReset:
		Reset ();
		break;

	    case miLoadCharges:
		LoadCharges ();
		break;

	    case miChargeWin1:
	    case miChargeWin2:
		WinMgr->AddWindow (new ChargeWindow);
		break;

	    case miPrintSettings:
		PrintSettings ();
		break;

	    case miPrintCharges:
		PrintCharges ();
		break;

	    case miResetCharges:
		ResetCharges ();
		break;

	    case miMatrixWin:
		WinMgr->AddWindow (
		    new MatrixWindow (Point (10, 10), Config.GetDevCount ()));
		break;

	    case miCallWin:
		WinMgr->AddWindow (new CallWindow);
		break;

	    case miCLIWin:
		WinMgr->AddWindow (new CLIWindow);
		break;

#ifdef LINUX
	    case miIMonWin:
		WinMgr->AddWindow (new IMonWindow (Point (10, 10)));
		break;
#endif

	    case miTile:
		WinMgr->Tile ();
		break;

	    case miCascade:
		WinMgr->Cascade ();
		break;

	    case miCloseAll:
		CloseAll ();
		break;

	    case miRedraw:
		RedrawScreen ();
		break;

	    case miResize:
		Resize (WinMgr->GetTopWindow ());
		break;

	    case miZoom:
		Zoom (WinMgr->GetTopWindow ());
		break;

	    case miClose:
		Close (WinMgr->GetTopWindow ());
		break;

	    case miWindowList:
		WinMgr->Browse (WinMgr->ChooseWindow ());
		break;

	    case 0:
		K = MainMenue->GetAbortKey ();
		if (K != vkAbort) {
		    // Window hotkey
		    WinMgr->Browse (WinMgr->FindWindowWithKey (K));
		}
		break;

	}

    }

    // Close the debug logfile
    DoneDebugLog ();

    // Return the program exit code
    return 0;
}




int main (int argc, char* argv [])
{
    // Set the default language and country
    DefaultLanguage = laGerman;
    DefaultCountry = 49;

    // Declare an application object
    IstecApp MyApp (argc, argv);

    // Use it...
    return MyApp.Run ();

}




