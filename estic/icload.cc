/*****************************************************************************/
/*                                                                           */
/*                                 ICLOAD.CC                                 */
/*                                                                           */
/* (C) 1995     Ullrich von Bassewitz                                        */
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



#include <stdio.h>
#include <stdlib.h>

#include "delay.h"
#include "str.h"
#include "filepath.h"

#include "icconfig.h"
#include "iccom.h"



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



static String ComPortName = "COM2";
static IstecBaseConfig BaseConfig;
static IstecDevConfig DevConfig;



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



static void EvalCmd (int RetCode)
// Evaluate the return code of an istec command function. If the return
// code denotes an error, an error message is printed on stderr and the
// program is terminated.
{
    switch (RetCode) {

        case ieRecBufOverflow:
            // Receive buffer overflow
            fprintf (stderr, "Receive buffer overflow (internal icload error)\n");
            exit (2);

        case ieRecBufUnderflow:
            // Receive buffer underflow
            fprintf (stderr, "Receive buffer underflow (internal icload error)\n");
            exit (2);

        case ieInvalidReply:
            // Invalid reply
            fprintf (stderr, "Invalid reply from the ISTEC\n");
            exit (2);

        case ieWrongDevice:
            // Wrong device number in reply
            fprintf (stderr, "Wrong device number in reply\n");
            exit (2);

        case iePortNotOpen:
            // COM port not open
            fprintf (stderr, "Serial device is not open (internal icload error)\n");
            exit (2);

        case ieTimeout:
            // Timeout
            fprintf (stderr, "Communication timeout\n");
            exit (1);

        case ieDone:
            // ok
            break;

        default:
            fprintf (stderr, "EvalCmd: Unexpected return code\n");
            exit (3);

    }
}



static void LoadFile (String Name)
// Load the configuration from a file
{
    // Add the default extension to the file name
    AddDefaultExtension (Name, ".ic");

    // Open the file
    FILE* F = fopen (Name.GetStr (), "rb");
    if (F == NULL) {
        fprintf (stderr, "Error opening %s\n", Name.GetStr ());
        exit (1);
    }

    // Read the configuration
    fread (&BaseConfig, sizeof (BaseConfig), 1, F);
    fread (&DevConfig, sizeof (DevConfig), 1, F);

    // Close the file
    fclose (F);
}



static void ResetCharges ()
// Reset all charges
{
    // Create an empty charges object
    IstecCharges Charges;

    // Send the istec command
    EvalCmd (IstecPutCharges (Charges));
}



static int GetNewCharges ()
// Request and wait for an update of the charge info. Return a result code.
{
    // Reset the flag
    ChargeUpdate = 0;

    // Send the command
    int Result = IstecRequestCharges ();

    if (Result == ieDone) {

        // Wait for the new charges
        unsigned I = 0;
        do {
            Delay (100);
            IstecPoll ();
        } while (++I < 40 && ChargeUpdate == 0);

        // Check for a timeout
        if (ChargeUpdate == 0) {
            // Timeout, pop up a message
            Result = ieTimeout;
        } else {
            Result = ieDone;
        }
    }

    return Result;
}



static int LoadConfig (IstecBaseConfig& BC, struct DevConfig* DC)
// Load all config files and the charges from the istec
{
    // Load the config stuff from the istec
    int Result = IstecGetConfig (BC, DC);

    // If we could get the configuration, read also the charges
    if (Result == ieDone) {
        Result = GetNewCharges ();
    }

    // Return the result
    return Result;
}



static void Usage ()
// Print usage information and exit
{
    fprintf (stderr, "ICLOAD - Load ISTEC configuration\n"
                     "(C) 1995 Ullrich von Bassewitz <uz@ibb.schwaben.com>\n"
                     "\n"
                     "Usage: icload [options] [file]\n"
                     "\n"
                     "Options:\n"
                     "\t-aADR   Set port base address (DOS only)\n"
                     "\t-c      Print charges on stdout\n"
                     "\t-iIRQ   Set port IRQ (DOS only)\n"
                     "\t-n      Don't make new config permanent\n"
                     "\t-pNAME  Set name of serial device\n"
                     "\t-r      Reset charges\n");
    exit (1);
}



int main (int argc, char* argv [])
{
    // Command line options
    const char* ConfigFile = 0;
    int optPrintCharges = 0;
    int optResetCharges = 0;
    int optNotPermanent = 0;


    // Parse the command line
    int I = 1;
    while (I < argc) {

        char* Item = argv [I];
        if (*Item == '-') {

            Item++;
            switch (*Item) {

                case 'a':
                    // Set port address
                    PortBase = atoi (++Item);
                    break;

                case 'c':
                    // Print charges
                    optPrintCharges = 1;
                    break;

                case 'i':
                    // Set port irq
                    PortIRQ = atoi (++Item);
                    break;

                case 'n':
                    // Don't make changes permanent
                    optNotPermanent = 1;
                    break;

                case 'p':
                    // Set port name
                    ComPortName = ++Item;
                    break;

                case 'r':
                    // Clear the charges
                    optResetCharges = 1;
                    break;

                default:
                    // Print usage information and exit
                    Usage ();
                    break;

            }

        } else {

            // Configuration file name
            if (ConfigFile) {
                // Already got a filename
                fprintf (stderr, "Duplicate file name: %s\n", Item);
                exit (1);
            } else {
                ConfigFile = Item;
            }

        }

        // Next argument
        I++;
    }


    // Open the port
    if (OpenComPort (ComPortName) != 0) {
        // Port could not be opened
        fprintf (stderr, "Could not open serial port!\n");
        exit (1);
    }

    // The com port is open. Make shure, it is closed on exit
    atexit (CloseComPort);

    // Com port could be opened, check for the istec
    EvalCmd (IstecReady ());

    // Load the current configuration and charges from the istec
    IstecBaseConfig CurrentBaseConfig;
    IstecDevConfig CurrentDevConfig;
    EvalCmd (LoadConfig (CurrentBaseConfig, CurrentDevConfig));

    // Ok, now:
    //
    //  1. If a config file is specified, send that file to the istec.
    //  2. If an output of the device charges is requested, do that.
    //  3. If the charges should be cleared, do that.
    //  4. If MakePermanent is not switched off, _and_ if a new config
    //     file was sent to the istec, make the config permament.
    //

    // 1.
    if (ConfigFile != NULL) {
        // Load a config file into memory
        LoadFile (ConfigFile);

        // Send the stuff to the istec
        EvalCmd (IstecPutConfig (BaseConfig, DevConfig,
                                 CurrentBaseConfig.AB_InterfaceCount));

    }

    // 2.
    //
    if (optPrintCharges) {
        for (unsigned I = 0; I < CurrentBaseConfig.AB_InterfaceCount; I++) {
            printf ("%d ", Charges [I]);
        }
        printf ("\n");
    }

    // 3.
    if (optResetCharges) {
        ResetCharges ();
    }

    // 4.
    if (ConfigFile != 0 && optNotPermanent == 0) {
        EvalCmd (IstecMakePermanent ());
    }

    // Close the com port
    CloseComPort ();

    // Success
    return 0;
}










