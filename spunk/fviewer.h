/*****************************************************************************/
/*									     */
/*				   FVIEWER.H				     */
/*									     */
/* (C) 1993-96	Ullrich von Bassewitz					     */
/*		Wacholderweg 14						     */
/*		D-70597 Stuttgart					     */
/* EMail:	uz@ibb.schwaben.com					     */
/*									     */
/*****************************************************************************/



// $Id$
//
// $Log$
//
//



#ifndef __FVIEWER_H
#define __FVIEWER_H



#include "textstrm.h"
#include "keydef.h"
#include "itemwin.h"



/*****************************************************************************/
/*				     Data				     */
/*****************************************************************************/



const u32 InvalidLine	= 0xFFFFFFFF;	// An invalid line



/*****************************************************************************/
/*			       class FileViewer				     */
/*****************************************************************************/



class FileViewer: public ItemWindow {

private:
    virtual void DrawLine (int Y, int Single = 1);
    // Draw one line at position Y. The line is read from the current
    // position in the file

    int AtStart ();
    // Return true if the first displayed line is at start of file

    int AtEnd ();
    // Return true if the last displayed line is at the end of the file

    void Init ();
    // Called from the constructors


protected:
    TextFileStream*	F;		//
    u32			FirstLine;	// First line in window
    u32			MarkedLine;	// Marked line in window
    i16			HOffset;	// horizontal line offset
    i16			HDelta;		// Amount of chars to skip horizontal
    i16			HLimit;		// Limit for HOffset
    String		HelpKey;	// help keyword
    Rect		ZoomSize;	// Small size for zooming


    virtual u32 GetStatusFlags ();
    // Returns the flags that are used to build the status line in Browse


public:
    FileViewer (const String& Name, const Rect& Bounds,
		u16 aState = wfFramed, u16 aPalette = paGray,
		u16 Number = 0);
    FileViewer (TextFileStream* S, const Rect& Bounds,
		u16 aState = wfFramed, u16 aPalette = paGray,
		u16 Number = 0);

    FileViewer (StreamableInit);
    // Build constructor

    virtual ~FileViewer ();
    // Destroy a fileviewer

    virtual void Store (Stream&) const;
    // Store the object into a stream

    virtual void Load (Stream&);
    // Load the object from a stream

    virtual u16 StreamableID () const;
    // Return the stream ID of the object

    static Streamable* Build ();
    // Return a new, empty object

    virtual void DrawInterior ();
    // Redraw the window interior

    void SetHelpKey (const String& NewKey);
    // Set the help keyword

    const String& GetHelpKey () const;
    // Return the current help keyword

    int HasHelp () const;
    // Return true if the viewer has a valid help key

    // Key handling functions (they are public, because it can be useful
    // to call for example End () before browsing)
    virtual void PgUp ();
    virtual void PgDn ();
    virtual void Down ();
    virtual void Up ();
    virtual void Left ();
    virtual void Right ();
    virtual void Home ();
    virtual void End ();
    virtual void ToTop ();
    virtual void ToBot ();

    virtual void CenterLine (unsigned Line);
    // Center the given line in the viewer (if possible)

    void CenterAndMarkLine (unsigned Line);
    // Center (if possible) and mark the given line

    void ShowLine (unsigned Line);
    // Show the line in the window if it is not already visible

    virtual int LineIsVisible (unsigned Line);
    // Returns 1 if the given line is visible in the window, 0 otherwise

    virtual void HandleKey (Key& K);
    // Key dispatcher used in Browse

    int GetStatus () const;
    // Get the status of the used stream

    int GetErrorInfo () const;
    // Get the error info of the used stream

    const String& GetName () const;
    // Get the file name of the text file

    virtual void Zoom ();
    // Zoom the window
};



inline int FileViewer::AtStart ()
// Return true if the first displayed line is at start of file
{
    return (FirstLine == 0);
}



inline int FileViewer::AtEnd ()
// Return true if the last displayed line is at the end of the file
{
    return (FirstLine + IYSize () >= F->LineCount ());
}



inline int FileViewer::GetStatus () const
// Get the status of the used stream
{
    return F->GetStatus ();
}



inline int FileViewer::GetErrorInfo () const
// Get the error info of the used stream
{
    return F->GetErrorInfo ();
}



inline const String& FileViewer::GetName () const
// Get the file name of the text file
{
    return F->GetName ();
}



inline void FileViewer::SetHelpKey (const String& NewKey)
// Set the help keyword
{
    HelpKey = NewKey;
}



inline const String& FileViewer::GetHelpKey () const
// Return the current help keyword
{
    return HelpKey;
}



inline int FileViewer::HasHelp () const
// Return true if the viewer has a valid help key
{
    return HelpKey.Len () > 0;
}



// End of FVIEWER.H

#endif



