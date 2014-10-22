/*****************************************************************************/
/*                                                                           */
/*                                 ICDEVS.CC                                 */
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



#include "listbox.h"
#include "progutil.h"
#include "menue.h"
#include "menuitem.h"
#include "statline.h"
#include "strcvt.h"
#include "stdmsg.h"
#include "stdmenue.h"
#include "settings.h"
#include "crcstrm.h"
#include "memstrm.h"

#include "icmsg.h"
#include "icconfig.h"



/*****************************************************************************/
/*                             Message constants                             */
/*****************************************************************************/



const u16 msKeine                       = MSGBASE_ICDEVS +  0;
const u16 msInland                      = MSGBASE_ICDEVS +  1;
const u16 msOrt                         = MSGBASE_ICDEVS +  2;
const u16 msHalbamt                     = MSGBASE_ICDEVS +  3;
const u16 msNichtamt                    = MSGBASE_ICDEVS +  4;
const u16 msFernsprechen                = MSGBASE_ICDEVS +  5;
const u16 msFaxG3                       = MSGBASE_ICDEVS +  6;
const u16 msDatenModem                  = MSGBASE_ICDEVS +  7;
const u16 msDatexJModem                 = MSGBASE_ICDEVS +  8;
const u16 msAnrufbeantworter            = MSGBASE_ICDEVS +  9;
const u16 msKombiDienste                = MSGBASE_ICDEVS + 10;
const u16 msOn                          = MSGBASE_ICDEVS + 11;
const u16 msOff                         = MSGBASE_ICDEVS + 12;
const u16 msNoReroute                   = MSGBASE_ICDEVS + 13;
const u16 msExtReroute                  = MSGBASE_ICDEVS + 14;
const u16 msIntReroute                  = MSGBASE_ICDEVS + 15;
const u16 msDeviceHeader                = MSGBASE_ICDEVS + 16;



/*****************************************************************************/
/*                      Explicit template instantiation                      */
/*****************************************************************************/



#ifdef EXPLICIT_TEMPLATES
template class ListBox<IstecDevConfig>;
#endif



/*****************************************************************************/
/*                             class DevListBox                              */
/*****************************************************************************/



class DevListBox: public ListBox<IstecDevConfig> {


private:
    static const String& DialCapsName (unsigned DialCaps);
    // Map the number of a dial capability to a name of fixed length

    static const String& ServiceName (unsigned Service);
    // Map the number of a service to a name of fixed length

    static String RerouteName (unsigned Val, const String& Num);
    // Map the reroute capability to a string with fixed length

    static const String& BoolName (unsigned B);
    // Map a boolean flag to a string with fixed length


protected:
    virtual void Print (int Index, int X, int Y, u16 Attr);
    // Display one of the listbox entries

public:
    DevListBox (i16 aID, const Point& aSize, WindowItem* NextItem = NULL);

};



DevListBox::DevListBox (i16 aID, const Point& aSize, WindowItem* NextItem):
    ListBox <IstecDevConfig> ("", aID, aSize, atEditNormal, atEditBar, atEditHigh, NextItem)
{
}



const String& DevListBox::DialCapsName (unsigned DialCaps)
// Map the number of a dial capability to a name of fixed length
{
    unsigned MsgNum = msKeine;
    switch (DialCaps) {
        case dcKeine:       MsgNum = msKeine;           break;
        case dcInland:      MsgNum = msInland;          break;
        case dcOrt:         MsgNum = msOrt;             break;
        case dcHalbamt:     MsgNum = msHalbamt;         break;
        case dcNichtamt:    MsgNum = msNichtamt;        break;
    }
    return LoadAppMsg (MsgNum);
}



const String& DevListBox::ServiceName (unsigned Service)
// Map the number of a service to a name of fixed length
{
    unsigned MsgNum = msFernsprechen;
    switch (Service) {
        case svFernsprechen:        MsgNum = msFernsprechen;        break;
        case svFaxG3:               MsgNum = msFaxG3;               break;
        case svDatenModem:          MsgNum = msDatenModem;          break;
        case svDatexJModem:         MsgNum = msDatexJModem;         break;
        case svAnrufbeantworter:    MsgNum = msAnrufbeantworter;    break;
        case svKombi:               MsgNum = msKombiDienste;        break;
    }
    return LoadAppMsg (MsgNum);
}



