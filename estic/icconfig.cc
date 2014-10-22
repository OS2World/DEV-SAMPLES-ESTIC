/*****************************************************************************/
/*                                                                           */
/*                                ICCONFIG.CC                                */
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



#include <string.h>

#include "check.h"
#include "str.h"

#include "icobjid.h"
#include "icerror.h"
#include "icdlog.h"
#include "icconfig.h"



// Register the classes
LINK (IstecBaseConfig, ID_IstecBaseConfig);
LINK (IstecDevConfig, ID_IstecDevConfig);
LINK (IstecDevColl, ID_IstecDevColl);
LINK (IstecConfig, ID_IstecConfig);






/*****************************************************************************/
/*                      Explicit template instantiation                      */
/*****************************************************************************/



#ifdef EXPLICIT_TEMPLATES
template class SortedCollection<IstecDevConfig, unsigned char>;
template class Collection<IstecDevConfig>;
#endif



/*****************************************************************************/
/*                           class IstecBaseConfig                           */
/*****************************************************************************/



IstecBaseConfig::IstecBaseConfig ():
    Connection (coPointToMulti),
    DevCount (8),
    Protocol (prDSS1),
    VersionHigh (1),
    VersionLow (70),
    TFEAssignment (21),
    Music (0),
    IntS0 (0),
    ExtS0 (1),
    QueryLoc1 (0),
    QueryLoc2 (0),
    MusicPort (0),
    CountryCode (49),
    AlarmTone (0)
// Constructor
{
    memset (MSN, 0, sizeof (MSN));
    memset (Number1, 0, sizeof (Number1));
    memset (Number2, 0, sizeof (Number2));
    memset (MSNGroups, 0xFF, sizeof (MSNGroups));
    memset (TFELoc, 0, sizeof (TFELoc));
    memset (Signaling, 0, sizeof (Signaling));
    memset (Reserved, 0, sizeof (Reserved));
}



void IstecBaseConfig::Load (Stream& S)
// Load the object from a stream
{
    if (S.GetStatus () != stOk) {
        // Stream has an error
        return;
    }

    S >> Connection >> DevCount >> Protocol >> VersionHigh >> VersionLow;
    S >> TFEAssignment;
    S.Read (MSN, sizeof (MSN));
    S >> Music >> IntS0 >> ExtS0 >> QueryLoc1 >> QueryLoc2;
    S.Read (Number1, sizeof (Number1));
    S.Read (Number2, sizeof (Number2));
    S.Read (MSNGroups, sizeof (MSNGroups));
    S >> MusicPort >> CountryCode;
    S.Read (TFELoc, sizeof (TFELoc));
    S >> AlarmTone;
    S.Read (Signaling, sizeof (Signaling));
    S.Read (Reserved, sizeof (Reserved));
}



void IstecBaseConfig::Store (Stream& S) const
// Store the object into a stream
{
    if (S.GetStatus () != stOk) {
        // Stream has an error
        return;
    }

    S << Connection << DevCount << Protocol << VersionHigh << VersionLow;
    S << TFEAssignment;
    S.Write (MSN, sizeof (MSN));
    S << Music << IntS0 << ExtS0 << QueryLoc1 << QueryLoc2;
    S.Write (Number1, sizeof (Number1));
    S.Write (Number2, sizeof (Number2));
    S.Write (MSNGroups, sizeof (MSNGroups));
    S << MusicPort << CountryCode;
    S.Write (TFELoc, sizeof (TFELoc));
    S << AlarmTone;
    S.Write (Signaling, sizeof (Signaling));
    S.Write (Reserved, sizeof (Reserved));
}



u16 IstecBaseConfig::StreamableID () const
// Return the stream ID
{
    return ID_IstecBaseConfig;
}



Streamable* IstecBaseConfig::Build ()
// Return a new instance
{
    return new IstecBaseConfig (Empty);
}



