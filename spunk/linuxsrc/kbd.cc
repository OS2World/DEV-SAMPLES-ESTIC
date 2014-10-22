/*****************************************************************************/
/*                                                                           */
/*                                    KBD.CC                                 */
/*                                                                           */
/* (C) 1993-95  Ullrich von Bassewitz                                        */
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



#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <termcap.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/kd.h>

#include "../machine.h"
#include "../msgid.h"
#include "../object.h"
#include "../stack.h"
#include "../charset.h"
#include "../environ.h"
#include "../keymap.h"
#include "../program.h"
#include "../kbd.h"



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



// The one and only keyboard instance
Keyboard* Kbd;



// An instance of class KeyMapper to map char sequences to keys
static KeyMapper        Mapper;



// A charset containing the available extended keys
static CharSet          AvailExtKeys;



// An array for mapping extended to virtual keys
const VirtualMapSize = 50;
struct { Key EK; Key VK; } VirtualMap [VirtualMapSize];
static unsigned VirtualMapCount = 0;



// Terminal settings at startup
static termios StartupSettings;
static int StartupMetaMode;



// Translation table for the ISO8859-1 charset to the IBM codepage 437
unsigned char TT_ISO_8859_1 [256] = {
      0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,    // 0
     16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,    // 1
     32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,    // 2
     48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,    // 3
     64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,    // 4
     80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,    // 5
     96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111,    // 6
    112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,    // 7
     32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,    // 8
     32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,    // 9
     32,173,155,156, 32,157, 32, 32, 32, 32,166,174,170, 32, 32, 32,    // A
    248,241,253, 32, 32,230, 32,249, 32, 32,167,175,172, 32, 32,168,    // B
     32, 32, 32, 32,142,143,146,128, 32,144, 32, 32, 32, 32, 32, 32,    // C
     32,165, 32, 32, 32, 32,153, 32, 32, 32, 32, 32,154, 32, 32,225,    // D
    133,160,131, 32,132,134,145,135,138,130,136,137,141,161,140,139,    // E
     32,164,149,162,147, 32,148,246, 32,151,163, 32,129, 32,176,152     // F
};



unsigned char TT_NOP [256] = {
      0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,    // 0
     16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,    // 1
     32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,    // 2
     48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,    // 3
     64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,    // 4
     80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,    // 5
     96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111,    // 6
    112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,    // 7
    128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,    // 8
    144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,    // 9
    160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,    // A
    176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,    // B
    192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,    // C
    208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,    // D
    224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,    // E
    240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255     // F
};



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



static int KbdIsConsole ()
// Return true if this is a console screen
{
    // This is the console if we can request the state of the keyboard leds
    int Dummy;
    return ioctl (STDIN_FILENO, KDGETLED, &Dummy) == 0;
}



static int KbdCharAvail ()
// Use select() to check for characters
{
    // Timeout is zero
    timeval Timeout;
    Timeout.tv_usec = 0;
    Timeout.tv_sec  = 0;

    // File descriptor is 0 (stdin)
    fd_set Desc;
    FD_ZERO (&Desc);
    FD_SET (STDIN_FILENO, &Desc);

    // Check input status
    return select (STDIN_FILENO+1, &Desc, NULL, NULL, &Timeout);
}



static void KbdInit ()
// Remember the current keyboard settings, then switch to raw mode
{
    // Get current settings
    tcgetattr (STDIN_FILENO, &StartupSettings);

    // switch to raw mode
    termios Settings = StartupSettings;
    Settings.c_iflag &= ~(IGNBRK | IGNCR | INLCR | ICRNL | IUCLC |
                          IXANY | IXON | IXOFF | INPCK | ISTRIP);
    Settings.c_iflag |= (BRKINT | IGNPAR);
    Settings.c_oflag &= ~OPOST;
    Settings.c_lflag &= ~(XCASE | ECHONL | NOFLSH | ICANON | ISIG | ECHO);
    Settings.c_cflag |= CREAD;
    Settings.c_cc[VTIME] = 1;           // wait 100 ms for keys
    Settings.c_cc[VMIN]  = 0;           // return if timeout or keys available
    tcsetattr (STDIN_FILENO, TCSADRAIN, &Settings);

    // If this is the console, remember the old meta setting and switch it
    // to "prefix with escape"
    if (ioctl (STDIN_FILENO, KDGKBMETA, &StartupMetaMode) == 0) {
        // Successful - this is the console
        ioctl (STDIN_FILENO, KDSKBMETA, (void*) K_ESCPREFIX);
    } else {
        // Flag that there is no startup mode
        StartupMetaMode = -1;
    }
}



