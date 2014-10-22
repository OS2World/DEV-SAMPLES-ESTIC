/*****************************************************************************/
/*                                                                           */
/*                                  ISTEC.H                                  */
/*                                                                           */
/* (C) 1995-97  Ullrich von Bassewitz                                        */
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



#ifndef _ESTIC_H
#define _ESTIC_H



#include "program.h"

#include "icconfig.h"
#include "icshort.h"



/*****************************************************************************/
/*                               Menue items                                 */
/*****************************************************************************/



const i16 miNone                        =    1;

const i16 miEstic                       = 1000;
const i16 miAbout                       = 1100;

const i16 miFile                        = 2000;
const i16 miFileLoadConfig              = 2100;
const i16 miFileSaveConfig              = 2200;
const i16 miFileLoadShort               = 2300;
const i16 miFileSaveShort               = 2400;
const i16 miViewLog                     = 2500;
const i16 miReadAliases                 = 2600;
const i16 miReadCronFile                = 2700;
const i16 miQuit                        = 2800;

const i16 miIstec                       = 3000;
const i16 miIstecLoadConfig             = 3100;
const i16 miIstecSaveConfig             = 3200;
const i16 miIstecLoadShort              = 3300;
const i16 miIstecSaveShort              = 3400;
const i16 miVersion                     = 3500;
const i16 miEditSysParams               = 3600;
const i16 miEditDevParams               = 3700;
const i16 miEditShort                   = 3800;
const i16 miReset                       = 3900;

const i16 miCharges                     = 4000;
const i16 miLoadCharges                 = 4100;
const i16 miChargeWin1                  = 4200;
const i16 miPrintSettings               = 4300;
const i16 miPrintCharges                = 4400;
const i16 miResetCharges                = 4500;

const i16 miWindow                      = 5000;
const i16 miOpen                        = 5100;
const i16 miChargeWin2                  = 5110;
const i16 miMatrixWin                   = 5120;
const i16 miCallWin                     = 5130;
const i16 miCLIWin                      = 5140;
const i16 miIMonWin                     = 5150;
const i16 miTile                        = 5200;
const i16 miCascade                     = 5300;
const i16 miCloseAll                    = 5400;
const i16 miRedraw                      = 5500;
const i16 miResize                      = 5600;
const i16 miZoom                        = 5700;
const i16 miClose                       = 5800;
const i16 miWindowList                  = 5900;



/*****************************************************************************/
/*                              class IstecApp                               */
/*****************************************************************************/



// Forwards
class IstecOptions;
class ComPort;



class IstecApp: public Program {


private:
    static TopMenueBar* CreateMenueBar ();
    static BottomStatusLine* CreateStatusLine ();

protected:
    u32                 StatusFlags;            // statusline flags
    String              ComPortName;            // Device name
    String              SettingsFile;           // Name of options file

    int                 IstecPresent;
    int                 Changes;                // True if we have changes
    IstecConfig         Config;
    ShortNumberColl     ShortNumbers;
    int                 ShortNumberChanges;

    // Files
    String              SaveDir;                // Default load/save directory
    String              DebugLog;               // Debug log file name

    // Printing
    String              Headline;
    String              Currency;

    // Screen & Windows
    unsigned            VideoMode;
    int                 ShowDateTime;
    int                 ShowInfoOnStartup;

    // Update related
    int                 DiagModeUpdate;
    Time                LastUpdate;
    int                 DiagModeUpdateCounter;
    int                 ChargesUpdateCounter;



    void CronHandler (const Time& T);
    // Is called from idle every minute, checks periodic events

    void DisplayDateTime (const Time& T);
    // Display the time in the upper right corner

    void ReadIniFile ();
    // Read default settings from an ini file

    int LoadConfig ();
    // Calls IstecGetConfig an returns the same codes but displays a message as
    // this can last some time

    int StoreConfig ();
    // Calls IstecPutConfig an returns the same codes but displays a message as
    // this can last some time

    int LoadShortNumbers ();
    // Calls IstecGetShortNumbers and returns the same codes but displays
    // a message as this can last some time

    int StoreShortNumbers ();
    // Calls IstecPutShortNumbers and returns the same codes but displays
    // a message as this can last some time

    void InitIstecConfig ();
    // Try to connect to the istec and download the istec configuration.
    // If this is impossible, initialize the configuration data to known
    // values.

    void AskWriteChanges ();
    // If there are differnces between the configuration stored in the istec and
    // the configuration in memory, ask to store the config data into the istec.

    void AskWriteShortNumbers ();
    // If there are differnces between the short numbers stored in the istec and
    // the short numbers in memory, ask to store the numbers into the istec.

    String GetIstecName ();
    // Return the name of the istec, determined by the parameters of the base
    // configuration.

    const char* GetProtocolName (unsigned Prot);
    // Return the protocol name used by the istec

    const String& GetConnectionName (unsigned Conn);
    // Return the connection type of the istec

    void ShowIstecConfig ();
    // Show the base configuration

    int EvalCmd (int RetCode);
    // Evaluate the return code of an istec command function. If the return
    // code denotes an error, an error message is poped up. The function
    // return 1 if the command has been error free, 0 on errors.

    void SetSaveDir (const String& Path);
    // Strips the name part from the given filename and stores the remaining part
    // (the directory) including the trailing path separator into SaveDir.

    void LoadFile ();
    // Load the configuration from a file

    void SaveFile ();
    // Save the current configuration to a file

    void LoadShort ();
    // Load the short numbers from a file

    void SaveShort ();
    // Save the current short numbers to a file

    void ViewLog ();
    // View a logfile

    void LoadIstec ();
    // Load the configuration from the istec

    void SaveIstec ();
    // Save the current config to the istec

    void SysParams ();
    // Edit istec system parameters

    void DevParams ();
    // Set the device parameters

    void EditShortNumbers ();
    // Edit shortcut numbers

    void Reset ();
    // Reset the istec

    void LoadCharges ();
    // Reload the charges from the istec

    void PrintSettings ();
    // Edit settings for printing charges

    void PrintCharges ();
    // Print the charges

    void ResetCharges ();
    // Reset all charges

    void CloseAll ();
    // Close all windows

    void Resize (ItemWindow* Win);
    // Resize a window

    void Zoom (ItemWindow* Win);
    // Zoom a window

    void Close (ItemWindow* Win);
    // Close a window

    void BackgroundWork (const Time& Current);
    // Idle function. Is used to check for debug messages in the receive queue



public:
    IstecApp (int argc, char* argv []);
    // Construct an application object

    virtual ~IstecApp ();
    // Destruct an application object

    virtual int Run ();
    // Run the application

    void HandleEvent (Event& E);
    // Handle incoming events. Calls Update() if the application is idle

    virtual void DisableCommand (i16 ID);
    // Disable the command bound to the menue item with the given ID

    virtual void EnableCommand (i16 ID);
    // Enable the command bound to the menue item with the given ID

};



// End of ESTIC.H

#endif