unsigned char* IstecBaseConfig::Pack (unsigned char* Buf) const
// Pack the data of struct IstecBaseConfig into an array ready for
// transmission. The function returns Buf.
{
    unsigned I;
    unsigned char* B = Buf;

    // Clear the complete buffer
    memset (B, 0, BaseConfigSize);

    // Copy the values
    *B++ = Connection;
    *B++ = DevCount;
    *B++ = Protocol;
    *B++ = VersionHigh;
    *B++ = VersionLow;
    *B++ = TFEAssignment;
    for (I = 0; I < bcMSNCount; I++) {
        memcpy (B, MSN [I], sizeof (MSN [I]));
        B += sizeof (MSN [I]);
    }
    *B++ = Music;
    *B++ = IntS0;
    *B++ = ExtS0;
    *B++ = QueryLoc1;
    *B++ = QueryLoc2;
    memcpy (B, Number1, sizeof (Number1));
    B += sizeof (Number1);
    memcpy (B, Number2, sizeof (Number2));
    B += sizeof (Number2);
    memcpy (B, MSNGroups, sizeof (MSNGroups));
    B += sizeof (MSNGroups);

    // Version 1.93 extensions follow.
    unsigned Version = unsigned (VersionHigh) * 100 + unsigned (VersionLow);
    if (Version >= 193) {

        *B++            = MusicPort;
        *B++            = (unsigned char) (CountryCode & 0x00FF);
        *B++            = (unsigned char) ((CountryCode >> 8) & 0x00FF);
        memcpy (B, TFELoc, sizeof (TFELoc));
        B += sizeof (TFELoc);

        if (Version >= 200) {
            *B++        = AlarmTone;
            for (I = 0; I < bcMSNCount; I++) {
                *B++ = OutSignal (Signaling [I]);
            }
        }

    }

    // Return the prepared buffer
    return Buf;
}



IstecBaseConfig& IstecBaseConfig::Unpack (const unsigned char* Buf)
// Unpack an array of char that contains data for an IstecBaseConfig struct.
// The function returns this.
{
    unsigned I;

    if ((Connection = *Buf++) > coMax) {
        WriteDebugLog (FormatStr ("IstecBaseConfig::Unpack: Got invalid value for connection: %02X",
                                  Connection));
        Connection = coPointToMulti;
    }
    DevCount = *Buf++;
    if ((Protocol = *Buf++) > prMax) {
        WriteDebugLog (FormatStr ("IstecBaseConfig::Unpack: Got invalid value for protocol: %02X",
                                  Protocol));
        Protocol = prDSS1;
    }
    VersionHigh   = *Buf++;
    VersionLow    = *Buf++;
    TFEAssignment = *Buf++;
    if ((TFEAssignment != 0) &&
        (TFEAssignment < 21 || TFEAssignment >= 21 + DevCount)) {
        WriteDebugLog (FormatStr ("IstecBaseConfig::Unpack: Got invalid value for TFE assignment: %02X",
                                  TFEAssignment));
        TFEAssignment = 21;
    }
    for (I = 0; I < bcMSNCount; I++) {
        memcpy (MSN [I], Buf, sizeof (MSN [I]));
        Buf += sizeof (MSN [I]);
    }
    Music             = (*Buf++ != 0);
    IntS0             = *Buf++;
    ExtS0             = *Buf++;
    QueryLoc1         = *Buf++;
    QueryLoc2         = *Buf++;
    memcpy (Number1, Buf, sizeof (Number1));
    Buf += sizeof (Number1);
    memcpy (Number2, Buf, sizeof (Number2));
    Buf += sizeof (Number2);
    memcpy (MSNGroups, Buf, sizeof (MSNGroups));
    Buf += sizeof (MSNGroups);

    // Version 1.93 extensions follow
    unsigned Version = unsigned (VersionHigh) * 100 + unsigned (VersionLow);
    if (Version >= 193) {

        MusicPort   = *Buf++;
        CountryCode = unsigned (Buf [1]) * 0x100 + unsigned (Buf [0]);
        Buf += 2;
        memcpy (TFELoc, Buf, sizeof (TFELoc));
        Buf += sizeof (TFELoc);

        if (Version >= 200) {
            AlarmTone = *Buf++;
            for (I = 0; I < bcMSNCount; I++) {
                Signaling [I] = InSignal (*Buf++);
            }
            memcpy (Reserved, Buf, sizeof (Reserved));
        } else {
            // Use defaults for the additional fields
            AlarmTone = 0;
            for (I = 0; I < bcMSNCount; I++) {
                Signaling [I] = siStandard;
            }
            memset (Reserved, 0, sizeof (Reserved));
        }

    } else {

        // This is an old version, clear out the additional fields
        MusicPort   = 0;            // None
        CountryCode = 49;           // Germany
        memset (TFELoc, 0, sizeof (TFELoc));
        AlarmTone   = 0;            // Off
        for (I = 0; I < bcMSNCount; I++) {
            Signaling [I] = siStandard;
        }
        memset (Reserved, 0, sizeof (Reserved));
    }

    // Return the prepared struct
    return *this;
}



