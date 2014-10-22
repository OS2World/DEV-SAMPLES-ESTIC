/*****************************************************************************/
/*                                                                           */
/*                                 FVIEWER.CC                                */
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



#include "streamid.h"
#include "fviewer.h"
#include "progutil.h"



// Register the classes
LINK (FileViewer, ID_FileViewer);



/*****************************************************************************/
/*                             class FileViewer                              */
/*****************************************************************************/



FileViewer::FileViewer (const String& Name, const Rect& Bounds,
                        u16 aState, u16 aPalette,
                        u16 Number) :
    ItemWindow (Bounds, aState, aPalette, Number),
    F (new TextFileStream (Name)),
    FirstLine (0),
    MarkedLine (InvalidLine),
    HOffset (0),
    HDelta (10),
    HLimit (1000)
{
    SetHeader (String (" ") + Name + ' ');
    Init ();
}



FileViewer::FileViewer (TextFileStream* S, const Rect& Bounds,
                        u16 aState, u16 aPalette, u16 Number):
    ItemWindow (Bounds, aState, aPalette, Number),
    F (S),
    FirstLine (0),
    HOffset (0),
    HDelta (10),
    HLimit (1000)
{
    Init ();
}



FileViewer::FileViewer (StreamableInit):
    ItemWindow (Empty)
// Build constructor
{
}



void FileViewer::Init ()
// Caled from the constructors
{
    // Set the zoom size
    ZoomSize = OBounds;
    if (OBounds == Background->GetDesktop ()) {
        // We already have max size, choose a random smaller size
        ZoomSize.Grow (-3, -3);
    }

    // Draw the window contents
    DrawInterior ();
}



FileViewer::~FileViewer ()
{
    // Delete the text file stream
    delete F;
}



void FileViewer::Store (Stream& S) const
// Store the object into a stream
{
    // Call the derived function writing out the Window stuff
    ItemWindow::Store (S);

    // Write out the FileViewer stuff
    S << FirstLine << MarkedLine << HOffset << HDelta << HLimit
      << HelpKey << ZoomSize;

    // Write out the name of the displayed file
    S << F->GetName ();
}



void FileViewer::Load (Stream& S)
// Load the object from a stream
{
    // Call the derived function to load the Window stuff
    ItemWindow::Load (S);

    // Load the FileViewer stuff
    S >> FirstLine >> MarkedLine >> HOffset >> HDelta >> HLimit
      >> HelpKey >> ZoomSize;

    // Read the name of the displayed file
    String Name (Empty);
    S >> Name;

    // Reopen the file
    F = new TextFileStream (Name);

    // If the file is valid, validate FirstLine
    if (F->GetStatus () == stOk) {

        if (FirstLine + IYSize () > F->LineCount ()) {
            // File has changed, FirstLine is invalid
            FirstLine = 0;
        }

    }

    // If the file is valid, draw the window interior
    DrawInterior ();
}



u16 FileViewer::StreamableID () const
// Return the stream ID of the object
{
    return ID_FileViewer;
}



Streamable* FileViewer::Build ()
// Return a new, empty object
{
    return new FileViewer (Empty);
}



void FileViewer::PgUp ()
{
    if (FirstLine != 0) {

        if (FirstLine > IYSize () - 1) {
            FirstLine -= IYSize () - 1;
        } else {
            FirstLine = 0;
        }

        DrawInterior ();

    }
}



void FileViewer::PgDn ()
{
    if (!AtEnd ()) {

        // Scroll window contents up
        FirstLine += IYSize () - 1;
        if (FirstLine > F->LineCount () - IYSize ()) {
            FirstLine = F->LineCount () - IYSize ();
        }

        DrawInterior ();

    }
}



void FileViewer::Down ()
{
    if (!AtEnd ()) {

        // Don't do screen output
        Lock ();

        // Scroll window contents up
        FirstLine++;
        ScrollUp ();

        // Seek to the current starting line in the stream
        F->LineSeek (FirstLine + MaxY ());

        // Display line at the bottom of the window
        DrawLine (MaxY ());

        // Update the screen
        Unlock ();

    }
}



void FileViewer::Up ()
{
    if (!AtStart ()) {

        // Don't do screen output
        Lock ();

        // Scroll window contents down
        FirstLine--;
        ScrollDown ();

        // Seek to the current starting line in the stream
        F->LineSeek (FirstLine);

        // Display line at the top of the window
        DrawLine (0);

        // Update the screen
        Unlock ();

    }
}



void FileViewer::Left ()
{
    if (HOffset > 0) {
        if (HOffset > HDelta) {
            HOffset -= HDelta;
        } else {
            HOffset = 0;
        }
        DrawInterior ();
    }
}



void FileViewer::Right ()
{
    if (HOffset < HLimit) {
        if ((HOffset += HDelta) > HLimit) {
            HOffset = HLimit;
        }
        DrawInterior ();
    }
}