String DevListBox::RerouteName (unsigned Val, const String& Num)
// Map the reroute capability to a string with fixed length
{
    const StringLength = 11;
    const PadLength = 12;
    String Res (PadLength);

    if (Val == 0x00) {
        // No reroute
        Res = LoadAppMsg (msNoReroute);
    } else if (Val == 0x01) {
        // External reroute - check the length
        Res = Num;
        if (Res.Len () > StringLength) {
            // Too long for display
            Res = LoadAppMsg (msExtReroute);
        }
    } else {
        Res = FormatStr (LoadAppMsg (msIntReroute).GetStr (), Val);
    }

    // Pad to length
    Res.Pad (String::Right, PadLength);

    // Return the result
    return Res;
}



const String& DevListBox::BoolName (unsigned B)
// Map a boolean flag to a string with fixed length
{
    return LoadAppMsg (B ? msOn : msOff);
}



void DevListBox::Print (int Index, int X, int Y, u16 Attr)
{
    // Get the entry
    IstecDevConfig* Info = Coll->At (Index);

    // Create a string with the correct length
    String Line (Size.X);

    // Build the line
    String PIN = Info->GetPIN ();
    PIN.Pad (String::Left, 4);
    Line = FormatStr (" %2u   ", Info->DevNum + 21);
    Line += DialCapsName (Info->DialCaps);
    Line += ServiceName (Info->Service);
    Line += RerouteName (Info->Reroute, Info->GetExtNum ());
    Line += BoolName (Info->ChargePulse);
    Line += PIN;

    // Pad the line
    Line.Pad (String::Right, Size.X - 1);
    Line += ' ';

    // Print the name
    Owner->Write (X, Y, Line, Attr);
}



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