IstecBaseConfig& IstecBaseConfig::Unpack (const IstecMsg& Msg)
// Unpack an array of char that contains data for an IstecBaseConfig struct.
// The function returns this.
{
    PRECONDITION (Msg.Size >= 94 && Msg.At (0) == 0x17);
    return Unpack (&Msg.Data [1]);
}



int operator == (const IstecBaseConfig& lhs, const IstecBaseConfig& rhs)
{
    // For simplicity, convert to an array, then do a memcmp
    unsigned char Left [BaseConfigSize], Right [BaseConfigSize];
    return memcmp (lhs.Pack (Left), rhs.Pack (Right), BaseConfigSize) == 0;
}



int operator != (const IstecBaseConfig& lhs, const IstecBaseConfig& rhs)
{
    // For simplicity, convert to an array, then do a memcmp
    unsigned char Left [BaseConfigSize], Right [BaseConfigSize];
    return memcmp (lhs.Pack (Left), rhs.Pack (Right), BaseConfigSize) != 0;
}



unsigned IstecBaseConfig::IstecID () const
// Return the type of the istec, determined by the parameters of the base
// configuration. The return value is 0 if the istec type could not be
// identified, 1008 for an istec 1008 etc.
{
    if (ExtS0 == 1) {
        // It is a 10XX type
        switch (DevCount) {
            case 3:     return 1003;
            case 8:     return 1008;
            case 16:    return 1016;
            case 24:    return 1024;
        }
    } else if (ExtS0 == 2) {
        // Type 20XX or 24XX
        if (IntS0 == 0) {
            // It is a 20XX type
            switch (DevCount) {
                case 16:    return 2016;
                case 24:    return 2024;
            }
        } else if (IntS0 == 4) {
            // It is a 24XX type
            switch (DevCount) {
                case 0:     return 2400;
                case 16:    return 2416;
                case 24:    return 2424;
            }
        }
    }

    // Unknown type if it comes here...
    return 0;
}



/*****************************************************************************/
/*                           class IstecDevConfig                            */
/*****************************************************************************/



IstecDevConfig::IstecDevConfig (unsigned char aDevNum):
    DevNum (aDevNum),
    DialCaps (dcKeine),
    Service (svFernsprechen),
    Reroute (0),
    ChargePulse (0),
    TerminalMode (0),
    InternalKnock (0),
    ExternalKnock (0),
    ExtNumLen (0),
    RerouteIfBusy (0),
    RerouteIfVoid (0),
    RingsUntilReroute (0)
// Create an IstecDevConfig
{
    memset (PIN, 0xFF, sizeof (PIN));
    ClearExtNum ();
}