static void KbdExit ()
// Reset the keyboard to the startup state
{
    // Reset keyboard settings
    tcsetattr (STDIN_FILENO, TCSADRAIN, &StartupSettings);

    // Reset meta mode handling
    if (StartupMetaMode != -1) {
        ioctl (STDIN_FILENO, KDSKBMETA, &StartupMetaMode);
    }
}



static char* KbdGetCap (const char* Cap)
// Get a capability from the termcap file
{
    static char CapBuf [128];
    char* CapPtr = CapBuf;
    return tgetstr (Cap, &CapPtr);
}



static void KbdAddToMap (const char* S, Key K)
// Add the given string to the mapper. If K is an extended key, add it to
// the map of available extended keys.

{
    Mapper.Add (S, K);
    if (S && IsExtendedKey (K)) {
        AvailExtKeys += (K & 0xFF);
    }
}



static void KbdAddCapToMap (const char* Cap, Key K)
// Search the termcap entry for the capability Cap and add it to the Mapper
// using the key K.
{
    KbdAddToMap (KbdGetCap (Cap), K);
}



static int KbdHasKey (Key K)
// Return true if the given extended(!) key is available on the keyboard
{
    return AvailExtKeys [K & 0xFF] != 0;
}



static void KbdAddVMap (Key VK, Key EK)
// Add the mapping to the virtual key map
{
    // Check for overflow
    if (VirtualMapCount >= VirtualMapSize) {
        // Overflow!
        FAIL ("KbdAddVirtualMap: Buffer overflow!");
    }

    // Add the mapping
    VirtualMap [VirtualMapCount].EK = EK;
    VirtualMap [VirtualMapCount].VK = VK;
    VirtualMapCount++;
}



static void KbdAddVMap (Key VK, Key MainKey, Key AltKey)
// If MainKey is available on the keyboard, add a mapping MainKey --> VK. In
// any case, add a mapping AltKey --> VK. Because the latter will be added
// later, a search for a virtual key first finds MainKey as the equivalent
// extended key. If there is no mapping for MainKey, AltKey is found.
{
    // Conditionally add VK --> MainKey
    if (KbdHasKey (MainKey)) {
        // Key exists, add a mapping
        KbdAddVMap (VK, MainKey);
    }

    // Always add VK --> AltKey
    KbdAddVMap (VK, AltKey);
}



static Key KbdMapExtended (Key K)
// Map an extended key to a virtual key. Return the virtual key if a map
// exists, otherwise return the key unchanged.
{
    for (unsigned I = 0; I < VirtualMapCount; I++) {
        if (VirtualMap [I].EK == K) {
            return VirtualMap [I].VK;
        }
    }
    return K;
}



static Key KbdMapVirtual (Key K)
// Map a virtual key to an extended key. Return the extended key if a map
// exists, otherwise return the key unchanged.
{
    for (unsigned I = 0; I < VirtualMapCount; I++) {
        if (VirtualMap [I].VK == K) {
            return VirtualMap [I].EK;
        }
    }
    return K;
}



/*****************************************************************************/
/*                              Class Keyboard                               */
/*****************************************************************************/



Key Keyboard::RawKey ()
{
    // Not used
    return (Key) kbNoKey;
}



