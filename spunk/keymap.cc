/*****************************************************************************/
/*                                                                           */
/*                                  KEYMAP.CC                                */
/*                                                                           */
/* (C) 1993-95  Ullrich von Bassewitz                                        */
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



#include <string.h>

#include "keymap.h"



/*****************************************************************************/
/*                      Explicit template instantiation                      */
/*****************************************************************************/



#ifdef EXPLICIT_TEMPLATES
template class Collection<KeyMap>;
template class SortedCollection<KeyMap, char>;
#endif



/*****************************************************************************/
/*                              class KeyMapper                              */
/*****************************************************************************/



KeyMapper::KeyMapper () :
    SortedCollection<KeyMap, char> (20, 10, 1)
{
}



void KeyMapper::Add (const char* S, Key K)
// If S is not NULL or empty, add an entry with the given mapping
{
    if (S && *S != '\0') {
        Insert (new KeyMap (S, K));
    }
}



void* KeyMapper::GetItem (Stream& S)
{
    return (void*) S.Get ();
}



void KeyMapper::PutItem (Stream& S, void* Item) const
{
    S.Put ((KeyMap*) Item);
}



int KeyMapper::Compare (const char* Key1, const char* Key2)
{
    return strcmp (Key1, Key2);
}



const char* KeyMapper::KeyOf (const KeyMap* Item)
{
    return Item->Sequence.GetStr ();
}



int KeyMapper::Find (const char* S, int& Index)
// Try to find the string S. Return values are
//   0: There has been no match and there will be no possible match, even
//      if we add more chars to S. Index is invalid on return.
//   1: There has been a partial match. Maybe we get a full match if we
//      add more chars to S. Index is invalid on return.
//   2: There has been a full match. Index is the index of the entry found.
{
    if (Search (S, Index)) {
        // We have a full match
        return 2;
    }

    int Len = strlen (S);
    while (Index < GetCount ()) {
        if (strncmp (S, At (Index)->Sequence.GetStr (), Len) == 0) {
            // Partial match
            return 1;
        }
        Index++;
    }

    // No match
    return 0;
}