void IstecDevConfig::Load (Stream& S)
// Load the object from a stream
{
    if (S.GetStatus () != stOk) {
        // Stream error
        return;
    }

    S >> DevNum >> DialCaps >> Service >> Reroute >> ChargePulse;
    S.Read (PIN, sizeof (PIN));
    S >> ExtNum >> TerminalMode >> InternalKnock >> ExternalKnock;
    S >> ExtNumLen;
    S >> RerouteIfBusy >> RerouteIfVoid >> RingsUntilReroute;
}



void IstecDevConfig::Store (Stream& S) const
// Store the object into a stream
{
    if (S.GetStatus () != stOk) {
        // Stream error
        return;
    }

    S << DevNum << DialCaps << Service << Reroute << ChargePulse;
    S.Write (PIN, sizeof (PIN));
    S << ExtNum << TerminalMode << InternalKnock << ExternalKnock;
    S << ExtNumLen;
    S << RerouteIfBusy << RerouteIfVoid << RingsUntilReroute;
}



u16 IstecDevConfig::StreamableID () const
// Return the stream ID
{
    return ID_IstecDevConfig;
}



Streamable* IstecDevConfig::Build ()
// Return a new instance
{
    return new IstecDevConfig (Empty);
}



String IstecDevConfig::GetPIN () const
// Get the PIN as a string
{
    return FromBCD (PIN, sizeof (PIN));
}



void IstecDevConfig::SetPIN (const String& aPIN)
// Set the PIN from a string
{
    ToBCD (aPIN.GetStr (), PIN, sizeof (PIN));
}



unsigned char* IstecDevConfig::Pack (unsigned char* Buf, double FirmwareVersion) const
// Pack the data of struct IstecDevConfig into an array ready for transmission.
// The function returns Buf.
{
    // Clear the complete buffer
    memset (Buf, 0, DevConfigSize);


    unsigned char* B = Buf;

    *B++ = DevNum;
    *B++ = DialCaps;
    *B++ = Service;
    *B++ = Reroute;
    *B++ = ChargePulse;
    *B++ = PIN [0];
    *B++ = PIN [1];
    ToBCD (ExtNum.GetStr (), B, 10);
    B += 10;

    // Version 1.93+1.94 extensions follow
    if (FirmwareVersion > 1.92) {

        // This is a new version config record
        *B++ = TerminalMode;

        if (FirmwareVersion > 1.93) {
            *B++ = InternalKnock;
            *B++ = 0;   // Fill byte
            *B++ = ExternalKnock & 0xFF;
            *B++ = (ExternalKnock >> 8) & 0xFF;
            if (FirmwareVersion > 1.95) {
                *B++ = ExtNum.Len ();
                *B++ = RerouteIfBusy;
                *B++ = RerouteIfVoid;
                *B++ = RingsUntilReroute;
            }
        }
    }

    // Return the prepared buffer
    return Buf;
}



IstecDevConfig& IstecDevConfig::Unpack (const unsigned char* Buf,
                                        double FirmwareVersion)
