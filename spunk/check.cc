/*****************************************************************************/
/*                                                                           */
/*                                   CHECK.CC                                */
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



#include <stdio.h>
#include <stdlib.h>



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



const char* _MsgInternalError   = "Internal error: ";
const char* _MsgAbstractCall    = "Call to abstract method";
const char* _MsgPrecondition    = "Precondition violated: ";
const char* _MsgCheckFailed     = "Check failed: ";



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



static void _CheckFailed (const char* Msg, const char* Cond,
                          int Code, const char* File, int Line);




// The fail vector
#ifdef __GNUC__
volatile
#endif
void (*CheckFailed) (const char*, const char* Cond,
                     int Code, const char* File, int Line) = _CheckFailed;




static void _CheckFailed (const char* Msg, const char* Cond,
                          int Code, const char* File, int Line)
{
    fprintf (stderr, "%s%s", Msg, Cond);
    if (Code) {
        fprintf (stderr, " (= %d), file ", Code);
    } else {
        fprintf (stderr, ", file ");
    }
    fprintf (stderr, "%s, line %d\n", File, Line);
    exit (3);
}



void Check (const char* Msg, const char* Cond,
            int Code, const char* File, int Line)
{
    if (Code != 0) {
        CheckFailed (Msg, Cond, Code, File, Line);
    }
}




