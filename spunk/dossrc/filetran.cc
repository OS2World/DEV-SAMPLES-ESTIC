/******************************************************************************/
/*                                                                            */
/* Beispielprogramm zur Demonstration der Verwendung des Moduls U-SER.        */
/* Verwendet zwei serielle Schnittstellen und �bertr�gt eine als Parameter    */
/* angegebene Text-Datei von einer Schnittstelle zur anderen und zeigt sie    */
/* dann auf dem Bildschirm an.                                                */
/*                                                                            */
/*  (C) 1993 by Ullrich von Bassewitz                                         */
/*              Zwehrenb�hlstra�e 33                                          */
/*              7400 T�bingen                                                 */
/*                                                                            */
/*                                                                            */
/*  �nderungen:                                                               */
/*                                                                            */
/******************************************************************************/



#include <stdlib.h>
#include <stdio.h>
#include <dos.h>

#include "sercom.h"


#define TEST
#define HW_HANDSHAKE

#ifndef TEST
/* "Normale" Konfiguration */

#define COM1     "COM1"
#define COM2     "COM2"
#define BufSize  512
#define Baud     19200L         /* H�chste Baudrate ohne FIFO */

#else
/* Test-Konfiguration */

#define COM1     "COM2"
#define COM2     "COM3"         /* COM 3 auf INT 0x70 */
#define BufSize  512
#define Baud     9600L          /* Nur mit FIFO ! */

#endif



ComPort* SendPort = NULL;
ComPort* RecPort  = NULL;



void Usage (void)
{
    fprintf (stderr, "Syntax: FILETRAN <Text-Datei>\n");
#ifdef HW_HANDSHAKE
    fprintf (stderr, "Voraussetzung: COM1 und COM2 m�ssen vorhanden, und\n"
                     "�ber ein Nullmodem-Kabel verbunden sein.\n"
                     "Das Programm verwendet RTS/CTS Handshake und sendet\n"
                     "deshalb nicht, wenn das Kabel nicht korrekt verdrahtet\n"
                     "ist. In diesem Fall sollte ein anderes Kabel oder ein\n"
                     "anderer Handshake (XON/XOFF) verwendet werden...\n");
#else
    fprintf (stderr, "Voraussetzung: COM1 und COM2 m�ssen vorhanden, und\n"
                     "�ber ein Kabel verbunden sein, bei dem die Adern 2\n"
                     "und 3 gekreuzt sind.\n"
                     "Das Programm verwendet XON/XOFF Handshake und sendet\n"
                     "deshalb auch dann, wenn das Kabel fehlt. In diesem\n"
                     "Fall erfolgt keine Bildschirmausgabe...\n");
#endif
    exit (1);
}




void ExitFunc (void)
/* Schlie�t offene Port wenn das Programm endet */
{
    delete RecPort;
    delete SendPort;
}