// Unpack an array of char that contains data for a IstecDevConfig struct.
// The function returns *this and corrects invalid raw values.
{
    unsigned svMax = FirmwareVersion < 1.92? svAnrufbeantworter : svKombi;

    DevNum = *Buf++;
    if ((DialCaps = *Buf++) > dcMax) {
        WriteDebugLog (FormatStr ("IstecDevConfig::Unpack:: Got invalid value for DialCaps: %02X",
                                  DialCaps));
        DialCaps = dcKeine;
    }
    if ((Service = *Buf++) > svMax) {
        WriteDebugLog (FormatStr ("IstecDevConfig::Unpack:: Got invalid value for Service: %02X",
                                  Service));
        Service = svFernsprechen;
    }
    Reroute         = *Buf++ != 0;
    ChargePulse     = (*Buf++ != 0);
    PIN [0]         = *Buf++;
    PIN [1]         = *Buf++;
    ExtNum = FromBCD (Buf, 10);
    Buf += 10;

    // Version 1.93+1.94 extensions follow
    if (FirmwareVersion > 1.92) {

        // This is a new version config record
        TerminalMode = (*Buf++ != 0);

        if (FirmwareVersion > 1.93) {
            InternalKnock = *Buf++;
            Buf++;      // Skip next byte
            ExternalKnock = Buf [0] + 256 * Buf [1];
            Buf += 2;
            if (FirmwareVersion > 1.95) {
                ExtNum.Trunc (*Buf++);
                RerouteIfBusy       = (*Buf++ != 0);
                RerouteIfVoid       = (*Buf++ != 0);
                RingsUntilReroute   = *Buf++;
            } else {
                ExtNumLen           = 0;
                RerouteIfBusy       = 0;
                RerouteIfVoid       = 0;
                RingsUntilReroute   = 0;
            }

        } else {
            // Clear out the additional fields
            InternalKnock       = 0;
            ExternalKnock       = 0;
            ExtNumLen           = 0;
            RerouteIfBusy       = 0;
            RerouteIfVoid       = 0;
            RingsUntilReroute   = 0;
        }

    } else {

        // Clear out the additional fields
        TerminalMode        = 0;
        InternalKnock       = 0;
        ExternalKnock       = 0;
        ExtNumLen           = 0;
        RerouteIfBusy       = 0;
        RerouteIfVoid       = 0;
        RingsUntilReroute   = 0;

    }

    // Return the prepared struct
    return *this;
}



IstecDevConfig& IstecDevConfig::Unpack (const IstecMsg& Msg, double aFirmware)
// Unpack an array of char that contains data for a IstecDevConfig struct.
// The function returns *this and corrects invalid raw values.
{
    PRECONDITION (Msg.Size >= 18 && Msg.At (0) == 0x16);
    return Unpack (&Msg.Data [1], aFirmware);
}



int operator == (const IstecDevConfig& lhs, const IstecDevConfig& rhs)
{
    // For simplicity, convert to an array, then do a memcmp
    unsigned char Left [DevConfigSize], Right [DevConfigSize];
    return memcmp (lhs.Pack (Left), rhs.Pack (Right), DevConfigSize) == 0;
}



int operator != (const IstecDevConfig& lhs, const IstecDevConfig& rhs)
{
    // For simplicity, convert to an array, then do a memcmp
    unsigned char Left [DevConfigSize], Right [DevConfigSize];
    return memcmp (lhs.Pack (Left), rhs.Pack (Right), DevConfigSize) != 0;
}



/*****************************************************************************/
/*                            class IstecDevColl                             */
/*****************************************************************************/



int IstecDevColl::Compare (const unsigned char* Key1, const unsigned char* Key2)
{
    if (*Key1 < *Key2) {
        return -1;
    } else if (*Key1 > *Key2) {
        return 1;
    } else {
        return 0;
    }
}



const unsigned char* IstecDevColl::KeyOf (const IstecDevConfig* Item)
// Helpers for managing sort order
{
    return &Item->DevNum;
}



IstecDevColl::IstecDevColl (StreamableInit):
    SortedCollection<IstecDevConfig, unsigned char> (Empty)
// Build constructor
{
}



IstecDevColl::IstecDevColl ():
    SortedCollection<IstecDevConfig, unsigned char> (8, 8, 1)
// Create a IstecDevColl
{
}



u16 IstecDevColl::StreamableID () const
// Return the stream ID
{
    return ID_IstecDevColl;
}



Streamable* IstecDevColl::Build ()
// Return a new instance
{
    return new IstecDevColl (Empty);
}



IstecDevConfig& IstecDevColl::NewDev (unsigned char Dev)
// Create and insert a new device with the given number
{
    // Create a new entry
    IstecDevConfig* Cfg = new IstecDevConfig (Dev);

    // Insert the new entry...
    Insert (Cfg);

    // ... and return it
    return *Cfg;
}



