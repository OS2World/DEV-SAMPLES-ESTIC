/*****************************************************************************/
/*                                                                           */
/*                                  THREAD.H                                 */
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



#ifndef _THREAD_H
#define _THREAD_H



#include "machine.h"
#include "object.h"
#include "circbuf.h"
#include "keydef.h"
#include "window.h"



/*****************************************************************************/
/*                               class Thread                                */
/*****************************************************************************/



class Thread: public Object {

    friend inline Thread* CurThread ();
    friend void Window::Activate ();
    friend void Window::Deactivate ();

private:
    struct RegKey {
        RegKey*   Next;
        Key       K;
    };

    static Thread*              Current;
    ListNode<Window>            ANode;
    RegKey*                     KeyList;

protected:
    CircularBuffer<Key, 16>     KeyBuf;
    int                         Quit;

//  virtual void HandleEvent (Event &)
    virtual void GetEvent ();

private:
    // Static signal handlers. This handler does nothing but calling
    // Current->Sig<whatever>. To handle a signal, just override the virtual
    // Sig<whatever> functions. If the virtual function returns zero, the
    // signal is re-raised, otherwise it is assumed that the function has
    // handled the signal.
    // Beware: On some architectures a few signals do not occur( e.g. DOS).
    // On other, some signals are only delivered to thread 1 (the application).
    // The default action of all Sig<whatever> functions is to re-raise the
    // signal. Because the handling is set to SIG_DFLT before calling those
    // functions, the default handler will catch and handle the signal.
    static void SIGHandler (int);

protected:
    // Functions called by SIGHandler
    virtual int SigTerm ();
    virtual int SigBreak ();
    virtual int SigInt ();
    virtual int SigIll ();
    virtual int SigFPE ();
    virtual int SigSegV ();
    virtual int SigBus ();
    virtual int SigAbrt ();
    virtual int SigUsr1 ();
    virtual int SigUsr2 ();
    virtual int SigUsr3 ();
    virtual int SigWinCh ();            // Linux only
    virtual int SigDivZ ();             // Watcom OS/2 only
    virtual int SigOVF ();              // Watcom OS/2 only

public:
    Thread ();
    ~Thread ();

    // The main execution function
    virtual int Run ()  = 0;

    // Checking the Quit Flag
    int Quitting ();

    virtual void Idle ();
    // This is the idle function of the thread class. Default is to
    // do nothing. This function may be called by anyone at anytime
    // but it's not guaranteed to be called regularly.

    // Handling Keyboard input
    virtual void KbdPut (Key K);
    virtual Key KbdGet ();
    virtual Key KbdPeek ();
    virtual int KbdKeyAvail ();

    // Registering keys
    void RegisterKey (Key K);
    void UnregisterKey (Key K);
    int KeyIsRegistered (Key K);
};



inline Thread* CurThread ()
{
    return Thread::Current;
}



inline int Thread::Quitting ()
{
    return Quit;
}



// End of THREAD.H

#endif