int main (int argc, char *argv [])
{
    FILE *F;
    int C;
    int T;

    /* Parameter �berpr�fen */
    if (argc != 2) {
        Usage ();
    }

    /* Datei zur �bertragung �ffnen */
    if ((F = fopen (argv [1], "rt")) == NULL) {
        fprintf (stderr, "Fehler beim �ffnen von %s\n", argv [1]);
        exit (1);
    }

    /* Parameter der beiden Ports einstellen */
#ifdef HW_HANDSHAKE
    char Connection = 'M';   /* H/W Handshake */
    char XonXoff    = 'D';   /* kein XON/XOFF verwenden */
#else
    char Connection = 'D';   /* Kein H/W Handshake */
    char XonXoff    = 'E';   /* XON/XOFF verwenden */
#endif
    char Parity     = 'N';   /* Keine Parit�t */
    char StopBits   = 1;     /* 1 Stopbit */
    char DataBits   = 8;     /* 8 Datenbits */

#ifdef TEST
    /*
     * Beispiel f�r die Verwendung anderer Interrupts: Der hier verwendete
     * COM3 sitzt auf einer vierfach SIO-Karte und ist auf Interrupt 12
     * gejumpert. Interrupt 12 entspricht der Interruptvektor 0x74. Da
     * diese Leitung am zweiten Interrupt-Controller sitzt, ist die Maske
     * f�r den ersten Controller leer (0x00 bzw. 0xFF), die Maske f�r den
     * zweiten Controller berechnet sich nach
     *
     *          IC2Mask    = 0x01 << (IntNr - 0x70);
     *          NotIC2Mask = ~IC2Mask;
     *
     * Siehe dazu auch die Anmerkungen im Handbuch.
     *
     */
    unsigned IntNr      = 0x70;
#else
    unsigned IntNr      = 0x00;
#endif


    /* Ports installieren */
    SendPort = new ComPort (COM1, Baud, DataBits, Parity, StopBits,
                            Connection, XonXoff, 0, 0);
    RecPort = new ComPort (COM2, Baud, DataBits, Parity, StopBits,
                            Connection, XonXoff, 0, IntNr);
    if (SendPort->Open () != 0) {
        /* Port nicht verf�gbar */
        fprintf (stderr, "Keinen UART f�r %s gefunden!\n",
                 COM1);
        exit (1);
    }
    if (RecPort->Open () != 0) {
        /* Port nicht verf�gbar */
        fprintf (stderr, "Keinen UART f�r %s gefunden!\n",
                 COM2);
        exit (1);
    }

    /* Exit-Funktion installieren */
    atexit (ExitFunc);

    /* Wenn Hardware-Handshake gew�hlt ist, m�ssen noch die Statusleitungen
     * der beiden Ports aktiv gemacht werden. Das ist bei Verwendung des
     * Software-Handshakes nicht notwendig.
     */
#ifdef HW_HANDSHAKE
    SendPort->DTROn ();
    RecPort->DTROn ();

    /* Warten bis die Statusleitungen des Empf�ngers ok sind */
    for (T = 0; T < 30; T++) {
        unsigned Status = SendPort->ModemStatus ();
        const HandShake = csClearToSend | csDataSetReady;
        if ((Status & HandShake) == HandShake) {
            /* Status ist Ok */
            break;
        } else {
            /* Nicht ok, warten */
            delay (100);
            T++;
        }
    }
    /* Timeout pr�fen */
    if (T == 30) {
        fprintf (stderr, "Statusleitungen nicht ok, Senden nicht m�glich.\n"
                         "Ben�tigt wird: CTS aktiv und DSR aktiv,\n"
                         "Status ist:    CTS %s und DSR %s.\n",
                         SendPort->ModemStatus () & csClearToSend ?
                         "aktiv" : "inaktiv",
                         SendPort->ModemStatus () & csDataSetReady ?
                         "aktiv" : "inaktiv");
        exit (1);
    }
#endif

    /* Datei �bertragen */
    while ((C = getc (F)) != EOF) {
        /* Warten bis Platz im Ausgabepuffer ist. */
        for (T = 0; T < 60; T++) {
            if (SendPort->TXFree () == 0) {
                /* Kein Platz, etwas warten */
                delay (50);
            } else {
                /* Platz, senden */
                break;
            }
        }

        /* Timeout pr�fen und Zeichen senden */
        if (T == 60) {
            fprintf (stderr, "Timeout beim Senden\n");
            fclose (F);
            exit (1);
        }
        SendPort->Send (C);

        /* Falls Zeichen im Empfangspuffer sind, abholen und auf dem
         * Bildschirm ausgeben.
         */
        while (RecPort->RXCount () > 0) {
            /* Es sind welche da */
            putchar (RecPort->Receive ());
        }
    }

    /* Ganz wichtig ! Obwohl die Datei zu Ende ist, befinden sich evtl. (bzw.
     * eher mit Sicherheit) noch Zeichen in den Sende- und Empfangspuffern.
     * Es mu� daher gewartet werden bis die Puffer beide geleert sind.
     * Nochmal ACHTUNG: Wird ein UART mit FIFO verwendet, so kann es
     * passieren, da� sowohl im Empfangs-, als auch im Sendepuffer keine
     * Zeichen mehr sind, die Datei aber trotzdem nicht vollst�ndig �ber-
     * tragen wurde. Das liegt daran, da� der UART die Zeichen im FIFO erst
     * mit einer Verz�gerung herausgibt, die die Dauer von 4 Zeichen betr�gt.
     * Deshalb am besten immer etwas l�nger warten...
     */
    delay (BufSize * (10000.0 / (double) Baud) + 100);
    while (RecPort->RXCount () > 0) {
        /* Es sind welche da */
        putchar (RecPort->Receive ());
    }

    /* Datei schlie�en */
    fclose (F);

    /* Statusleitungen inaktiv */
#ifdef HW_HANDSHAKE
    SendPort->DTROff ();
    RecPort->DTROff ();
#endif

    /* Ports werden �ber ExitFunc geschlossen */
    return 0;

}