/*****************************************************************************/
/*                             class IstecConfig                             */
/*****************************************************************************/



IstecConfig::IstecConfig ()
// Create an IstecConfig object
{
    // Insert the devices into the collection
    for (unsigned I = 0; I < GetDevCount (); I++) {
        DevColl.NewDev (I);
    }
}



IstecConfig::IstecConfig (StreamableInit):
    BaseConfig (Empty),
    DevColl (Empty)
    // Build constructor
{
}



void IstecConfig::Load (Stream& S)
// Load the object from a stream
{
    S >> BaseConfig >> DevColl;
}



void IstecConfig::Store (Stream& S) const
// Store the object into a stream
{
    S << BaseConfig << DevColl;
}



u16 IstecConfig::StreamableID () const
// Return the stream ID
{
    return ID_IstecConfig;
}



Streamable* IstecConfig::Build ()
// Return a new instance
{
    return new IstecConfig (Empty);
}



IstecDevConfig& IstecConfig::GetDevConfig (unsigned char Dev)
// Get the config for the specified device. If it does not exist, it is created
{
    // Search for the index
    int Index;
    if (DevColl.Search (&Dev, Index) == 0) {
        // No entry til now, create one
        return DevColl.NewDev (Dev);
    } else {
        // Found an entry, return it
        return *DevColl [Index];
    }
}



const IstecDevConfig& IstecConfig::GetDevConfig (unsigned char Dev) const
// Get the config for the specified device. If it does not exist,
// FAIL is called
{
    // Unfortunately, the collection member functions are not made const
    // as needed, and changing them now will break lots of old code. So
    // do it the ugly way and cast away the constness of this.
    IstecConfig* THIS = (IstecConfig*) (this);

    // Search for the index
    int Index;
    if (THIS->DevColl.Search (&Dev, Index) == 0) {
        // No entry til now, this must not happen!
        FAIL ("IstecConfig::GetDevConfig: Device does not exist!");
    }
    return *DevColl.At (Index);
}



void IstecConfig::UnpackDevConfig (const IstecMsg& Msg, double aFirmware)
// Unpack a device config from an istec message
{
    PRECONDITION (Msg.Size > 18 && Msg.At (0) == 0x16);
    IstecDevConfig& DevConfig = GetDevConfig (Msg.Data [1]);
    DevConfig.Unpack (&Msg.Data [1], aFirmware);
}



void IstecConfig::UnpackBaseConfig (const IstecMsg& Msg)
// Unpack the base configuguration from an istec message
{
    BaseConfig.Unpack (Msg);
}



void IstecConfig::PackDevConfig (unsigned char Dev, unsigned char* Buf,
                                 double aFirmware) const
// Pack a device config
{
    const IstecDevConfig& DevConfig = GetDevConfig (Dev);
    DevConfig.Pack (Buf, aFirmware);
}



void IstecConfig::PackBaseConfig (unsigned char* Buf) const
// Pack the base configuration
{
    BaseConfig.Pack (Buf);
}



String IstecConfig::GetNumber1 () const
// Get the numbers from the base configuration
{
    char Buf [sizeof (BaseConfig.Number1)];
    FromPascal (Buf, BaseConfig.Number1, sizeof (Buf));
    return Buf;
}



String IstecConfig::GetNumber2 () const
// Get the numbers from the base configuration
{
    char Buf [sizeof (BaseConfig.Number2)];
    FromPascal (Buf, BaseConfig.Number2, sizeof (Buf));
    return Buf;
}



void IstecConfig::SetNumber1 (const String& Num)
// Set the number in the base configuration
{
    ToPascal (Num.GetStr (), BaseConfig.Number1, sizeof (BaseConfig.Number1));
}



void IstecConfig::SetNumber2 (const String& Num)
// Set the number in the base configuration
{
    ToPascal (Num.GetStr (), BaseConfig.Number2, sizeof (BaseConfig.Number2));
}



