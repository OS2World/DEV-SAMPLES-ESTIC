/*****************************************************************************/
/*                                                                           */
/*                                  KEYMAP.H                                 */
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



#ifndef __KEYMAP_H
#define __KEYMAP_H


#include "coll.h"
#include "str.h"
#include "keydef.h"



/*****************************************************************************/
/*                               class KeyMap                                */
/*****************************************************************************/



// Forward
class KeyMapper;

class KeyMap: public Streamable {

    friend class KeyMapper;

private:
    String      Sequence;               // ESC sequence from keyboard
    Key         K;                      // Resulting key

public:
    KeyMap (const String& S, Key aKey);
    // Construct a keymap object

    KeyMap  (const char* S, Key aKey);
    // Construct a keymap object

    Key GetKey () const;
    // Return the key

};



inline KeyMap::KeyMap (const String& S, Key aKey):
    Sequence (S),
    K (aKey)
{
}



inline KeyMap::KeyMap (const char* S, Key aKey):
    Sequence (S),
    K (aKey)
{
}



inline Key KeyMap::GetKey () const
{
    return K;
}



/*****************************************************************************/
/*                              class KeyMapper                              */
/*****************************************************************************/



class KeyMapper: public SortedCollection<KeyMap, char> {

protected:
    // Derived from class Collection
    virtual void* GetItem (Stream& S);
    virtual void PutItem (Stream& S, void* Item) const;

    virtual int Compare (const char* Key1, const char* Key2);
    virtual const char* KeyOf (const KeyMap* Item);

    KeyMapper (StreamableInit);
    // Build constructor

public:
    KeyMapper ();
    // Construct a KeyMapper object

    void Add (const char* S, Key K);
    // If S is not NULL or empty, add an entry with the given mapping

    int Find (const char* S, int& Index);
    // Try to find the string S. Return values are
    //   0: There has been no match and there will be no possible match, even
    //      if we add more chars to S. Index is invalid on return.
    //   1: There has been a partial match. Maybe we get a full match if we
    //      add more chars to S. Index is invalid on return.
    //   2: There has been a full match. Index is the index of the entry found.
};



inline KeyMapper::KeyMapper (StreamableInit):
    SortedCollection<KeyMap, char> (Empty)
{
}



// End of KEYMAP.H

#endif
