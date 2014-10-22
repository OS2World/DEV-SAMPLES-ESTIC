/*****************************************************************************/
/*									     */
/*				    THREAD.CC				     */
/*									     */
/* (C) 1993-96	Ullrich von Bassewitz					     */
/*		Wacholderweg 14						     */
/*		D-70597 Stuttgart					     */
/* EMail:	uz@ibb.schwaben.com					     */
/*									     */
/*****************************************************************************/



// $Id$
//
// $Log$
//
//



#include <signal.h>

#include "screen.h"
#include "kbd.h"
#include "window.h"
#include "thread.h"



/*****************************************************************************/
/*			Explicit template instantiation			     */
/*****************************************************************************/



#ifdef EXPLICIT_TEMPLATES
template class CircularBuffer<Key, 16>;
#endif



/*****************************************************************************/
/*				 class Thread				     */
/*****************************************************************************/



// DJGPP has some weird declaration of the signal handler type so add a cast
#ifdef __GO32__
#define SignalHandleFunc(__f)	((SignalHandler) __f)
#else
#define SignalHandleFunc(__f)	__f
#endif



// Pointer to the current thread
Thread* Thread::Current;



Thread::Thread () :
    ANode (NULL),
    KeyList (NULL),
    Quit (0)
{
    Current = this;

    // Try to catch all signals
#ifdef SIGTERM
    signal (SIGTERM, SignalHandleFunc (SIGHandler));
#endif
#ifdef SIGBREAK
    signal (SIGBREAK, SignalHandleFunc (SIGHandler));
#endif
#ifdef SIGINT
    signal (SIGINT, SignalHandleFunc (SIGHandler));
#endif
#ifdef SIGILL
    signal (SIGILL, SignalHandleFunc (SIGHandler));
#endif
#ifdef SIGFPE
    signal (SIGFPE, SignalHandleFunc (SIGHandler));
#endif
#ifdef SIGSEGV
    signal (SIGSEGV, SignalHandleFunc (SIGHandler));
#endif
#ifdef SIGBUS
    signal (SIGBUS, SignalHandleFunc (SIGHandler));
#endif
#ifdef SIGABRT
    signal (SIGABRT, SignalHandleFunc (SIGHandler));
#endif
#ifdef SIGUSR1
    signal (SIGUSR1, SignalHandleFunc (SIGHandler));
#endif
#ifdef SIGUSR2
    signal (SIGUSR2, SignalHandleFunc (SIGHandler));
#endif
#ifdef SIGUSR3
    signal (SIGUSR3, SignalHandleFunc (SIGHandler));
#endif
#ifdef SIGWINCH
    signal (SIGWINCH, SignalHandleFunc (SIGHandler));
#endif

}



Thread::~Thread ()
{
    // Delete the list of registered keys
    while (KeyList) {
	RegKey* Key = KeyList->Next;
	delete KeyList;
	KeyList = Key;
    }
}



void Thread::GetEvent ()
{
    KeyBuf.Put (Kbd->Get ());
}



void Thread::KbdPut (Key K)
{
    KeyBuf.Put (K);
}



Key Thread::KbdGet ()
{
    while (KeyBuf.IsEmpty ()) {
	GetEvent ();
    }
    return KeyBuf.Get ();
}



Key Thread::KbdPeek ()
{
    if (!KeyBuf.IsEmpty ()) {
	return KeyBuf.Peek ();
    } else {
	return Kbd->Peek ();
    }
}



int Thread::KbdKeyAvail ()
{
    return (KeyBuf.IsEmpty () == 0 || Kbd->KeyAvail ());
}