/*****************************************************************************/
/*                             class IstecCharges                            */
/*****************************************************************************/



void IstecCharges::Clear ()
// Clear the charges
{
    for (unsigned I = 0; I < IstecDevCount; I++) {
        Charges [I] = 0;
    }
}



unsigned& IstecCharges::operator [] (unsigned Device)
// Return a reference to the charges of a specific device
{
    PRECONDITION (Device < IstecDevCount);
    return Charges [Device];
}



const unsigned& IstecCharges::operator [] (unsigned Device) const
// Return a reference to the charges of a specific device
{
    PRECONDITION (Device < IstecDevCount);
    return Charges [Device];
}



unsigned char* IstecCharges::Pack (unsigned char* Buf) const
// Pack the data of IstecCharges into an array ready for transmission.
// The function returns Buf.
{
    unsigned DevCount = FirmwareVersion < 2.00? IstecDevCount : 8;
    unsigned char* B  = Buf;

    for (unsigned I = 0; I < DevCount; I++) {

        // Check range
        CHECK (Charges [I] <= 0xFFFF);

        // Get the value
        u16 Val = (u16) Charges [I];

        // Store the value into the array in little endian format
        *B++ = (unsigned char) Val;
        *B++ = (unsigned char) (Val / 256);

    }

    // Return the prepared buffer
    return Buf;
}



IstecCharges& IstecCharges::Unpack (const unsigned char* Buf)
// Unpack an array of char that contains data for an IstecCharges struct.
// The function returns this.
{
    unsigned DevCount = FirmwareVersion < 2.00? IstecDevCount : 8;

    for (unsigned I = 0; I < DevCount; I++) {

        // Convert the value from little endian format
        Charges [I] = Buf [0] + (unsigned)Buf [1] * 256;

        // Next one
        Buf += 2;
    }

    // Return the prepared struct
    return *this;
}



int operator == (const IstecCharges& lhs, const IstecCharges& rhs)
{
    unsigned DevCount = FirmwareVersion < 2.00? IstecDevCount : 8;

    for (unsigned I = 0; I < DevCount; I++) {
        if (lhs.Charges [I] != rhs.Charges [I]) {
            return 0;
        }
    }
    return 1;
}



int operator != (const IstecCharges& lhs, const IstecCharges& rhs)
{
    unsigned DevCount = FirmwareVersion < 2.00? IstecDevCount : 8;

    for (unsigned I = 0; I < DevCount; I++) {
        if (lhs.Charges [I] == rhs.Charges [I]) {
            return 0;
        }
    }
    return 1;
}



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



unsigned char* ToBCD (const char* S, unsigned char* T, unsigned TSize, int LowFirst)
// Convert the number string in S to a BCD representation in T, filling unused
// digits with 'F'. The function returns T. When LowFirst is set, decoding
// order is low/high, when LowFirst is zero it's the other way round.
{
    // Check the parameters
    PRECONDITION (S != NULL && (T != NULL || TSize == 0));

    // Calculate the maximum digit count and make shure, T is big enough
    unsigned Count = TSize * 2;
    unsigned Len   = strlen (S);
    if (Len > Count) {
        // OOPS - S is too long, what now?
        IstecError (msErrorIntIgnored);
        Len = Count;
    }

    // Pre-fill the target string with FF
    memset (T, 0xFF, TSize);

    // Convert to BCD and store
    unsigned char* P = T;
    int LowNibble = LowFirst;
    if (LowFirst) {
        while (Len--) {
            if (LowNibble) {
                *P = (*S - '0') | 0xF0;
                LowNibble = 0;
            } else {
                *P = ((*S - '0') << 4) | (*P & 0x0F);
                LowNibble = 1;
                P++;
            }
            S++;
        }
    } else {
        while (Len--) {
            if (LowNibble) {
                *P = (*P & 0xF0) | ((*S - '0') & 0x0F);
                LowNibble = 0;
                P++;
            } else {
                *P = ((*S - '0') << 4) | 0x0F;
                LowNibble = 1;
            }
            S++;
        }
    }

    // Return the target buffer
    return T;
}



