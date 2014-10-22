/*****************************************************************************/
/*                                                                           */
/*                                   KBD.H                                   */
/*                                                                           */
/* (C) 1993-96  Ullrich von Bassewitz                                        */
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



// Class Keyboard has some inline functions that don't need to be inline
// (because they are rather big and there is no need for speed), but they
// are declared this way, because they are only used in a few places (so
// no code size penalty) and this way they are out of the target specific
// .cc files.



#ifndef _KBD_H
#define _KBD_H


#include "machine.h"
#include "object.h"
#include "circbuf.h"
#include "keydef.h"
#include "str.h"



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



// Instance to handle keyboard input
extern class Keyboard* Kbd;



/*****************************************************************************/
/*                              Class Keyboard                               */
/*****************************************************************************/



class Keyboard : public Object {

private:
    CircularBuffer<Key, 16>     KeyBuf;
    int                         Console;
    unsigned char*              TransTable;


    Key RawKey ();
    // Get a raw (unmapped) key from the os

    void GetMappedKey (int Wait = 1);
    // Read keys until the needed key is not found in the mapper or until
    // a valid sequence is found. If Wait is zero, return immediately if
    // no match is found and no more keys are available.

    Key Translate (Key K);
    // Translate plain keys via the translation table (if valid)

public:
    Keyboard ();
    // Construct a keyboard object

    ~Keyboard ();
    // Destruct a keyboard object

    Key Get ();
    // Return a key from the keyboard

    Key Peek ();
    // Return the next key but don't remove it from the queue. This function
    // returns kbNoKey if no key is currently available.

    void Put (Key);
    // Put back a key into the queue

    int KeyAvail ();
    // Return true if a key is available

    int IsConsole ();
    // Return true if the keyboard is the console keyboard

    String GetKeyName (Key K);
    // Return a string describing the give key
};



inline Key Keyboard::Translate (Key K)
// Translate plain keys via the translation table (if valid)
{
    return IsPlainKey (K) && TransTable != NULL ? TransTable [K] : K;
}



inline Key Keyboard::Get ()
{
    if (KeyBuf.IsEmpty ()) {
        // Get a key from the os
        GetMappedKey ();
    }

    // In any case a key is now available
    return KeyBuf.Get ();
}



inline Key Keyboard::Peek ()
// Return the next key but don't remove it from the queue. This function
// returns kbNoKey if no key is currently available.
{
    if (KeyBuf.IsEmpty ()) {
        // Get new key but don't wait
        GetMappedKey (0);
    }

    // Return kbNoKey if no key is available, otherwise return the first
    // key in the buffer
    if (KeyBuf.IsEmpty ()) {
        return kbNoKey;
    } else {
        // Return top of stack
        return KeyBuf.Peek ();
    }
}



inline void Keyboard::Put (Key K)
{
    KeyBuf.PutInFront (K);
}



inline int Keyboard::KeyAvail ()
{
    if (!KeyBuf.IsEmpty ()) {
        // Buffer contains characters
        return 1;
    }

    // Buffer is emtpy, read new keys if there are any
    GetMappedKey (0);

    // Now return the buffer status
    return KeyBuf.IsEmpty () == 0;
}



inline int Keyboard::IsConsole ()
// Return true if the keyboard is the console keyboard
{
    return Console;
}



// End of KBD.H

#endif
