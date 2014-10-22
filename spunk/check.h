/*****************************************************************************/
/*                                                                           */
/*                                   CHECK.H                                 */
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



#ifndef _CHECK_H
#define _CHECK_H



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



extern const char* _MsgInternalError;           // "Internal error: "
extern const char* _MsgAbstractCall;            // "Call to abstract method"
extern const char* _MsgPrecondition;            // "Precondition violated: "
extern const char* _MsgCheckFailed;             // "Check failed: "



extern
#ifdef __GNUC__
volatile
#endif
void (*CheckFailed) (const char* Msg, const char* Cond, int Code,
                            const char* File, int Line);
// Function pointer that is called from Check if the condition code is true.



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



extern void Check (const char* Msg, const char* Cond, int Code,
                   const char* File, int Line);
// This function is called from all check macros (see below). It checks, wether
// the given Code is true (!= 0). If so, it calls the CheckFailed vector with
// the given strings. If not, it simply returns.



#define FAIL(s) CheckFailed (_MsgInternalError, s, 0, __FILE__, __LINE__)
// Fail macro. Is used if something evil happens, calls CheckFailed directly.



#define ABSTRACT() FAIL(_MsgAbstractCall)
// Short for FAIL. Is used in abstract class member functions.


#ifdef SPUNK_NODEBUG

#define PRECONDITION(c)
#define CHECK(c)
#define ZCHECK(c)

#else

// These macros are usually defined!!!
#define PRECONDITION(c) Check (_MsgPrecondition,                \
                        #c, !(c), __FILE__, __LINE__)

#define CHECK(c)        Check (_MsgCheckFailed,                 \
                        #c, !(c), __FILE__, __LINE__)

#define ZCHECK(c)       Check (_MsgCheckFailed,                 \
                        #c, c, __FILE__, __LINE__)

#endif



// End of CHECK.H

#endif
