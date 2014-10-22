/*****************************************************************************/
/*                                                                           */
/*                                SETTINGS.CC                                */
/*                                                                           */
/* (C) 1993-96  Ullrich von Bassewitz                                        */
/*              Zwehrenbuehlstrasse 33                                       */
/*              D-72070 Tuebingen                                            */
/* EMail:       uz@ibb.schwaben.com                                          */
/*                                                                           */
/*****************************************************************************/



// $Id$
//
// $Log$
//
//



// This module declares functions to work with program options (settings).
// Settings are stored in a class derived from class ResourceFile that is
// used internally. The external interface is a procedural one - this allows
// the functions to check for a non existing or not open settings resource.
// A not existing file is transparent to the programmer, the functions behave
// as if the key did not exist (when calling OptGet...) and ignore the data
// (when calling OptPut).



#include "rect.h"
#include "filepath.h"
#include "memstrm.h"
#include "resource.h"
#include "settings.h"



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



// Flag that says if we should flush the Settings file after every write
// access. Safe but slow.
static int FlushAlways;

// Global pointer to the one and only settings database
static class ResourceFile* Settings = NULL;



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



static void StgFlushAlways ()
// Flush the file if the FlushAlways flag is set. This function is called
// after every write
{
    if (Settings && FlushAlways) {
        Settings->Flush ();
    }
}



unsigned StgOpen (const String& Filename, int MustFlush)
// Filename is expanded (made absolute) and the function tries to open a
// settings database with this name. If the database does not exist, a new
// one is created. A ResourceFile error code is returned (which is zero if
// no error occured). If there is an error, the database is deleted and all
/// other functions just ignore the data as explained above.
{
    // If there is already a settings object, something is wrong
    PRECONDITION (Settings == NULL);

    // Remember the flag
    FlushAlways = MustFlush;

    // Try to create the object
    Settings = new ResourceFile (new FileStream (MakeAbsolute (Filename)));

    // Check for errors
    unsigned Result = Settings->GetStatus ();
    if (Result != reOk) {

        // An error occurred
        delete Settings;
        Settings = NULL;

    } else {

        // In case we just created the file, flush it, so the file contents
        // are valid
        StgFlushAlways ();

    }

    // Ok, done
    return Result;
}



void StgClose ()
// Close the settings database
{
    if (Settings) {
        // If the status is ok, pack the file
        if (Settings->GetStatus () == reOk) {
            Settings->Pack ();
        }
        // Delete the file
        delete Settings;
        Settings = NULL;
    }
}



int StgEmpty ()
// Return true if the settings file is empty, that is, it does not contain
// any resources
{
    return Settings == 0 || Settings->GetCount () == 0;
}



void StgFlush ()
// Flush the settings database to disk
{
    if (Settings) {
        Settings->Flush ();
    }
}



Streamable* StgGet (const String& Name)
// Read an object from the settings database, returns NULL if the key was not
// found or the database does not exist.
{
    if (Settings) {
        return Settings->Get (Name);
    } else {
        // No database
        return NULL;
    }
}



Point StgGetPoint (const String& Name, const Point& Default)
// Return a point (position) or the default if there is no matching
// key in the resource.
{
    // As Point is not a Streamable object there is some work to read/write
    // a Point. The way to do it is to use a MemoryStream object that holds
    // the Point and read/write this MemoryStream object.
    MemoryStream* M = (MemoryStream*) StgGet (Name);
    if (M == NULL) {
        // No such key
        return Default;
    } else {
        // Found! Get the Point from the MemoryStream and delete it
        Point P;
        *M >> P;
        delete M;
        return P;
    }
}



Point StgGetPoint (const String& Name, int XDef, int YDef)
// Return a point (position) or the default if there is no matching
// key in the resource.
{
    return StgGetPoint (Name, Point (XDef, YDef));
}



Rect StgGetRect (const String& Name, const Rect& Default)
// Return a rect or the default if there is no matching key
// in the resource.
{
    // As Rect is not a Streamable object there is some work to read/write
    // a Rect. The way to do it is to use a MemoryStream object that holds
    // the Rect and read/write this MemoryStream object.
    MemoryStream* M = (MemoryStream*) StgGet (Name);
    if (M == NULL) {
        // No such key
        return Default;
    } else {
        // Found! Get the Rect from the MemoryStream and delete it
        Rect R;
        *M >> R;
        delete M;
        return R;
    }
}



String StgGetString (const String& Name, const String& Default)
// Return a String or the default if there is no matching key in the
// resource.
{
    String* S = (String*) StgGet (Name);
    if (S == NULL) {
        // Key not found
        return Default;
    } else {
        String T = *S;
        delete S;
        return T;
    }
}



void StgPut (const Streamable* Obj, const String& Name)
// Put an object into the settings database
{
    if (Settings) {
        Settings->Put (Obj, Name);
        StgFlushAlways ();
    }
}



void StgPutPoint (const Point& P, const String& Name)
// Write a Point object into the settings database
{
    if (Settings) {
        // As Point is not a Streamable object there is some work to read/write
        // a Point. The way to do it is to use a MemoryStream object that holds
        // the Point and read/write this MemoryStream object.
        MemoryStream M (sizeof (Point));
        M << P;
        Settings->Put (M, Name);
        StgFlushAlways ();
    }
}



void StgPutRect (const Rect& R, const String& Name)
// Write a Rect object into the settings database
{
    if (Settings) {
        // As Rect is not a Streamable object there is some work to read/write
        // a Rect. The way to do it is to use a MemoryStream object that holds
        // the Rect and read/write this MemoryStream object.
        MemoryStream M (sizeof (Rect));
        M << R;
        Settings->Put (M, Name);
        StgFlushAlways ();
    }
}



