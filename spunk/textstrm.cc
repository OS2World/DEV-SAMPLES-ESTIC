/*****************************************************************************/
/*                                                                           */
/*                                TEXTSTRM.H                                 */
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



#include <string.h>

#include "chartype.h"
#include "textstrm.h"



/*****************************************************************************/
/*                           class TextFileStream                            */
/*****************************************************************************/



TextFileStream::TextFileStream (const String& Name) :
    FileStream (Name, "rb"),
    Index (NULL),
    Limit (0),
    Count (0),
    Size (GetSize ()),
    TabSize (8),
    InOutFlags (tfDropCtrls | tfReplaceTabs),
    ReplaceChar (' ')
// Open the stream in read-only mode. A non existant file is considered
// an error. InOutFlags is set to DropCtrls | ReplaceTabs, ReplaceChar
// is set to a blank
{
    // Create the index for the beginning of all lines
    MakeLineIndex ();
}



TextFileStream::~TextFileStream ()
// Close the file, delete the line index
{
    // Free the index
    delete [] Index;
}



void TextFileStream::MakeLineIndex (int LineLen)
// Make a index into the file, storing the position of the beginning of
// all lines. This speeds up seeking but consumes memory...
// LineLen is the estimated length of a line in the file used for
// estimating the size of the buffer.
{
    static const Delta   = 100;
    static const BufSize = 4096;

    // Estimate the count of lines
    if ((Limit = Size / LineLen) < 100) {
        Limit = 100;
    }

    // Release an eventually allocated old index
    delete [] Index;
    Index = NULL;
    Count = 0;

    // Allocate memory for new index
    u32* I = new u32 [Limit];

    // Calculate line index
    if (Size > 0) {
        I [Count++] = 0;
    }

    //
    char* Buf = new char [BufSize];
    int Fill = 0;
    u32 Bytes = Size;
    u32 BasePos = 0;
    while (Bytes) {

        // Buffer refill
        BasePos += Fill;
        Fill = (Bytes < BufSize) ? Bytes : BufSize;
        Bytes -= Fill;
        Read (Buf, Fill);

#if defined (DOS) || defined (DOS32) || defined (OS2)
        // In case of DOS and OS/2, a Ctrl-Z terminates the file. Bytes is
        // invalid if this happens. Only the last CPM block (128 bytes) can
        // contain a Ctrl-Z.
        if (Bytes < 128) {
            // Read something from the last 128 bytes
            char* CtrlZPos = (char*) memchr (Buf, 0x1A, Fill);
            if (CtrlZPos != NULL) {
                // Found a Ctrl-Z
                Fill = CtrlZPos - Buf;
                Bytes = 0;
            }
        }
#endif

        char* Pos = Buf;
        while (Pos && Pos < &Buf [Fill]) {
            Pos = (char*) memchr ((void*) Pos, '\n', Fill - (Pos - Buf));
            if (Pos) {
                // Found another line begin, skip the lf
                Pos++;
                if (Count >= Limit) {
                    // Index overflow
                    Limit += Delta;
                    u32* P = new u32 [Limit];
                    memcpy (P, I, Count*sizeof (u32));
                    delete [] I;
                    I = P;
                }
                I [Count++] = BasePos + (Pos - Buf);
            }
        }
    }

    // Check if the last char in the buffer is a linefeed. If so, the last
    // index value points to EOF and is invalid. If Size is greater zero,
    // Fill will also be greater zero
    if (Size > 0 && Buf [Fill-1] == '\n') {
        Count--;
    }

    // Delete the buffer
    delete Buf;

    // Assign new index
    Index = I;

}



void TextFileStream::LineSeek (u32 Line)
// Seek to a line absolute. The first line has the number 0. If the given
// number is greater than the number of lines in the file, the seek
// positions at end of file.
{
    if (Line > Count) {
        Line = Count;
    }
    if (Line == Count) {
        Seek (Size);
    } else {
        Seek (Index [Line]);
    }
}



String TextFileStream::GetLine ()
// Read and return the next line from the file
{
    // A char buffer with a fixed length is used for speed
    char Buf [512];
    unsigned Count = 0;

    // Read char by char and stop at end of line
    register int C;
    while (1) {
        C = Read ();
        if (C == '\n' || C == '\r' || C == EOF) {
            break;
        } else if (C == '\t' && InOutFlags & tfReplaceTabs) {
            // Handle tabs
            unsigned Stop = ((Count + TabSize) / TabSize) * TabSize;
            if (Stop > sizeof (Buf) - 1) {
                Stop = sizeof (Buf) - 1;
            }
            while (Count < Stop) {
                Buf [Count++] = ' ';
            }
        } else {

            // Check if there is room left in the buffer
            if (Count < sizeof (Buf) - 1) {

                // Check for control chars
                if (IsCntrl (C)) {

                    // Control char - maybe special handling
                    switch (InOutFlags & tfHandleCtrls) {

                        case tfReplaceCtrls:
                            // Replace control char
                            C = ReplaceChar;
                            break;

                        case tfDropCtrls:
                            // Ignore this char
                            continue;

                    }
                }

                // Add the char to the buffer
                Buf [Count++] = C;
            }
        }
    }

    // Add the trailing zero
    Buf [Count] = '\0';

    // Skip the rest of the line
    if (C == '\r') {
        // Skip until end of line
        do {
            C = Read ();
        } while (C != '\n' && C != EOF);
    }

    // Return the line
    return String (Buf);
}



void TextFileStream::SetTabSize (unsigned NewTabSize)
// Set the new tabulator size (default is fixed 8 chars)
{
    PRECONDITION (NewTabSize > 0 && NewTabSize <= 80);
    TabSize = NewTabSize;
}



