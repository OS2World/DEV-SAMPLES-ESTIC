/*****************************************************************************/
/*                                                                           */
/*                                 ICFILE.H                                  */
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



// Load/save configuration data to a file



#include "stream.h"
#include "syserror.h"
#include "progutil.h"

#include "icmsg.h"
#include "icfile.h"



/*****************************************************************************/
/*                             Message constants                             */
/*****************************************************************************/



const u16 msInvalidVersion              = MSGBASE_ICFILE + 0;
const u16 msOpenError                   = MSGBASE_ICFILE + 1;



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



// Change this if the data representation changes
const u32 FileVersion   = 160;



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



String IstecSaveFile (const String& Filename, const IstecConfig& Config)
// Save a configuration to a file. The function returns an error message or
// the empty string if all is well.
{
    // Open the stream
    FileStream S (Filename, "wb");
    if (S.GetStatus () != stOk) {
        return LoadAppMsg (msOpenError) + GetSysErrorMsg (S.GetErrorInfo ());
    }

    // Put version and config data into the file
    S << FileVersion << Config;

    // Success
    return "";
}



String IstecLoadFile (const String& Filename, IstecConfig& Config)
// Load a configuration from a file. The function returns an error message or
// the empty string if all is well.
{
    // Open the stream
    FileStream S (Filename, "rb");
    if (S.GetStatus () != stOk) {
        return LoadAppMsg (msOpenError) + GetSysErrorMsg (S.GetErrorInfo ());
    }

    // Read the version from the file
    u32 Version;
    S >> Version;

    if (Version != FileVersion) {
        return LoadAppMsg (msInvalidVersion);
    }

    // Load the config data
    S >> Config;

    // Success
    return "";
}



