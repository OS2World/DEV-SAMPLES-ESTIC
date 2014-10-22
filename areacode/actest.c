#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "areacode.h"



int main (void)
{
    acInfo AC;
    char Buf [256];
    unsigned RC;
    acMaxMem = 0;

    while (1) {

        printf ("Enter phone number: ");
        fflush (stdout);
        gets (Buf);
        if (strlen (Buf) == 0) {
            break;
        }

        switch ((RC = GetAreaCodeInfo (&AC, Buf))) {

            case acOk:
                printf ("acOK:\n"
                        "  PrefixLen = %d\n"
                        "  Info      = %s\n",
                        AC.AreaCodeLen, AC.Info);
                break;

            case acFileError:
                printf ("acFileError\n");
                break;

            case acInvalidFile:
                printf ("acInvalidFile\n");
                break;

            case acWrongVersion:
                printf ("acWrongVersion\n");
                break;

            default:
                printf ("Unknown return: %u\n", RC);
                break;

        }
        printf ("\n");

    }

    return 0;
}

