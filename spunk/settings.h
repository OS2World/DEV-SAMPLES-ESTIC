/*****************************************************************************/
/*                                                                           */
/*                                SETTINGS.H                                 */
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



#ifndef __SETTINGS_H
#define __SETTINGS_H



// This module declares functions to work with program options (settings).
// Settings are stored in a class derived from class ResourceFile that is
// used internally. The external interface is a procedural one - this allows
// the functions to check for a non existing or not open settings resource.
// A not existing file is transparent to the programmer, the functions behave
// as if the key did not exist (when calling StgGet...) and ignore the data
// (when calling StgPut).



#include "rect.h"
#include "str.h"



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



unsigned StgOpen (const String& Filename, int FlushAlways = 1);
// Filename is expanded (made absolute) and the function tries to open a
// settings database with this name. If the database does not exist, a new
// one is created. A ResourceFile error code is returned (which is zero if
// no error occured). If there is an error, the database is deleted and all
/// other functions just ignore the data as explained above.
// If FlushAlways is set != zero, the settings file is flushed after every
// write to prevent the file from becoming corrupted in case of a program
// crash.

void StgClose ();
// Close the settings database

int StgEmpty ();
// Return true if the settings file is empty, that is, it does not contain
// any resources

void StgFlush ();
// Flush the settings database to disk

Streamable* StgGet (const String& Name);
// Read an object from the settings database, returns NULL if the key was not
// found or the database does not exist.

Point StgGetPoint (const String& Name, const Point& Default);
// Return a point (position) or the default if there is no matching
// key in the resource.

Point StgGetPoint (const String& Name, int XDef, int YDef);
// Return a point (position) or the default if there is no matching
// key in the resource.

Rect StgGetRect (const String& Name, const Rect& Default);
// Return a rect or the default if there is no matching key
// in the resource.

String StgGetString (const String& Name, const String& Default);
// Return a String or the default if there is no matching key in the
// resource.

void StgPut (const Streamable* Obj, const String& Name);
// Put an object into the settings database

inline void StgPut (const Streamable& Obj, const String& Name)
// Put an object into the settings database
{
    StgPut (&Obj, Name);
}

void StgPutPoint (const Point& P, const String& Name);
// Write a Point object into the settings database

void StgPutRect (const Rect& R, const String& Name);
// Write a Rect object into the settings database

inline void StgPutString (const String& S, const String& Name)
// Write a String object into the settings database
{
    StgPut (S, Name);
}



// End of SETTINGS.H

#endif

