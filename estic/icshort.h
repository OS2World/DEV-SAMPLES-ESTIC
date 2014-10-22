/*****************************************************************************/
/*                                                                           */
/*                                ICSHORT.H                                  */
/*                                                                           */
/* (C) 1996-97  Ullrich von Bassewitz                                        */
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



// Handle the short dial numbers



#ifndef _ICSHORT_H
#define _ICSHORT_H



#include "coll.h"

#include "istecmsg.h"



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



// Values for the "Usage" field in class ShortNumberInfo
const unsigned usUnused         = 0;
const unsigned usShortcut       = 1;
const unsigned usAutoDial       = 2;
const unsigned usLock           = 3;



/*****************************************************************************/
/*                           class ShortNumberInfo                           */
/*****************************************************************************/



class ShortNumberInfo: public Streamable {

    friend class ShortNumberColl;

    u16                 Memory;         // Memory location of this number
    u16                 Usage;          // Usage
    String              Number;         // Stored number
    unsigned char       AutoDial;       // "Babyruf"
    unsigned char       Devices;        // Valid for which device?
    unsigned char       Signaling;      // Ring attribute
    unsigned char       Locked;


public:
    ShortNumberInfo (unsigned aMemory);
    // Create a ShortNumberInfo

    ShortNumberInfo (const IstecMsg& Msg);
    // Create a ShortNumberInfo from an istec message

    ShortNumberInfo (StreamableInit);
    // Build constructor

    virtual void Load (Stream& S);
    // Load the object from a stream

    virtual void Store (Stream& S) const;
    // Store the object into a stream

    virtual u16 StreamableID () const;
    // Return the stream ID

    static Streamable* Build ();
    // Return a new instance

    void Unpack (const IstecMsg& Msg);
    // Unpack the contents of a message into the struct

    void Pack (IstecMsg& Msg) const;
    // Pack the data into a message ready for download to the istec

    unsigned GetMemory () const;
    // Return the memory setting

    unsigned GetUsage () const;
    // Get the usage info

    void SetUsage (unsigned NewUsage);
    // Set the usage info

    const String& GetNumber () const;
    // Get the number info

    void SetNumber (const String& NewNumber);
    // Set a new number

    unsigned GetSignaling () const;
    // Return the signaling info

    void SetSignaling (unsigned NewSignaling);
    // Set the signaling info

    String GetAlias () const;
    // Get the alias for the given number

    unsigned GetDeviceMap () const;
    // Return a bitmap of devices that are valid for the current usage

    void SetDeviceMap (unsigned NewMap);
    // Set a new device map

    String DeviceList ();
    // Return a list of devices (e.g. "-2--567-")

    const String& SignalName ();
    // Return the name for a specific signaling type

    const String& UsageName ();
    // Return the name for a usage
};



inline unsigned ShortNumberInfo::GetMemory () const
// Return the memory setting
{
    return Memory;
}



inline void ShortNumberInfo::SetUsage (unsigned NewUsage)
// Set the usage info
{
    Usage = NewUsage;
}



inline const String& ShortNumberInfo::GetNumber () const
// Get the number info
{
    return Number;
}



inline void ShortNumberInfo::SetSignaling (unsigned NewSignaling)
// Set the signaling info
{
    Signaling = NewSignaling;
}



/*****************************************************************************/
/*                           class ShortNumberColl                           */
/*****************************************************************************/



class ShortNumberColl: public SortedCollection<ShortNumberInfo, u16> {

protected:
    virtual int Compare (const u16* Key1, const u16* Key2);
    virtual const u16* KeyOf (const ShortNumberInfo* Item);
    // Helpers for managing sort order

public:
    ShortNumberColl ();
    // Construct a collection

    ShortNumberColl (StreamableInit);
    // Construct an empty collection

    ShortNumberInfo& NewShortNumber (u16 aMemory);
    // Create and insert a new short number

    ShortNumberInfo& NewShortNumber (const IstecMsg& Msg);
    // Create and insert a new short number

    ShortNumberInfo& GetShortNumber (u16 aMemory);
    // Get the entry in the specified memory. If it does not exist, it is created.
};



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



void ShortNumberList (ShortNumberColl& ShortNumbers, int& Changed);
// List all short numbers. Allow editing.

String FileSaveShort (const String& Filename, const ShortNumberColl& Numbers);
// Save short numbers to a file. The function returns an error message or
// the empty string if all is well.

String FileLoadShort (const String& Filename, ShortNumberColl& Numbers);
// Load a configuration from a file. The function returns an error message or
// the empty string if all is well.



// End of ICSHORT.H

#endif
