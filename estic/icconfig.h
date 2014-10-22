/*****************************************************************************/
/*                                                                           */
/*                                ICCONFIG.H                                 */
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



#ifndef _ICCONFIG_H
#define _ICCONFIG_H



#include <string.h>

#include "coll.h"

#include "icver.h"
#include "istecmsg.h"



/*****************************************************************************/
/*                                 Constants                                 */
/*****************************************************************************/



// Possible values for IstecBaseConfig.Connection
const unsigned coPointToMulti           = 0;
const unsigned coPointToPoint           = 1;
const unsigned coMax                    = 1;

// Possible values for IstecBaseConfig.Protocol
const unsigned pr1TR6                   = 0;
const unsigned prDSS1                   = 1;
const unsigned prMax                    = 1;

// Possible values for IstecBaseConfig.DialCaps
const unsigned dcKeine                  = 0;
const unsigned dcInland                 = 1;
const unsigned dcOrt                    = 2;
const unsigned dcHalbamt                = 3;
const unsigned dcNichtamt               = 4;
const unsigned dcMax                    = 4;

// Possible values for IstecBaseConfig.Service
const unsigned svFernsprechen           = 0;
const unsigned svFaxG3                  = 1;
const unsigned svDatenModem             = 2;
const unsigned svDatexJModem            = 3;
const unsigned svAnrufbeantworter       = 4;
const unsigned svKombi                  = 5;

// Flags for the internal and external knock bitsets
const unsigned knInt21                  = 0x0001;
const unsigned knInt22                  = 0x0002;
const unsigned knInt23                  = 0x0004;
const unsigned knInt24                  = 0x0008;
const unsigned knInt25                  = 0x0010;
const unsigned knInt26                  = 0x0020;
const unsigned knInt27                  = 0x0040;
const unsigned knInt28                  = 0x0080;
const unsigned knMSN0                   = 0x0001;
const unsigned knMSN1                   = 0x0002;
const unsigned knMSN2                   = 0x0004;
const unsigned knMSN3                   = 0x0008;
const unsigned knMSN4                   = 0x0010;
const unsigned knMSN5                   = 0x0020;
const unsigned knMSN6                   = 0x0040;
const unsigned knMSN7                   = 0x0080;
const unsigned knMSN8                   = 0x0100;
const unsigned knMSN9                   = 0x0200;
const unsigned knTFE1                   = 0x0400;
const unsigned knTFE2                   = 0x0800;
const unsigned knTFE3                   = 0x1000;
const unsigned knTFE4                   = 0x2000;

// Signaling
const unsigned siStandard               = 0;
const unsigned siSignal1                = 1;
const unsigned siSignal2                = 2;
const unsigned siSignal3                = 3;
const unsigned siNone                   = 4;

// Count of MSN's
const unsigned bcMSNCount               = 10;

// Maximum count of devices
const unsigned IstecDevCount            = 64;

// Raw sizes of the transmission data buffers. The actual space occupied from
// the date maybe less, depending on the istec firmware version.
const unsigned BaseConfigSize           = 116;
const unsigned DevConfigSize            = 26;
const unsigned ChargeSize               = 128;



/*****************************************************************************/
/*                           class IstecBaseConfig                           */
/*****************************************************************************/



// Basic ISTEC configuration struct
class IstecBaseConfig: public Streamable {

public:
    unsigned char       Connection;
    unsigned char       DevCount;
    unsigned char       Protocol;
    unsigned char       VersionHigh;
    unsigned char       VersionLow;
    unsigned char       TFEAssignment;

    unsigned char       MSN [bcMSNCount] [5];   // MSN in BCD

    unsigned char       Music;                  // 0 == off, 1 == on
    unsigned char       IntS0;
    unsigned char       ExtS0;

    unsigned char       QueryLoc1;
    unsigned char       QueryLoc2;
    unsigned char       Number1 [11];           // Number 1, pascal style
    unsigned char       Number2 [11];           // Number 2, pascal style

    unsigned char       MSNGroups [10];

//  ----------------------------------------    // 1.93 and up
    unsigned char       MusicPort;
    u16                 CountryCode;
    unsigned char       TFELoc [4];             // TFE Location (bitmap)

//  ----------------------------------------    // 2.00 and up

    unsigned char       AlarmTone;              // Unused according to E. docs
    unsigned char       Signaling [bcMSNCount];
    unsigned char       Reserved [5];


    IstecBaseConfig ();
    // Constructor