Keyboard::Keyboard ():
    Console (KbdIsConsole ()),
    TransTable (new unsigned char [256])
{
    // Switch the keyboard into raw mode
    KbdInit ();

    // Check out the LC_CTYPE environment variable and assign the correct
    // translation table. There are only two possibilities until now, so
    // don't bother.
    switch (GetEnvVal ("LC_CTYPE", String ("0^NONE|1^ISO_8859_1|"))) {

        case 1:
            // ISO 8859-1
            memcpy (TransTable, TT_ISO_8859_1, 256);
            break;

        default:
            // use a 1:1 mapping
           memcpy (TransTable, TT_NOP, 256);

    }

    // The termcap entry has been read before. Read in some key translations
    // and add them to the mapper

    // Function keys
    KbdAddCapToMap ("k1",       kbF1);
    KbdAddCapToMap ("k2",       kbF2);
    KbdAddCapToMap ("k3",       kbF3);
    KbdAddCapToMap ("k4",       kbF4);
    KbdAddCapToMap ("k5",       kbF5);
    KbdAddCapToMap ("k6",       kbF6);
    KbdAddCapToMap ("k7",       kbF7);
    KbdAddCapToMap ("k8",       kbF8);
    KbdAddCapToMap ("k9",       kbF9);
    KbdAddCapToMap ("k0",       kbF10);

    // Cursor keys needed
    KbdAddCapToMap ("kl",       kbLeft);
    KbdAddCapToMap ("kr",       kbRight);
    KbdAddCapToMap ("kd",       kbDown);
    KbdAddCapToMap ("ku",       kbUp);

    // Page up/down, home, end etc.
    KbdAddCapToMap ("kN",       kbPgDn);
    KbdAddCapToMap ("kP",       kbPgUp);
    KbdAddCapToMap ("kh",       kbHome);
    KbdAddCapToMap ("kH",       kbEnd);
    KbdAddCapToMap ("kD",       kbDel);
    KbdAddCapToMap ("kI",       kbIns);

    // Add the meta keys and other sequences to the mapper
    KbdAddToMap ("\033a",       kbMetaA);
    KbdAddToMap ("\033b",       kbMetaB);
    KbdAddToMap ("\033c",       kbMetaC);
    KbdAddToMap ("\033d",       kbMetaD);
    KbdAddToMap ("\033e",       kbMetaE);
    KbdAddToMap ("\033f",       kbMetaF);
    KbdAddToMap ("\033g",       kbMetaG);
    KbdAddToMap ("\033h",       kbMetaH);
    KbdAddToMap ("\033i",       kbMetaI);
    KbdAddToMap ("\033j",       kbMetaJ);
    KbdAddToMap ("\033k",       kbMetaK);
    KbdAddToMap ("\033l",       kbMetaL);
    KbdAddToMap ("\033m",       kbMetaM);
    KbdAddToMap ("\033n",       kbMetaN);
    KbdAddToMap ("\033o",       kbMetaO);
    KbdAddToMap ("\033p",       kbMetaP);
    KbdAddToMap ("\033q",       kbMetaQ);
    KbdAddToMap ("\033r",       kbMetaR);
    KbdAddToMap ("\033s",       kbMetaS);
    KbdAddToMap ("\033t",       kbMetaT);
    KbdAddToMap ("\033u",       kbMetaU);
    KbdAddToMap ("\033v",       kbMetaV);
    KbdAddToMap ("\033w",       kbMetaW);
    KbdAddToMap ("\033x",       kbMetaX);
    KbdAddToMap ("\033y",       kbMetaY);
    KbdAddToMap ("\033z",       kbMetaZ);
    KbdAddToMap ("\033""0",     kbMeta0);
    KbdAddToMap ("\033""1",     kbMeta1);
    KbdAddToMap ("\033""2",     kbMeta2);
    KbdAddToMap ("\033""3",     kbMeta3);
    KbdAddToMap ("\033""4",     kbMeta4);
    KbdAddToMap ("\033""5",     kbMeta5);
    KbdAddToMap ("\033""6",     kbMeta6);
    KbdAddToMap ("\033""7",     kbMeta7);
    KbdAddToMap ("\033""8",     kbMeta8);
    KbdAddToMap ("\033""9",     kbMeta9);

    KbdAddToMap ("\033\x01",    kbEscCtrlA);
    KbdAddToMap ("\033\x02",    kbEscCtrlB);
    KbdAddToMap ("\033\x03",    kbEscCtrlC);
    KbdAddToMap ("\033\x04",    kbEscCtrlD);
    KbdAddToMap ("\033\x05",    kbEscCtrlE);
    KbdAddToMap ("\033\x06",    kbEscCtrlF);
    KbdAddToMap ("\033\x07",    kbEscCtrlG);
    KbdAddToMap ("\033\x08",    kbEscCtrlH);
    KbdAddToMap ("\033\x09",    kbEscCtrlI);
    KbdAddToMap ("\033\x0A",    kbEscCtrlJ);
    KbdAddToMap ("\033\x0B",    kbEscCtrlK);
    KbdAddToMap ("\033\x0C",    kbEscCtrlL);
    KbdAddToMap ("\033\x0D",    kbEscCtrlM);
    KbdAddToMap ("\033\x0E",    kbEscCtrlN);
    KbdAddToMap ("\033\x0F",    kbEscCtrlO);
    KbdAddToMap ("\033\x10",    kbEscCtrlP);
    KbdAddToMap ("\033\x11",    kbEscCtrlQ);
    KbdAddToMap ("\033\x12",    kbEscCtrlR);
    KbdAddToMap ("\033\x13",    kbEscCtrlS);
    KbdAddToMap ("\033\x14",    kbEscCtrlT);
    KbdAddToMap ("\033\x15",    kbEscCtrlU);
    KbdAddToMap ("\033\x16",    kbEscCtrlV);
    KbdAddToMap ("\033\x17",    kbEscCtrlW);
    KbdAddToMap ("\033\x18",    kbEscCtrlX);
    KbdAddToMap ("\033\x19",    kbEscCtrlY);
    KbdAddToMap ("\033\x1A",    kbEscCtrlZ);
    KbdAddToMap ("\033\033",    kbEscEsc);
    KbdAddToMap ("\x11\x13",    kbCtrlQS);
    KbdAddToMap ("\x11""s",     kbCtrlQS);
    KbdAddToMap ("\x11""S",     kbCtrlQS);
    KbdAddToMap ("\x11\x04",    kbCtrlQD);
    KbdAddToMap ("\x11""d",     kbCtrlQD);
    KbdAddToMap ("\x11""D",     kbCtrlQD);
    KbdAddToMap ("\x11\x12",    kbCtrlQR);
    KbdAddToMap ("\x11""r",     kbCtrlQR);
    KbdAddToMap ("\x11""E",     kbCtrlQR);
    KbdAddToMap ("\x11\x03",    kbCtrlQC);
    KbdAddToMap ("\x11""c",     kbCtrlQC);
    KbdAddToMap ("\x11""C",     kbCtrlQC);
    KbdAddToMap ("\x11\x05",    kbCtrlQC);
    KbdAddToMap ("\x11""e",     kbCtrlQC);
    KbdAddToMap ("\x11""E",     kbCtrlQC);
    KbdAddToMap ("\x11\x18",    kbCtrlQX);
    KbdAddToMap ("\x11""x",     kbCtrlQX);
    KbdAddToMap ("\x11""X",     kbCtrlQX);

    // Now create the mappings extended --> virtual (primary/secondary)
    KbdAddVMap (vkHelp,         kbF1,           kbEscCtrlH);
    KbdAddVMap (vkPgDn,         kbPgDn,         kbCtrlC);
    KbdAddVMap (vkPgUp,         kbPgUp,         kbCtrlR);
    KbdAddVMap (vkUp,           kbUp,           kbCtrlE);
    KbdAddVMap (vkDown,         kbDown,         kbCtrlX);
    KbdAddVMap (vkLeft,         kbLeft,         kbCtrlS);
    KbdAddVMap (vkRight,        kbRight,        kbCtrlD);
    KbdAddVMap (vkIns,          kbIns,          kbCtrlV);
    KbdAddVMap (vkDel,          kbDel,          kbCtrlG);
    KbdAddVMap (vkHome,         kbHome,         kbCtrlQS);
    KbdAddVMap (vkEnd,          kbEnd,          kbCtrlQD);
    KbdAddVMap (vkZoom,         kbF5,           kbEscCtrlZ);
    KbdAddVMap (vkClose,        kbMetaF3,       kbEscCtrlC);
    KbdAddVMap (vkOpen,         kbF3,           kbEscCtrlO);
    KbdAddVMap (vkSave,         kbF2,           kbEscCtrlS);
    KbdAddVMap (vkAccept,       kbF10,          kbEscCtrlA);

    // Keys that have no secondary form under linux
    KbdAddVMap (vkCtrlUp,       kbCtrlW);
    KbdAddVMap (vkCtrlDown,     kbCtrlZ);
    KbdAddVMap (vkCtrlLeft,     kbCtrlA);
    KbdAddVMap (vkCtrlRight,    kbCtrlF);
    KbdAddVMap (vkCtrlPgUp,     kbCtrlQR);
    KbdAddVMap (vkCtrlPgDn,     kbCtrlQC);
    KbdAddVMap (vkCtrlHome,     kbCtrlQE);
    KbdAddVMap (vkCtrlEnd,      kbCtrlQX);
    KbdAddVMap (vkResize,       kbEscCtrlR);
    KbdAddVMap (vkQuit,         kbMetaX);

    // Handle the abort key separate
    KbdAddVMap (vkAbort, IsConsole () ? kbEsc : kbEscEsc);

    // Map the character that c_cc[VERASE] sends to backspace (bug?
    // is c_cc signed?)
    if (StartupSettings.c_cc [VERASE] > 0 &&
        StartupSettings.c_cc [VERASE] <= 256) {
        TransTable [StartupSettings.c_cc [VERASE]] = (unsigned char) kbBack;
    } else {
        // It seems that the setting is invalid, check the termcap database
        // for a kb entry. If the entry is only one char, translate it via
        // the translation table, otherwise use the mapper
        char* BS = KbdGetCap ("kb");
        if (BS) {
            if (strlen (BS) == 1) {
                // Just one char
                TransTable [(unsigned char) (*BS)] = (unsigned char) kbBack;
            } else {
                // A sequence...
                KbdAddToMap (BS, kbBack);
            }
        }
    }
}