char* FromBCD (char* S, const unsigned char* T, unsigned TSize, int LowFirst)
// Convert the BCD string in T into an ASCII representation in S. Conversion
// stops if an invalid BCD char or TSize is reached. A trailing zero is added
// to S. It is assumed that S is big anough to hold the resulting string (S
// must be TSize*2+1 chars in size). When LowFirst is set, decoding order
// is low/high, when LowFirst is zero it's the other way round.
// The function returns S.
{
    // Check the parameters
    PRECONDITION (S != NULL && (T != NULL || TSize == 0));

    // Calculate the nibble count
    TSize *= 2;

    // Conversion loop
    char* Q = S;
    unsigned LowNibble = LowFirst;
    while (TSize--) {

        int C;
        if (LowNibble) {
            C = *T & 0x0F;
            LowNibble = 0;
            if (!LowFirst) {
                T++;
            }
        } else {
            C = (*T & 0xF0) >> 4;
            LowNibble = 1;
            if (LowFirst) {
                T++;
            }
        }

        if (C < 10) {
            *Q++ = '0' + C;
        } else {
            // Invalid char, end reached
            break;
        }
    }

    // Add a trailing zero
    *Q = '\0';

    // return the result
    return S;
}



String FromBCD (const unsigned char* T, unsigned TSize, int LowFirst)
// Convert the BCD string in T into an ASCII representation. Conversion
// stops if an invalid BCD char or TSize is reached. When LowFirst is set,
// decoding order is low/high, when LowFirst is zero it's the other way round.
// The resulting string is returned.
{
    // Allocate memory for the result
    char* Buf = new char [TSize * 2 + 1];

    // Convert the string
    String S = FromBCD (Buf, T, TSize, LowFirst);

    // Delete the buffer memory
    delete [] Buf;

    // Return the result
    return S;
}



unsigned char* ToPascal (const char* S, unsigned char* T, unsigned TSize)
// Convert the C style string in S to the pascal string in T and return T.
{
    // Check parameters
    PRECONDITION (S != NULL && T != NULL && TSize > 0);

    // Copy the string
    unsigned char* Q = T + 1;
    unsigned Length = 0;
    while (*S && TSize--) {
        *Q++ = *S++;
        Length++;
    }

    // Pad the target string with zeros
    while (TSize--) {
        *Q++ = '\0';
    }

    // Set the string length
    *T = Length;

    // Return the result
    return T;
}



char* FromPascal (char* S, const unsigned char* T, unsigned SSize)
// Convert the pascal style string in T into a C like string in S and return S.
{
    // Check parameters
    PRECONDITION (S != NULL && T != NULL && SSize > 0);

    // Get the string length and restrict it if S is not big enough
    unsigned Length = *T++;
    if (Length > SSize-1) {
        Length = SSize-1;
    }

    // Copy the string
    char* Q = S;
    while (Length--) {
        *Q++ = *T++;
    }

    // Add a trailing zero
    *Q = '\0';

    // Return the result
    return S;
}



unsigned char InSignal (unsigned char Type)
// Convert the given signal type into the internally used representation
{
    switch (Type) {
        case 1:         return siSignal1;
        case 2:         return siSignal2;
        case 3:         return siNone;
        case 4:         return siSignal3;
        default:        return siStandard;
    }
}



unsigned char OutSignal (unsigned char Type)
// Convert the given signal type into the representation used by the istec
{
    switch (Type) {
        case siStandard:        return 0;
        case siSignal1:         return 1;
        case siSignal2:         return 2;
        case siSignal3:         return 4;
        case siNone:            return 3;
        default:
            FAIL ("OutSignal: Unknown signal type");
            return 0;
    }
}