    IstecBaseConfig (StreamableInit);
    // Build constructor

    virtual void Load (Stream& S);
    // Load the object from a stream

    virtual void Store (Stream& S) const;
    // Store the object into a stream

    virtual u16 StreamableID () const;
    // Return the stream ID

    static Streamable* Build ();
    // Return a new instance

    unsigned char* Pack (unsigned char* Buf) const;
    // Pack the data of struct IstecBaseConfig into an array ready for
    // transmission. The function returns Buf.

    IstecBaseConfig& Unpack (const unsigned char* Buf);
    IstecBaseConfig& Unpack (const IstecMsg& Msg);
    // Unpack an array of char that contains data for an IstecBaseConfig struct.
    // The function returns *this and corrects invalid raw values.

    friend int operator == (const IstecBaseConfig&, const IstecBaseConfig&);
    friend int operator != (const IstecBaseConfig&, const IstecBaseConfig&);
    // Compare two structs

    double GetFirmwareVersion () const;
    // Return the firmware version from this base configuration

    unsigned IstecID () const;
    // Return the type of the istec, determined by the parameters of the base
    // configuration. The return value is 0 if the istec type could not be
    // identified, 1008 for an istec 1008 etc.

};



inline IstecBaseConfig::IstecBaseConfig (StreamableInit)
// Build constructor
{
}



inline double IstecBaseConfig::GetFirmwareVersion () const
// Return the firmware version from this base configuration
{
    return double (VersionHigh) + double (VersionLow) / 100;
}



/*****************************************************************************/
/*                           class IstecDevConfig                            */
/*****************************************************************************/



// Configuration of the devices device
class IstecDevConfig: public Streamable {

public:
    unsigned char       DevNum;
    unsigned char       DialCaps;
    unsigned char       Service;
    unsigned char       Reroute;
    unsigned char       ChargePulse;
    unsigned char       PIN [2];
    String              ExtNum;

//  ---------------------------------   // 1.93 and up
    unsigned char       TerminalMode;

//  ---------------------------------   // 1.94 and up
    unsigned char       InternalKnock;
    u16                 ExternalKnock;

//  ---------------------------------   // 1.95 and up

    unsigned char       ExtNumLen;      // Length of external number

//  ---------------------------------   // 2.00 and up

    unsigned char       RerouteIfBusy;
    unsigned char       RerouteIfVoid;
    unsigned char       RingsUntilReroute;


    IstecDevConfig (unsigned char aDevNum);
    // Create an IstecDevConfig

    IstecDevConfig (StreamableInit);
    // Build constructor

    virtual void Load (Stream& S);
    // Load the object from a stream

    virtual void Store (Stream& S) const;
    // Store the object into a stream

    virtual u16 StreamableID () const;
    // Return the stream ID

    static Streamable* Build ();
    // Return a new instance

    void ClearExtNum ();
    // Clear the external number

    String GetPIN () const;
    // Get the PIN as a string

    const String& GetExtNum () const;
    // Get the external reroute number as a string

    void SetPIN (const String& aPIN);
    // Set the PIN from a string

    void SetExtNum (const String& aExtNum);
    // Set the external reroute number from a string

    unsigned char* Pack (unsigned char* Buf,
                         double aFirmwareVersion = FirmwareVersion) const;
    // Pack the data of struct IstecDevConfig into an array ready for
    // transmission. The function returns Buf.

    IstecDevConfig& Unpack (const unsigned char* Buf,
                            double aFirmware = FirmwareVersion);
    IstecDevConfig& Unpack (const IstecMsg& Msg, double aFirmware = FirmwareVersion);
    // Unpack an array of char that contains data for a IstecDevConfig struct.
    // The function returns this.

    friend int operator == (const IstecDevConfig&, const IstecDevConfig&);
    friend int operator != (const IstecDevConfig&, const IstecDevConfig&);
    // Compare two structs

    int GetIntKnock (unsigned Bit) const;
    // Return true if the internal knock bit is set

    int GetExtKnock (unsigned Bit) const;
    // Return true if the external knock bit is set

    void SetIntKnock (unsigned Bit);
    // Set the interal knock to true

    void SetExtKnock (unsigned Bit);
    // Set the external knock to true

    void ClrIntKnock (unsigned Bit);
    // Set the interal knock to false

    void ClrExtKnock (unsigned Bit);
    // Set the external knock to false

};



inline IstecDevConfig::IstecDevConfig (StreamableInit):
    ExtNum (Empty)