static void EditDevConfig (IstecDevConfig& Config, unsigned DevCount)
// Edit the given device configuration.
{
    // Settings name for the window position
    static const String StgPosName = "EditDevConfig.ConfigMenue.Position";

    // Menue constants
    const miDialCaps      = 10;
    const miService       = 20;
    const miReroute       = 30;
    const miChargePulse   = 40;
    const miPIN           = 50;
    const miTerminalMode  = 60;
    const miKnockInt      = 70;
    const miKnockExt      = 80;
    const miKnockTFE      = 90;
    const miKnockInt21    = 1000;
    const miKnockInt22    = 1010;
    const miKnockInt23    = 1020;
    const miKnockInt24    = 1030;
    const miKnockInt25    = 1040;
    const miKnockInt26    = 1050;
    const miKnockInt27    = 1060;
    const miKnockInt28    = 1070;
    const miKnockMSN0     = 1100;
    const miKnockMSN1     = 1110;
    const miKnockMSN2     = 1120;
    const miKnockMSN3     = 1130;
    const miKnockMSN4     = 1140;
    const miKnockMSN5     = 1150;
    const miKnockMSN6     = 1160;
    const miKnockMSN7     = 1170;
    const miKnockMSN8     = 1180;
    const miKnockMSN9     = 1190;
    const miKnockTFE1     = 1200;
    const miKnockTFE2     = 1210;
    const miKnockTFE3     = 1220;
    const miKnockTFE4     = 1230;


    // Save the configuration into a memory stream, remember the CRC
    u32 OldCRC = GetCRC (Config);
    MemoryStream SaveStream;
    SaveStream << Config;
    SaveStream.Seek (0);

    // Load the menue
    Menue* M;
    if (FirmwareVersion < 1.92) {
        M = (Menue*) LoadResource ("@ICDEVS.ConfigMenue-1.90");
    } else if (FirmwareVersion < 1.93) {
        M = (Menue*) LoadResource ("@ICDEVS.ConfigMenue-1.92");
    } else if (FirmwareVersion < 1.95) {
        M = (Menue*) LoadResource ("@ICDEVS.ConfigMenue-1.93");
    } else {
        M = (Menue*) LoadResource ("@ICDEVS.ConfigMenue-2.00");
    }

    // If there is a stored window position, move the window to that position
    Point Pos = StgGetPoint (StgPosName, M->OuterBounds ().A);
    M->MoveAbs (Pos);

    // Set the menue header to show the device number
    M->SetHeader (FormatStr (LoadAppMsg (msDeviceHeader).GetStr (), Config.DevNum + 21));

    // Insert the values into the menue
    M->SetToggleValue (miDialCaps, Config.DialCaps);
    M->SetToggleValue (miService, Config.Service);
    M->SetToggleValue (miChargePulse, Config.ChargePulse);
    M->SetStringValue (miPIN, Config.GetPIN ());
    if (Config.Reroute == 0x01) {
        // External
        M->SetStringValue (miReroute, Config.GetExtNum ());
    } else if (Config.Reroute != 0x00) {
        // Internal
        M->SetStringValue (miReroute, U32Str (Config.Reroute));
    }
    if (M->ItemWithID (miTerminalMode) != NULL) {
        M->SetToggleValue (miTerminalMode, Config.TerminalMode);
    }
    if (M->ItemWithID (miKnockInt) != 0) {
        M->SetToggleValue (miKnockInt21, Config.GetIntKnock (knInt21));
        M->SetToggleValue (miKnockInt22, Config.GetIntKnock (knInt22));
        M->SetToggleValue (miKnockInt23, Config.GetIntKnock (knInt23));
        M->SetToggleValue (miKnockInt24, Config.GetIntKnock (knInt24));
        M->SetToggleValue (miKnockInt25, Config.GetIntKnock (knInt25));
        M->SetToggleValue (miKnockInt26, Config.GetIntKnock (knInt26));
        M->SetToggleValue (miKnockInt27, Config.GetIntKnock (knInt27));
        M->SetToggleValue (miKnockInt28, Config.GetIntKnock (knInt28));
    }
    if (M->ItemWithID (miKnockExt) != 0) {
        M->SetToggleValue (miKnockMSN0, Config.GetExtKnock (knMSN0));
        M->SetToggleValue (miKnockMSN1, Config.GetExtKnock (knMSN1));
        M->SetToggleValue (miKnockMSN2, Config.GetExtKnock (knMSN2));
        M->SetToggleValue (miKnockMSN3, Config.GetExtKnock (knMSN3));
        M->SetToggleValue (miKnockMSN4, Config.GetExtKnock (knMSN4));
        M->SetToggleValue (miKnockMSN5, Config.GetExtKnock (knMSN5));
        M->SetToggleValue (miKnockMSN6, Config.GetExtKnock (knMSN6));
        M->SetToggleValue (miKnockMSN7, Config.GetExtKnock (knMSN7));
        M->SetToggleValue (miKnockMSN8, Config.GetExtKnock (knMSN8));
        M->SetToggleValue (miKnockMSN9, Config.GetExtKnock (knMSN9));
    }
    if (M->ItemWithID (miKnockTFE) != 0) {
        M->SetToggleValue (miKnockTFE1, Config.GetExtKnock (knTFE1));
        M->SetToggleValue (miKnockTFE2, Config.GetExtKnock (knTFE2));
        M->SetToggleValue (miKnockTFE3, Config.GetExtKnock (knTFE3));
        M->SetToggleValue (miKnockTFE4, Config.GetExtKnock (knTFE4));
    }

    // Display a new status line and activate the menue
    PushStatusLine (siAbort | siAccept);
    M->Activate ();

    // Allow editing
    int Done = 0;
    String Num;
    while (!Done) {

        // Get a choice from the user
        int Choice = M->GetChoice ();

        // Look what he wants...
        switch (Choice) {

            case miDialCaps + dcKeine:
            case miDialCaps + dcInland:
            case miDialCaps + dcOrt:
            case miDialCaps + dcHalbamt:
            case miDialCaps + dcNichtamt:
                Config.DialCaps = Choice - miDialCaps;
                break;

            case miService + svFernsprechen:
            case miService + svFaxG3:
            case miService + svDatenModem:
            case miService + svDatexJModem:
            case miService + svAnrufbeantworter:
            case miService + svKombi:
                Config.Service = Choice - miService;
                break;

            case miReroute:
                Num = M->GetStringValue (miReroute);
                if (Num.IsEmpty ()) {
                    Config.Reroute = 0x00;
                    Config.ClearExtNum ();
                } else {
                    // Num does contain nothing than digits, so using atoi is
                    // save. Since the number is also range checked, I'll use
                    // an unsigned here
                    u32 Number = atoi (Num.GetStr ());
                    if (Num.Len () == 2 && Number >= 21 && Number < 21 + DevCount) {
                        // Internal
                        Config.Reroute = Number;
                        Config.ClearExtNum ();
                    } else {
                        // External
                        Config.Reroute = 0x01;
                        Config.SetExtNum (Num);
                    }
                }
                break;

            case miChargePulse+0:
            case miChargePulse+1:
                Config.ChargePulse = Choice - miChargePulse;
                break;

            case miPIN:
                Config.SetPIN (M->GetStringValue (miPIN));
                break;

            case miTerminalMode:
                Config.TerminalMode = 0;
                break;

            case miTerminalMode+1:
                Config.TerminalMode = 1;
                break;

            case miKnockInt21:
                Config.ClrIntKnock (knInt21);
                break;

            case miKnockInt21+1:
                Config.SetIntKnock (knInt21);
                break;

            case miKnockInt22:
                Config.ClrIntKnock (knInt22);
                break;

            case miKnockInt22+1:
                Config.SetIntKnock (knInt22);
                break;

            case miKnockInt23:
                Config.ClrIntKnock (knInt23);
                break;

            case miKnockInt23+1:
                Config.SetIntKnock (knInt23);
                break;

            case miKnockInt24:
                Config.ClrIntKnock (knInt24);
                break;

            case miKnockInt24+1:
                Config.SetIntKnock (knInt24);
                break;

            case miKnockInt25:
                Config.ClrIntKnock (knInt25);
                break;

            case miKnockInt25+1:
                Config.SetIntKnock (knInt25);
                break;

            case miKnockInt26:
                Config.ClrIntKnock (knInt26);
                break;

            case miKnockInt26+1:
                Config.SetIntKnock (knInt26);
                break;

            case miKnockInt27:
                Config.ClrIntKnock (knInt27);
                break;

            case miKnockInt27+1:
                Config.SetIntKnock (knInt27);
                break;

            case miKnockInt28:
                Config.ClrIntKnock (knInt28);
                break;

            case miKnockInt28+1:
                Config.SetIntKnock (knInt28);
                break;

            case miKnockMSN0:
                Config.ClrExtKnock (knMSN0);
                break;

            case miKnockMSN0+1:
                Config.SetExtKnock (knMSN0);
                break;

            case miKnockMSN1:
                Config.ClrExtKnock (knMSN1);
                break;

            case miKnockMSN1+1:
                Config.SetExtKnock (knMSN1);
                break;

            case miKnockMSN2:
                Config.ClrExtKnock (knMSN2);
                break;

            case miKnockMSN2+1:
                Config.SetExtKnock (knMSN2);
                break;

            case miKnockMSN3:
                Config.ClrExtKnock (knMSN3);
                break;

            case miKnockMSN3+1:
                Config.SetExtKnock (knMSN3);
                break;

            case miKnockMSN4:
                Config.ClrExtKnock (knMSN4);
                break;

            case miKnockMSN4+1:
                Config.SetExtKnock (knMSN4);
                break;

            case miKnockMSN5:
                Config.ClrExtKnock (knMSN5);
                break;

            case miKnockMSN5+1:
                Config.SetExtKnock (knMSN5);
                break;

            case miKnockMSN6:
                Config.ClrExtKnock (knMSN6);
                break;

            case miKnockMSN6+1:
                Config.SetExtKnock (knMSN6);
                break;

            case miKnockMSN7:
                Config.ClrExtKnock (knMSN7);
                break;

            case miKnockMSN7+1:
                Config.SetExtKnock (knMSN7);
                break;

            case miKnockMSN8:
                Config.ClrExtKnock (knMSN8);
                break;

            case miKnockMSN8+1:
                Config.SetExtKnock (knMSN8);
                break;

            case miKnockMSN9:
                Config.ClrExtKnock (knMSN9);
                break;

            case miKnockMSN9+1:
                Config.SetExtKnock (knMSN9);
                break;

            case miKnockTFE1:
                Config.ClrExtKnock (knTFE1);
                break;

            case miKnockTFE1+1:
                Config.SetExtKnock (knTFE1);
                break;

            case miKnockTFE2:
                Config.ClrExtKnock (knTFE2);
                break;

            case miKnockTFE2+1:
                Config.SetExtKnock (knTFE2);
                break;

            case miKnockTFE3:
                Config.ClrExtKnock (knTFE3);
                break;

            case miKnockTFE3+1:
                Config.SetExtKnock (knTFE3);
                break;

            case miKnockTFE4:
                Config.ClrExtKnock (knTFE4);
                break;

            case miKnockTFE4+1:
                Config.SetExtKnock (knTFE4);
                break;

            case 0:
                if (M->GetAbortKey () == vkAbort) {
                    // Abort - ask if we have changes
                    if (GetCRC (Config) != OldCRC) {
                        // We have changes
                        if (AskDiscardChanges () == 2) {
                            // Discard changes, reload from stream
                            SaveStream >> Config;
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

    // Save the current window position
    StgPutPoint (M->OuterBounds ().A, StgPosName);

    // Pop the status line, delete the menue
    PopStatusLine ();
    delete M;

}



void DeviceList (IstecConfig& Config, int& Changed)
// List all devices and there settings, including the charges. Allow editing
// the settings and charges.
{
    static const String StgPosName = "DeviceList.DeviceListWindow.Position";

    // Save the current config into a memory stream in case we want to cancel
    // changes. Calculate the CRC.
    u32 OldCRC = GetCRC (Config);
    MemoryStream SaveStream;
    SaveStream << Config;
    SaveStream.Seek (0);

    // Load the window
    Menue* Win = (Menue*) LoadResource ("@ICDEVS.DeviceListWindow");

    // If there is a stored window position, move the window to that position
    Point Pos = StgGetPoint (StgPosName, Win->OuterBounds ().A);
    Win->MoveAbs (Pos);

    // Create a listbox inside the window
    Point Size;
    Size.X = Win->IXSize ();
    Size.Y = Win->IYSize () - 2;
    DevListBox* Box = new DevListBox (1, Size);
    Box->SetColl (&Config.DevColl);
    Win->AddItem (Box);
    Box->SetPos (0, 2);
    Box->Select ();
    Box->Draw ();
    Win->Activate ();

    // New status line
    PushStatusLine (siAbort | siSelectKeys | siChange | siAccept);

    // Allow choosing an entry
    int Done = 0;
    while (!Done) {

        // Get keyboard input
        Key K = ::KbdGet ();

        // Let the box look for a useful key
        Box->HandleKey (K);

        // Look what's left
        int Selected;
        switch (K) {

            case kbEnter:
                Selected = Box->GetSelected ();
                if (Selected != -1) {
                    EditDevConfig (*Config.DevColl [Selected], Config.GetDevCount ());
                    Box->Draw ();
                }
                break;

            case vkAccept:
                // If we had changes, tell the caller
                if (GetCRC (Config) != OldCRC) {
                    Changed = 1;
                }
                Done = 1;
                break;

            case vkResize:
                Win->MoveResize ();
                break;

            case vkAbort:
                if (GetCRC (Config) != OldCRC) {
                    // We have changes - ask if we should discard them
                    if (AskDiscardChanges () == 2) {
                        // Discard the changes. To do that, reload the old
                        // config data from the memory stream.
                        SaveStream >> Config;
                        Done = 1;
                    }
                } else {
                    Done = 1;
                }
                break;

        }

    }

    // Restore the status line
    PopStatusLine ();

    // Set a new collection for the listbox (otherwise the box would try to
    // delete the collection)
    Box->SetColl (NULL);

    // Save the current window position
    StgPutPoint (Win->OuterBounds ().A, StgPosName);

    // Delete the window
    delete Win;

}



