/*****************************************************************************/
/*                                                                           */
/*                                ICBASEED.CC                                */
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



#include "cont.h"
#include "progutil.h"
#include "menue.h"
#include "menuitem.h"
#include "statline.h"
#include "stdmenue.h"
#include "stdmsg.h"
#include "settings.h"
#include "crcstrm.h"
#include "memstrm.h"

#include "icmsg.h"
#include "icerror.h"
#include "icconfig.h"
#include "icident.h"



/*****************************************************************************/
/*                             Message constants                             */
/*****************************************************************************/



const u16 msExtLevel            = MSGBASE_ICBASEED +  0;
const u16 msNumberTooLong       = MSGBASE_ICBASEED +  1;



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



static void EditQueryLoc (IstecBaseConfig& Cfg)
{
    // Name of the settings resource
    static const String StgPosName = "EditQueryLoc.Abfragestellen-Menue.Position";

    // Remember the old config, remember the CRC
    u32 OldCRC = GetCRC (Cfg);
    MemoryStream SaveStream;
    SaveStream << Cfg;
    SaveStream.Seek (0);

    // Load the menue
    Menue* Win = (Menue*) LoadResource ("@ICBASEED.Abfragestellen-Menue");

    // If there is a stored window position, move the window to that position
    Point Pos = StgGetPoint (StgPosName, Win->OuterBounds ().A);
    Win->MoveAbs (Pos);

    // Get pointers to the items
    LongItem* QL1 = (LongItem*) Win->ForcedItemWithID (1);
    LongItem* QL2 = (LongItem*) Win->ForcedItemWithID (2);

    // Transfer data into the menue
    QL1->SetValue (Cfg.QueryLoc1);
    QL2->SetValue (Cfg.QueryLoc2);

    // Set the limits
    QL1->SetMinMax (21, 20 + Cfg.DevCount);
    QL2->SetMinMax (21, 20 + Cfg.DevCount);

    // Activate the menue
    Win->Activate ();

    // New status line
    PushStatusLine (siAbort | siSelectKeys | siChange | siAccept);

    // Edit the menue
    int Done = 0;
    while (!Done) {

        // Get a selection
        switch (Win->GetChoice ()) {

            case 1:
                Cfg.QueryLoc1 = QL1->GetValue ();
                break;

            case 2:
                Cfg.QueryLoc2 = QL2->GetValue ();
                break;

            case 0:
                // Check the abort key
                if (Win->GetAbortKey () == vkAbort) {
                    // Abort - ask if we have changes
                    if (GetCRC (Cfg) != OldCRC) {
                        // We have changes
                        if (AskDiscardChanges () == 2) {
                            // Discard changes, reload the data from the stream
                            SaveStream >> Cfg;
                            Done = 1;
                        }
                    } else {
                        // No changes
                        Done = 1;
                    }
                } else if (Win->GetAbortKey () == vkAccept) {
                    // Accept the changes
                    Done = 1;
                }
                break;

        }
    }

    // Save the current window position
    StgPutPoint (Win->OuterBounds ().A, StgPosName);

    // Pop statusline, delete the menue
    PopStatusLine ();
    delete Win;
}



static void EditMSN (IstecBaseConfig& Config)
// Edit the MSN's
{
    // Name of the settings resource
    static const String StgPosName = "EditMSN.MSN-Eingabemenue.Position";

    // Remember the old config, remember the CRC
    u32 OldCRC = GetCRC (Config);
    MemoryStream SaveStream;
    SaveStream << Config;
    SaveStream.Seek (0);

    // Load the menue
    Menue* Win = (Menue*) LoadResource ("@ICBASEED.MSN-Eingabemenue");

    // If there is a stored window position, move the window to that position
    Point Pos = StgGetPoint (StgPosName, Win->OuterBounds ().A);
    Win->MoveAbs (Pos);

    // Transfer data into the menue
    for (unsigned I = 0; I < 10; I++) {
        Win->SetStringValue (I+1, FromBCD (Config.MSN [I], sizeof (Config.MSN [I])));
    }

    // Activate the menue
    Win->Activate ();

    // New status line
    PushStatusLine (siAbort | siSelectKeys | siChange | siAccept);

    // Edit the menue
    int Done = 0;
    while (!Done) {

        // Get a selection
        int Choice = Win->GetChoice ();

        //
        const char* S;
        switch (Choice) {

            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
                S = Win->GetStringValue (Choice).GetStr ();
                ToBCD (S, Config.MSN [Choice-1], sizeof (Config.MSN [0]));
                break;

            case 0:
                if (Win->GetAbortKey () == vkAbort) {
                    // Abort - ask if we have changes
                    if (GetCRC (Config) != OldCRC) {
                        // We have changes
                        if (AskDiscardChanges () == 2) {
                            // Discard changes, reload the data from the stream
                            SaveStream >> Config;
                            Done = 1;
                        }
                    } else {
                        // No changes
                        Done = 1;
                    }
                } else if (Win->GetAbortKey () == vkAccept) {
                    // Accept the changes
                    Done = 1;
                }
                break;

        }

    }

    // Save the current window position
    StgPutPoint (Win->OuterBounds ().A, StgPosName);

    // Pop statusline, delete the menue
    PopStatusLine ();
    delete Win;
}