// Build constructor
{
}



inline const String& IstecDevConfig::GetExtNum () const
// Get the external reroute number as a string
{
    return ExtNum;
}



inline void IstecDevConfig::ClearExtNum ()
// Clear the external number
{
    ExtNum.Clear ();
}



inline void IstecDevConfig::SetExtNum (const String& aExtNum)
// Set the external reroute number from a string
{
    ExtNum = aExtNum;
}



inline int IstecDevConfig::GetIntKnock (unsigned Bit) const
// Return true if the internal knock bit is set
{
    return (InternalKnock & Bit) != 0;
}



inline int IstecDevConfig::GetExtKnock (unsigned Bit) const
// Return true if the external knock bit is set
{
    return (ExternalKnock & Bit) != 0;
}



inline void IstecDevConfig::SetIntKnock (unsigned Bit)
// Set the interal knock to true
{
    InternalKnock |= Bit;
}



inline void IstecDevConfig::SetExtKnock (unsigned Bit)
// Set the external knock to true
{
    ExternalKnock |= Bit;
}



inline void IstecDevConfig::ClrIntKnock (unsigned Bit)
// Set the interal knock to false
{
    InternalKnock &= ~Bit;
}



inline void IstecDevConfig::ClrExtKnock (unsigned Bit)
// Set the external knock to false
{
    InternalKnock &= ~Bit;
}



/*****************************************************************************/
/*                            class IstecDevColl                             */
/*****************************************************************************/



class IstecDevColl: public SortedCollection<IstecDevConfig, unsigned char> {

protected:
    virtual int Compare (const unsigned char* Key1, const unsigned char* Key2);
    virtual const unsigned char* KeyOf (const IstecDevConfig* Item);
    // Helpers for managing sort order


public:
    IstecDevColl ();
    // Create a IstecDevColl

    IstecDevColl (StreamableInit);
    // Build constructor

    virtual u16 StreamableID () const;
    // Return the stream ID

    static Streamable* Build ();
    // Return a new instance

    IstecDevConfig& NewDev (unsigned char Dev);
    // Create and insert a new device with the given number
};



/*****************************************************************************/
/*                             class IstecConfig                             */
/*****************************************************************************/



class IstecConfig: public Streamable {

private:
    IstecConfig (StreamableInit);
    // Build constructor


public:
    IstecBaseConfig     BaseConfig;
    IstecDevColl        DevColl;


    IstecConfig ();
    // Create an IstecConfig object

    virtual void Load (Stream& S);
    // Load the object from a stream

    virtual void Store (Stream& S) const;
    // Store the object into a stream

    virtual u16 StreamableID () const;
    // Return the stream ID

    static Streamable* Build ();
    // Return a new instance

    IstecDevConfig& GetDevConfig (unsigned char Dev);
    // Get the config for the specified device. If the entry does not exist,
    // it is created.

    const IstecDevConfig& GetDevConfig (unsigned char Dev) const;
    // Get the config for the specified device. If the entry does not exist,
    // FAIL is called.

    unsigned GetDevCount () const;
    // Return the device count

    void UnpackDevConfig (const IstecMsg& Msg, double aFirmware = FirmwareVersion);
    // Unpack a device config from an istec message

    void UnpackBaseConfig (const IstecMsg& Msg);
    // Unpack the base configuration from an istec message

    void PackDevConfig (unsigned char Dev, unsigned char* Buf,
                        double aFirmware = FirmwareVersion) const;
    // Pack a device config

    void PackBaseConfig (unsigned char* Buf) const;
    // Pack the base configuration

    double GetFirmwareVersion () const;
    // Return the firmware version from the base config

    unsigned IstecID () const;
    // Return the type of the istec, determined by the parameters of the base
    // configuration. The return value is 0 if the istec type could not be
    // identified, 1008 for an istec 1008 etc.

    unsigned GetExtS0 () const;
    // Return the count of external S0 busses

    unsigned GetIntS0 () const;
    // Return the count of internal S0 busses

    unsigned GetProtocol () const;
    // Return the protocol used by the istec

    unsigned GetConnection () const;
    // Get the connection value from the base configuration

    unsigned GetMusic () const;
    // Get the music value from the base configuration

    unsigned GetMusicPort () const;
    // Get the music port from the base configuration

    unsigned GetCountryCode () const;
    // Get the country code from the base configuration

    unsigned GetTFEAssignment () const;
    // Get the TFE assignment from the base configuration