Keyboard::~Keyboard ()
{
    // Reset the keyboard state
    KbdExit ();

    // Delete the translation table
    delete [] TransTable;
}



void Keyboard::GetMappedKey (int Wait)
// Read keys until the needed key is not found in the mapper or until
// a valid sequence is found. If Wait is zero, return immediately if
// no match is found and no more keys are available.
{
    static int BufFill = 0;
    static unsigned char Buf [64] = "";

    // The handling is different, if we are on the console or not. On the
    // console, we can easily find a complete key sequence, because a
    // complete sequence is returned by read. This allows usage of the alt
    // keys.
    // On a terminal, we have to search for each sequence.


    if (IsConsole ()) {

        // This is the console. If we should not wait, first check if any
        // characters are available, if not, bail out
        if (Wait == 0 && KbdCharAvail () == 0) {
            // Nothing to do
            return;
        }

        // Now loop until we have a valid key
        while (1) {

            // Read in a complete sequence from the keyboard
            do {
                BufFill = read (0, Buf, sizeof (Buf)-1);
                if (BufFill == 0) {
                    // Timeout waiting for a key, allow some idle processing
                    App->Idle ();
                }
            } while (BufFill <= 0);
            Buf [BufFill] = '\0';

            // If there is only one key in the buffer, read it directly, otherwise
            // ask the mapper.
            Key K;
            if (BufFill == 1) {

                // Remap the key
                K = KbdMapExtended (Translate (Buf [0]));

                // Put the mapped key into the buffer and return
                KeyBuf.Put (K);
                return;

            } else {

                // There is more than one char in the buffer. This must be a complete
                // key sequence. Try to find the mapping for that key, insert the
                // mapped key. If we do not find a mapping, this is a key sequence,
                // we don't know, so throw it away.
                int Index;
                if (Mapper.Search ((char*) Buf, Index)) {
                    // Found!
                    K = KbdMapExtended (Mapper [Index]->GetKey ());
                    KeyBuf.Put (K);
                    return;
                }
            }

        }

    } else {

        // Not the console. If we have keys left in the buffer, try to find a
        // match. If we have a partial match, read more chars, until we get
        // a full or no match.
        char SBuf [sizeof (Buf)];
        int SCount = 1;

        while (1) {

            while (SCount <= BufFill) {
                // Add the next char from Buf to SBuf, add the trailing zero
                SBuf [SCount-1] = Buf [SCount-1];
                SBuf [SCount] = '\0';

                // Now search for this portion of Buf
                int Index;
                Key K;
                switch (Mapper.Find (SBuf, Index)) {

                    case 0:
                        // No match. Return the translated equivalent of the
                        // first char in Buf, then delete it from Buf.
                        K = KbdMapExtended (Translate (Buf [0]));
                        memmove (&Buf [0], &Buf [1], BufFill);
                        BufFill--;              // One char less
                        KeyBuf.Put (K);
                        return;

                    case 1:
                        // Partial match. Increase the length of the sequence,
                        // we search for. If the length succeeds some arbitary
                        // limit, handle it like no match.
                        if (SCount < 16) {
                            SCount++;
                        } else {
                            // Suspicious... Return the translated equivalent
                            // of the first char in Buf, then delete it from
                            // Buf.
                            K = KbdMapExtended (Translate (Buf [0]));
                            memmove (&Buf [0], &Buf [1], BufFill);
                            BufFill--;              // One char less
                            KeyBuf.Put (K);
                            return;
                        }
                        break;

                    case 2:
                        // Full match. Delete the sequence from the buffer.
                        memmove (&Buf [0], &Buf [SCount], BufFill - SCount + 1);
                        BufFill -= SCount;
                        K = KbdMapExtended (Mapper [Index]->GetKey ());
                        KeyBuf.Put (K);
                        return;

                }
            }

            // If we get here, the buffer is exhausted and we still have a
            // partial match (or the buffer has been empty on startup).
            // If we should read without a wait, check if there are characters
            // waiting. If not, don't do a blocking read but return without
            // doing anything.
            if (Wait == 0 && KbdCharAvail () == 0) {
                // No chars waiting
                return;
            }

            // Before trying to read more chars, check for a buffer overflow
            // (this should not happen, but who knows...)
            if (SCount >= (int) sizeof (Buf)) {
                FAIL ("Keyboard::GetMappedKey: Buffer overflow");
            }

            // Now read in a new chunk of chars.
            int Count;
            do {
                Count = read (0, &Buf [BufFill], sizeof (Buf) - BufFill - 1);
                if (Count == 0) {
                    // Timeout waiting for a key, allow some idle processing
                    App->Idle ();
                }
            } while (Count <= 0);
            BufFill += Count;
            Buf [BufFill] = '\0';

        }

    }

}



String Keyboard::GetKeyName (Key K)
// Return a string describing the give key
{
    if (IsVirtualKey (K)) {
        // It is a virtual key, remap it to it's extended form
        K = KbdMapVirtual (K);
    }

    // Now return the key description
    if (IsConsole ()) {
        return App->LoadMsg (MSGBASE_KBD + K);
    } else {
        return App->LoadMsg (MSGBASE_KBD + 0x200 + K);
    }

}


