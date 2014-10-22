/*****************************************************************************/
/*                                                                           */
/*                                 KEYDEF.CC                                 */
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



#include <stddef.h>

#include "keydef.h"



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



struct {
    Key Orig;           // Original key
    Key Meta;           // Meta version
} MetaTable [] = {
    {   '0',    kbMeta0         },
    {   '1',    kbMeta1         },
    {   '2',    kbMeta2         },
    {   '3',    kbMeta3         },
    {   '4',    kbMeta4         },
    {   '5',    kbMeta5         },
    {   '6',    kbMeta6         },
    {   '7',    kbMeta7         },
    {   '8',    kbMeta8         },
    {   '9',    kbMeta9         },
    {   'A',    kbMetaA         },
    {   'B',    kbMetaB         },
    {   'C',    kbMetaC         },
    {   'D',    kbMetaD         },
    {   'E',    kbMetaE         },
    {   'F',    kbMetaF         },
    {   'G',    kbMetaG         },
    {   'H',    kbMetaH         },
    {   'I',    kbMetaI         },
    {   'J',    kbMetaJ         },
    {   'K',    kbMetaK         },
    {   'L',    kbMetaL         },
    {   'M',    kbMetaM         },
    {   'N',    kbMetaN         },
    {   'O',    kbMetaO         },
    {   'P',    kbMetaP         },
    {   'Q',    kbMetaQ         },
    {   'R',    kbMetaR         },
    {   'S',    kbMetaS         },
    {   'T',    kbMetaT         },
    {   'U',    kbMetaU         },
    {   'V',    kbMetaV         },
    {   'W',    kbMetaW         },
    {   'X',    kbMetaX         },
    {   'Y',    kbMetaY         },
    {   'Z',    kbMetaZ         }
};



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



Key GetMetaCode (Key K)
// Return the "meta version" of the given key K or kbNoKey if none exists
{
    // Do a linear search (hmm..., is it worth a binary search?)
    for (size_t I = 0; I < sizeof (MetaTable) / sizeof (MetaTable [0]); I++) {
        if (MetaTable [I].Orig == K) {
            return MetaTable [I].Meta;
        }
    }

    // Not found
    return kbNoKey;
}



Key GetMetaKey (Key K)
// Return the "normal key" of the meta key given key K or kbNoKey if none
// exists
{
    // Do a linear search (hmm..., is it worth a binary search?)
    for (size_t I = 0; I < sizeof (MetaTable) / sizeof (MetaTable [0]); I++) {
        if (MetaTable [I].Meta == K) {
            return MetaTable [I].Orig;
        }
    }

    // Not found
    return kbNoKey;
}