void Thread::SIGHandler (int Sig)
{
    int Handled;

    // Re-Install the signal handler
    signal (Sig, SignalHandleFunc (SIGHandler));

    // Call the signal specific function
    switch (Sig) {

#ifdef SIGTERM
	case SIGTERM:
	    Handled = CurThread () -> SigTerm ();
	    break;
#endif

#ifdef SIGBREAK
	case SIGBREAK:
	    Handled = CurThread () -> SigBreak ();
	    break;
#endif

#ifdef SIGINT
	case SIGINT:
	    Handled = CurThread () -> SigInt ();
	    break;
#endif

#ifdef SIGILL
	case SIGILL:
	    Handled = CurThread () -> SigIll ();
	    break;
#endif

#ifdef SIGFPE
	case SIGFPE:
	    Handled = CurThread () -> SigFPE ();
	    break;
#endif

#ifdef SIGSEGV
	case SIGSEGV:
	    Handled = CurThread () -> SigSegV ();
	    break;
#endif

#ifdef SIGBUS
	case SIGBUS:
	    Handled = CurThread () -> SigBus ();
	    break;
#endif

#ifdef SIGABRT
	case SIGABRT:
	    Handled = CurThread () -> SigAbrt ();
	    break;
#endif

#ifdef SIGUSR1
	case SIGUSR1:
	    Handled = CurThread () -> SigUsr1 ();
	    break;
#endif

#ifdef SIGUSR2
	case SIGUSR2:
	    Handled = CurThread () -> SigUsr2 ();
	    break;
#endif

#ifdef SIGUSR3
	case SIGUSR3:
	    Handled = CurThread () -> SigUsr3 ();
	    break;
#endif

#ifdef SIGWINCH
	case SIGWINCH:
	    Handled = CurThread () -> SigWinCh ();
	    break;
#endif

#ifdef SIGDIVZ
	case SIGDIVZ:
	    Handled = CurThread () -> SigDivZ ();
	    break;
#endif

#ifdef SIGOVF
	case SIGOVF:
	    Handled = CurThread () -> SigOVF ();
	    break;
#endif

	default:
	    Handled = 0;
	    break;
    }

    // If the signal was not handled, install the default-handler and
    // re-raise the signal.
    if (!Handled) {
#ifndef __GO32__
	signal (Sig, SignalHandleFunc (SIG_DFL));
	raise (Sig);
#else
	// DJGPP is not able to raise a signal, so end the program
	FAIL ("Thread::SigHandler: Unhandled signal");
#endif
    }
}



int Thread::SigTerm ()
{
    // Don't ignore this one
    return 0;
}



int Thread::SigBreak ()
{
    // Don't ignore this one!
    return 0;
}



int Thread::SigInt ()
{
    // Don't ignore this one
    return 0;
}



int Thread::SigIll ()
{
    // OOPS - re-raise the signal
    return 0;
}



int Thread::SigFPE ()
{
    // OOPS - re-raise the signal
    FAIL ("Unexpected floating point exception");
    return 0;
}



int Thread::SigSegV ()
{
    // OOPS - re-raise the signal
    FAIL ("Segmentation violation");
    return 0;
}



int Thread::SigBus ()
{
    // OOPS - re-raise the signal
    return 0;
}



int Thread::SigAbrt ()
{
    // OOPS - re-raise the signal
    return 0;
}



int Thread::SigUsr1 ()
{
    // OOPS - re-raise the signal
    return 0;
}



int Thread::SigUsr2 ()
{
    // OOPS - re-raise the signal
    return 0;
}



int Thread::SigUsr3 ()
{
    // OOPS - re-raise the signal
    return 0;
}



int Thread::SigWinCh ()
{
    // Signal is handled
    return 1;
}



int Thread::SigDivZ ()
{
    // OOPS - re-raise the signal
    return 0;
}



int Thread::SigOVF ()
{
    // OOPS - re-raise the signal
    return 0;
}



void Thread::Idle ()
// This is the idle function of the thread class. Default is to
// do nothing. This function may be called by anyone at anytime
// but it's not guaranteed to be called regularly.
{
}



void Thread::RegisterKey (Key K)
// Register a key
{
    // We cannot register a kbNoKey key
    if (K == kbNoKey) {
	FAIL ("Thread::RegisterKey: Trying to register kbNoKey");
    }

    // Create a dynamic structure to store data
    RegKey* P = new RegKey;

    // Store data, insert new key in list
    P->K    = K;
    P->Next = KeyList;
    KeyList = P;
}



void Thread::UnregisterKey (Key K)
// Unregister a key
{
    RegKey* P  = KeyList;
    RegKey** Q = &KeyList;
    while (P != NULL) {
	if (P->K == K) {
	    // Unlink node, delete registration record
	    *Q = P->Next;
	    delete P;
	    return;
	}
	Q = &P->Next;
	P = P->Next;
    }
    FAIL ("Thread:KbdUnregister: Key not found");
}



int Thread::KeyIsRegistered (Key K)
// Check if a key is registered
{
    RegKey* P = KeyList;
    while (P != NULL) {
	if (P->K == K) {
	    // Found the key
	    return 1;
	}
	P = P->Next;
    }
    return 0;
}