void FileViewer::ToTop ()
{
    if (FirstLine != 0) {
        FirstLine = 0;
        DrawInterior ();
    }
}



void FileViewer::Home ()
{
    if (HOffset != 0 || FirstLine != 0) {
        HOffset = 0;
        FirstLine = 0;
        DrawInterior ();
    }
}



void FileViewer::ToBot ()
{
    if (!AtEnd ()) {
        FirstLine = F->LineCount () - IYSize ();
        DrawInterior ();
    }
}



void FileViewer::End ()
{
    if (HOffset != 0 || (!AtEnd ())) {
        FirstLine = F->LineCount () - IYSize ();
        HOffset = 0;
        DrawInterior ();
    }
}



void FileViewer::CenterLine (unsigned Line)
// Center the given line in the viewer (if possible)
{
    // Set the first line according to the given line
    if (Line < IYSize () / 2) {
        FirstLine = 0;
    } else {
        FirstLine = Line - IYSize () / 2;
    }
    DrawInterior ();
}



void FileViewer::CenterAndMarkLine (unsigned Line)
// Center (if possible) and mark the given line
{
    MarkedLine = Line;
    CenterLine (Line);
}



int FileViewer::LineIsVisible (unsigned Line)
// Returns 1 if the given line is visible in the window, 0 otherwise
{
    return (Line >= FirstLine && Line <= FirstLine + IYSize ());
}



void FileViewer::ShowLine (unsigned Line)
// Show the line in the window if it is not already visible
{
    if (!LineIsVisible (Line)) {
        CenterLine (Line);
    }
}



void FileViewer::HandleKey (Key& K)
{
    switch (K) {
        case vkUp:          Up ();      K = kbNoKey;    break;
        case vkDown:        Down ();    K = kbNoKey;    break;
        case vkLeft:        Left ();    K = kbNoKey;    break;
        case vkRight:       Right ();   K = kbNoKey;    break;
        case vkPgUp:        PgUp ();    K = kbNoKey;    break;
        case vkPgDn:        PgDn ();    K = kbNoKey;    break;
        case vkEnd:         End ();     K = kbNoKey;    break;
        case vkCtrlPgDn:    ToBot ();   K = kbNoKey;    break;
        case vkHome:        Home ();    K = kbNoKey;    break;
        case vkCtrlPgUp:    ToTop ();   K = kbNoKey;    break;
        default:            ItemWindow::HandleKey (K);  break;
    }
}



void FileViewer::DrawLine (int Y, int Single)
// Draw one line at position Y. The line is read from the current position in
// the file
{
    unsigned XSize = IBounds.XSize ();

    // Read the line from the file
    String Line = F->GetLine ().Cut (HOffset, XSize);

    // Convert the line to the internally used character set
    Line.InputCvt ();

    // Determine the line attribute
    int Attr;
    if (MarkedLine != InvalidLine && Y == (int) (MarkedLine - FirstLine)) {
        // Marked line, use invers attribute, write complete line
        Attr = atTextInvers;
        Single = 1;
    } else {
        // Some other line
        Attr = atTextNormal;
    }

    // Pad the line to the window width if needed
    if (Single) {
        // Only one line, no ClrScr(), pad to line length
        Line.Pad (String::Right, XSize);
    }

    // Draw the line
    Write (0, Y, Line, Attr);
}



void FileViewer::DrawInterior ()
// Redraw the window interior
{
    // Lock screen output
    Lock ();

    // Clear the screen
    Clear ();

    // Check the file status, don't do anything if the status is not ok
    if (F->GetStatus () == stOk) {

        // Seek to the current starting line in the stream
        F->LineSeek (FirstLine);

        // Now display all lines in the window
        u32 Cur = FirstLine;
        int LinesToDisplay = (int) (F->LineCount () - Cur);
        if (LinesToDisplay > (int) IYSize ()) {
            LinesToDisplay = IYSize ();
        }
        for (int Y = 0; Y < LinesToDisplay; Y++) {
            // Draw the line
            DrawLine (Y, 0);
        }

    }

    // Unlock the window, allow output
    Unlock ();
}



u32 FileViewer::GetStatusFlags ()
// Returns the flags that are used to build the status line in Browse
{
    u32 Flags = ItemWindow::GetStatusFlags () | siCursPgKeys_Move;
    if (HasHelp ()) {
        Flags |= siHelp;
    }
    return Flags;
}



void FileViewer::Zoom ()
// Zoom the window
{
    // Get the desktop bounds
    Rect Desktop = Background->GetDesktop ();

    // Check if we must zoom in or out
    if (OBounds != Desktop) {
        // Remember the old size, then zoom out
        ZoomSize = OBounds;
        Resize (Desktop);
    } else {
        // Zoom in
        Resize (ZoomSize);
    }
}