static void EditMSNGroups (IstecBaseConfig& Cfg, const String& ResName)
// Edit the MSN or EAZ grouping
{
    // Name of the settings resource
    const String StgPosName = "EditMSNGroups." + ResName + ".Position";

    // Load the menue
    Menue* M = (Menue*) LoadResource (ResName);

    // If there is a stored window position, move the window to that position
    Point Pos = StgGetPoint (StgPosName, M->OuterBounds ().A);
    M->MoveAbs (Pos);

    // Make a copy of the MSN groups
    unsigned char OldGroups [sizeof (Cfg.MSNGroups)];
    memcpy (OldGroups, Cfg.MSNGroups, sizeof (OldGroups));

    // Set up the menue
    for (unsigned I = 0; I < 10; I++) {
        // Set the MSN if the item for the MSN is available (otherwise it is
        // the EAZ grouping menue which has no MS numbers to show)
        StringItem* Item = (StringItem*) M->ItemWithID (1000 + I);
        if (Item != NULL) {
            String MSN = FromBCD (Cfg.MSN [I], sizeof (Cfg.MSN [I]));
            Item->SetItemText (Item->GetItemText () + " (" + MSN + ")");
        }

        // Set the grouping
        unsigned Mask = 0x01;
        for (unsigned J = 0; J < 8; J++) {
            M->SetToggleValue (I*16+J*2+1, (Cfg.MSNGroups [I] & Mask) != 0);
            Mask <<= 1;
        }
    }

    // New statusline
    PushStatusLine (siAbort | siAccept | siSelectKeys | siChange);

    // Activate the menue
    M->Activate ();

    // Allow editing
    int Done = 0;
    while (!Done) {

        // Get a selection
        unsigned Sel = M->GetChoice ();

        // Check for abort
        if (Sel == 0) {
            if (M->GetAbortKey () == vkAbort) {
                // Abort - ask if we have changes
                if (memcmp (OldGroups, Cfg.MSNGroups, sizeof (OldGroups)) != 0) {
                    // We have changes
                    if (AskDiscardChanges () == 2) {
                        // Discard changes
                        memcpy (Cfg.MSNGroups, OldGroups, sizeof (OldGroups));
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

        } else {

            unsigned MSN = (Sel - 1) / 16;
            unsigned Mask = 0x01 << (((Sel - 1) % 16) / 2);

            if (Sel & 0x0001)  {
                // Switch off
                Cfg.MSNGroups [MSN] &= ~Mask;
            } else {
                // Switch on
                Cfg.MSNGroups [MSN] |= Mask;
            }
        }
    }

    // Old status line
    PopStatusLine ();

    // Save the current window position
    StgPutPoint (M->OuterBounds ().A, StgPosName);

    // Discard the menue
    delete M;
}



static void EditSignaling (IstecBaseConfig& Cfg, const Point& Pos)
// Edit the signal types for the different MSN's
{
    // Load the menue
    Menue* M = (Menue*) LoadResource ("@ICBASEED.MSN-Rufsignale");

    // Move the menue near to the given position
    M->PlaceNear (Pos);

    // Make a copy of the MSN signaling
    unsigned char OldSignaling [sizeof (Cfg.Signaling)];
    memcpy (OldSignaling, Cfg.Signaling, sizeof (Cfg.Signaling));

    // Set up the menue
    for (unsigned I = 0; I < bcMSNCount; I++) {
        // Put the MSN into the name
        ToggleItem* T = (ToggleItem*) M->ItemWithID ((I+1) * 10);
        String MSN = FromBCD (Cfg.MSN [I], sizeof (Cfg.MSN [I]));
        T->SetItemText (T->GetItemText () + " (" + MSN + ")");
        T->SetValue (Cfg.Signaling [I]);
    }

    // New statusline
    PushStatusLine (siAbort | siAccept | siSelectKeys | siChange);

    // Activate the menue
    M->Activate ();

    // Allow editing
    int Done = 0;
    while (!Done) {

        // Get a selection
        unsigned Sel = M->GetChoice ();

        // Check for abort
        if (Sel == 0) {
            if (M->GetAbortKey () == vkAbort) {
                // Abort - ask if we have changes
                if (memcmp (OldSignaling, Cfg.Signaling, sizeof (OldSignaling)) != 0) {
                    // We have changes
                    if (AskDiscardChanges () == 2) {
                        // Discard changes
                        memcpy (Cfg.Signaling, OldSignaling, sizeof (OldSignaling));
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

        } else {

            // A value has changed
            unsigned MSN = (Sel / 10) - 1;
            unsigned Val = Sel % 10;
            Cfg.Signaling [MSN] = Val;

        }
    }

    // Old status line
    PopStatusLine ();

    // Discard the menue
    delete M;
}



void LoadConfigDefault (IstecConfig& Config, unsigned IstecType)
// Load the default parameter settings for the given istec type.
{
    // Construct the resource name
    String ResName = FormatStr ("@ICBASEED.Default-%u", IstecType);

    // Load the container with the default data
    Container* C = (Container*) LoadResource (ResName);

    // Get a pointer to the data. Cast to char*.
    const unsigned char* Data = (const unsigned char*) C->GetData ();

    // Copy the data for both config structs. Data is always in old format!
    Config.BaseConfig.Unpack (Data);
    Data += 93;
    for (unsigned I = 0; I < Config.GetDevCount (); I++) {
        Config.GetDevConfig (I).Unpack (Data, 1.7);
        Data += 17;
    }

    // Discard the container
    delete C;
}



static unsigned NewIstecType (WindowItem* Item)
// Get a new istec type
{
    // Load the menue
    Menue* M = (Menue*) LoadResource ("@ICBASEED.TypeMenue");

    // Place it near the item
    M->PlaceNear (Item);

    // Get a choice
    unsigned Sel = M->GetChoice ();

    // Delete the menue
    delete M;

    // Return the selection
    return Sel;
}



void EditBaseConfig (IstecConfig& Config, int IstecPresent, int& Changed)
// Allow editing the istec base configuration. If the istec is not present,
// the type of the istec can be changed, resulting in a load of the default
// for this istec type. If Config has been changed, Changed is set to 1,
// otherwise, Changed is left untouched.
{
    // ID's of menue items for EditBaseConfig
    const miIstecType           =  10;
    const miProtocol            =  20;
    const miExtLevel            =  30;
    const miMusic               =  40;
    const miConnection          =  50;
    const miNumber1             =  60;
    const miNumber2             =  70;
    const miTFEAssignment       =  80;
    const miQueryLoc            =  90;
    const miMSN                 = 100;
    const miMSNGroups           = 110;
    const miEAZGroups           = 120;
    const miExternalMusicPort   = 130;
    const miCountryCode         = 140;
    const miSignaling           = 150;

    // Values for the miMusic toggle item
    const musOff                = 0;
    const musInternal           = 1;
    const musExternal           = 2;



    // Name of the settings resource
    static const String StgPosName = "EditBaseConfig.BaseConfigMenue.Position";

    // Remember the configuration (save it into a memory stream). Remember
    // the old CRC.
    u32 OldCRC = GetCRC (Config);
    MemoryStream SaveStream;
    SaveStream << Config;
    SaveStream.Seek (0);

    // New status line
    PushStatusLine (siAbort | siSelectKeys | siChange | siAccept);

    // INitialize variables needed below
    int Done = 0;
    int NeedConfig = 1;
    unsigned IstecType = 0;     // Initialize it to make gcc happy
    LongItem* ExternalMusicPort = NULL;
    Menue* Win = NULL;

    // Loop...
    while (!Done) {

        if (NeedConfig) {

            // If there is a menue, store the current position into the
            // settings, then delete it
            if (Win != NULL) {
                // Save the current window position
                StgPutPoint (Win->OuterBounds ().A, StgPosName);
                delete Win;
            }

            // Reload the window from the resource
            if (FirmwareVersion < 1.93) {
                Win = (Menue*) LoadResource ("@ICBASEED.BaseConfigMenue-1.92");
            } if (FirmwareVersion < 2.00) {
                Win = (Menue*) LoadResource ("@ICBASEED.BaseConfigMenue-1.93");
            } else {
                Win = (Menue*) LoadResource ("@ICBASEED.BaseConfigMenue-2.00");
            }

            // If there is a stored window position, move the window to that
            // position
            Point Pos = StgGetPoint (StgPosName, Win->OuterBounds ().A);
            Win->MoveAbs (Pos);

            // Get the type of the istec
            IstecType = Config.IstecID ();

            // Transfer data into the menue
            Win->SetStringValue (miIstecType, GetIstecName (IstecType));
            Win->SetToggleValue (miProtocol, Config.GetProtocol () != pr1TR6);
            ExternalMusicPort = (LongItem*) Win->ItemWithID (miExternalMusicPort);
            if (ExternalMusicPort == NULL) {
                Win->SetToggleValue (miMusic, Config.GetMusic () != 0);
            } else {
                if (Config.GetMusic () != 0) {
                    Win->SetToggleValue (miMusic, musInternal);
                    Win->GrayItem (miExternalMusicPort);
                } else {
                    if (Config.GetMusicPort () == 0) {
                        Win->SetToggleValue (miMusic, musOff);
                        ExternalMusicPort->SetValue (21);
                        Win->GrayItem (miExternalMusicPort);
                    } else {
                        Win->SetToggleValue (miMusic, musExternal);
                        ExternalMusicPort->SetValue (Config.GetMusicPort () + 20);
                    }
                }
            }
            Win->SetToggleValue (miConnection, Config.GetConnection () != coPointToMulti);
            Win->SetStringValue (miNumber1, Config.GetNumber1 ());
            Win->SetStringValue (miNumber2, Config.GetNumber2 ());
            Win->SetLongValue   (miTFEAssignment, Config.GetTFEAssignment ());
            if (Win->ItemWithID (miCountryCode) != NULL) {
                Win->SetLongValue   (miCountryCode, Config.GetCountryCode ());
            }

            // Enable/disable menue items dependent on the system
            if (IstecPresent) {
                Win->GrayItem (miIstecType);
            }
            if (Config.GetProtocol () == pr1TR6) {
                Win->GrayItem (miMSN);
                Win->GrayItem (miMSNGroups);
            } else if (Config.GetProtocol () == prDSS1) {
                Win->GrayItem (miEAZGroups);
            }

            switch (IstecType) {

                case 1003:
                case 1008:
                    if (IstecPresent) {
                        // Protocol is hard wired in the istec
                        Win->GrayItem (miProtocol);
                    }
                    Win->GrayItem (miExtLevel);
                    if (Config.GetConnection () == coPointToMulti) {
                        Win->GrayItem (miNumber1);
                    }
                    Win->GrayItem (miNumber2);
                    Win->GrayItem (miQueryLoc);
                    break;

                case 1016:
                    Win->GrayItem (miConnection);
                    Win->GrayItem (miNumber2);
                    break;

                case 1024:
                    Win->GrayItem (miExtLevel);
                    Win->GrayItem (miConnection);
                    Win->GrayItem (miNumber2);
                    break;

                case 2016:
                    Win->GrayItem (miConnection);
                    break;

                case 2024:
                    Win->GrayItem (miExtLevel);
                    Win->GrayItem (miConnection);
                    break;

                case 2400:
                    Win->GrayItem (miExtLevel);
                    if (Config.GetConnection () == coPointToMulti) {
                        Win->GrayItem (miNumber1);
                        Win->GrayItem (miNumber2);
                    }
                    break;

                case 2416:
                    Win->GrayItem (miConnection);
                    break;

                case 2424:
                    Win->GrayItem (miExtLevel);
                    Win->GrayItem (miConnection);
                    break;

            }

            // Activate the menue
            Win->Activate ();

            // Config done
            NeedConfig = 0;

        }

        // Get a selection
        int Choice = Win->GetChoice ();

        //
        String Num;
        unsigned NewType;
        switch (Choice) {

            case miIstecType:
                NewType = NewIstecType (Win->ItemWithID (miIstecType));
                if (NewType != 0 && NewType != IstecType) {
                    // Got a new selection, load the default
                    NeedConfig = 1;
                    LoadConfigDefault (Config, NewType);
                }
                break;

            case miProtocol:
                Config.BaseConfig.Protocol = pr1TR6;
                Win->GrayItem (miMSN);
                Win->GrayItem (miMSNGroups);
                Win->ActivateItem (miEAZGroups);
                break;

            case miProtocol+1:
                Config.BaseConfig.Protocol = prDSS1;
                Win->ActivateItem (miMSN);
                Win->ActivateItem (miMSNGroups);
                Win->GrayItem (miEAZGroups);
                break;

            case miExtLevel:
                InformationMsg (LoadAppMsg (msExtLevel));
                break;

            case miMusic+musOff:
                Config.BaseConfig.Music = 0;
                Config.BaseConfig.MusicPort = 0;
                if (ExternalMusicPort != NULL) {
                    Win->GrayItem (miExternalMusicPort);
                }
                break;

            case miMusic+musInternal:
                Config.BaseConfig.Music = 1;
                if (ExternalMusicPort != NULL) {
                    Win->GrayItem (miExternalMusicPort);
                }
                break;

            case miMusic+musExternal:
                Config.BaseConfig.Music = 0;
                Config.BaseConfig.MusicPort = Win->GetLongValue (miExternalMusicPort) - 20;
                Win->ActivateItem (miExternalMusicPort);
                break;

            case miConnection:
                Config.BaseConfig.Connection = coPointToMulti;
                Win->GrayItem (miNumber1);
                Win->GrayItem (miNumber1);
                break;

            case miConnection+1:
                Config.BaseConfig.Connection = coPointToPoint;
                Win->ActivateItem (miNumber1);
                if (Config.GetExtS0 () > 1) {
                    Win->ActivateItem (miNumber2);
                }
                break;

            case miNumber1:
                Config.SetNumber1 (Win->GetStringValue (miNumber1));
                break;

            case miNumber2:
                Config.SetNumber2 (Win->GetStringValue (miNumber2));
                break;

            case miTFEAssignment:
                Config.BaseConfig.TFEAssignment = Win->GetLongValue (miTFEAssignment);
                break;

            case miQueryLoc:
                EditQueryLoc (Config.BaseConfig);
                break;

            case miMSN:
                EditMSN (Config.BaseConfig);
                break;

            case miMSNGroups:
                EditMSNGroups (Config.BaseConfig, "@ICBASEED.MSN-GruppenMenue");
                break;

            case miEAZGroups:
                EditMSNGroups (Config.BaseConfig, "@ICBASEED.EAZ-GruppenMenue");
                break;

            case miExternalMusicPort:
                Config.BaseConfig.MusicPort = Win->GetLongValue (miExternalMusicPort) - 20;
                break;

            case miCountryCode:
                Config.BaseConfig.CountryCode = Win->GetLongValue (miCountryCode);
                break;

            case miSignaling:
                {
                    Point Pos = Win->ForcedItemWithID (miSignaling)->Pos ();
                    Win->Absolute (Pos);
                    EditSignaling (Config.BaseConfig, Pos);
                }
                break;

            case 0:
                if (Win->GetAbortKey () == vkAbort) {
                    // Abort - ask if we have changes
                    if (GetCRC (Config) != OldCRC) {
                        // We have changes
                        if (AskDiscardChanges () == 2) {
                            // Discard changes, reload the data from the stream
                            SaveStream >> Config;
                            Done = 1;
                        }
                    } else {
                        // No changes
                        Done = 1;
                    }
                } else if (Win->GetAbortKey () == vkAccept) {
                    // Accept the changes, set the Changed flag
                    if (GetCRC (Config) != OldCRC) {
                        // We have changes
                        Changed = 1;
                    }
                    Done = 1;
                }
                break;

        }

    }

    // Delete the menue, restore old statusline
    delete Win;
    PopStatusLine ();

}


