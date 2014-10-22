/*****************************************************************************/
/*                                                                           */
/*                                ICCPRINT.CC                                */
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



// Print the charges



#include <stdio.h>
#include <errno.h>

#include "str.h"
#include "syserror.h"
#include "datetime.h"
#include "progutil.h"

#include "icident.h"
#include "iccom.h"
#include "iclog.h"



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



String PrintCharges (const String& Filename)
// Print the charges and return an error string. If all is ok, the empty
// String is returned.
{
    // Open the file
    FILE* F = fopen (Filename.GetStr (), "wt");
    if (F == NULL) {
        // An error
        return GetSysErrorMsg (errno);
    }

    // Ok, loop through the devices...
    for (unsigned Dev = 0; Dev < IstecDevCount; Dev++) {
        unsigned Units = Charges [Dev];
        fprintf (F, "%d %4d %6.2f\n", Dev +21, Units, Units * PricePerUnit);
    }

    // Close the file
    fclose (F);

    // Return success
    return "";
}