    String GetNumber1 () const;
    String GetNumber2 () const;
    // Get the numbers from the base configuration

    void SetNumber1 (const String& Num);
    void SetNumber2 (const String& Num);
    // Set the number in the base configuration

};



inline unsigned IstecConfig::GetDevCount () const
{
    return BaseConfig.DevCount;
}



inline double IstecConfig::GetFirmwareVersion () const
// Return the firmware version from the base config
{
    return BaseConfig.GetFirmwareVersion ();
}



inline unsigned IstecConfig::IstecID () const
// Return the type of the istec, determined by the parameters of the base
// configuration. The return value is 0 if the istec type could not be
// identified, 1008 for an istec 1008 etc.
{
    return BaseConfig.IstecID ();
}



inline unsigned IstecConfig::GetExtS0 () const
// Return the count of external S0 busses
{
    return BaseConfig.ExtS0;
}



inline unsigned IstecConfig::GetIntS0 () const
// Return the count of internal S0 busses
{
    return BaseConfig.IntS0;
}



inline unsigned IstecConfig::GetProtocol () const
// Return the protocol used by the istec
{
    return BaseConfig.Protocol;
}



inline unsigned IstecConfig::GetConnection () const
// Get the connection value from the base configuration
{
    return BaseConfig.Connection;
}



inline unsigned IstecConfig::GetMusic () const
// Get the music value from the base configuration
{
    return BaseConfig.Music;
}



inline unsigned IstecConfig::GetMusicPort () const
// Get the music port from the base configuration
{
    return BaseConfig.MusicPort;
}



inline unsigned IstecConfig::GetCountryCode () const
// Get the country code from the base configuration
{
    return BaseConfig.CountryCode;
}



inline unsigned IstecConfig::GetTFEAssignment () const
// Get the TFE assignment from the base configuration
{
    return BaseConfig.TFEAssignment;
}



/*****************************************************************************/
/*                             class IstecCharges                            */
/*****************************************************************************/



class IstecCharges {

    unsigned Charges [IstecDevCount];

public:
    IstecCharges ();
    // Constructor - clears the charges on startup

    void Clear ();
    // Clear the charges

    unsigned& operator [] (unsigned Device);
    // Return a reference to the charges of a specific device

    const unsigned& operator [] (unsigned Device) const;
    // Return a reference to the charges of a specific device

    unsigned char* Pack (unsigned char* Buf) const;
    // Pack the data of IstecCharges into an array ready for transmission.
    // The function returns Buf.

    IstecCharges& Unpack (const unsigned char* Buf);
    // Unpack an array of char that contains data for an IstecCharges struct.
    // The function returns this.

    friend int operator == (const IstecCharges&, const IstecCharges&);
    friend int operator != (const IstecCharges&, const IstecCharges&);
    // Compare two structs

};



inline IstecCharges::IstecCharges ()
// Constructor - clears the charges on startup
{
    Clear ();
}



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



unsigned char* ToBCD (const char* S, unsigned char* T, unsigned TSize, int LowFirst = 1);
// Convert the number string in S to a BCD representation in T, filling unused
// digits with 'F'. The function returns T. When LowFirst is set, decoding
// order is low/high, when LowFirst is zero it's the other way round.

char* FromBCD (char* S, const unsigned char* T, unsigned TSize, int LowFirst = 1);
// Convert the BCD string in T into an ASCII representation in S. Conversion
// stops if an invalid BCD char or TSize is reached. A trailing zero is added
// to S. It is assumed that S is big anough to hold the resulting string (S
// must be TSize*2+1 chars in size). When LowFirst is set, decoding order
// is low/high, when LowFirst is zero it's the other way round.
// The function returns S.

String FromBCD (const unsigned char* T, unsigned TSize, int LowFirst = 1);
// Convert the BCD string in T into an ASCII representation. Conversion
// stops if an invalid BCD char or TSize is reached. When LowFirst is set,
// decoding order is low/high, when LowFirst is zero it's the other way round.
// The resulting string is returned.

unsigned char* ToPascal (const char* S, unsigned char* T, unsigned TSize);
// Convert the C style string in S to the pascal string in T and return T.

char* FromPascal (char* S, const unsigned char* T, unsigned SSize);
// Convert the pascal style string in T into a C like string in S and return S.

unsigned char InSignal (unsigned char Type);
// Convert the given signal type into the internally used representation

unsigned char OutSignal (unsigned char Type);
// Convert the given signal type into the representation used by the istec



// End of ICCONFIG.H

#endif


