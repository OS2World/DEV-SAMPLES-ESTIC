/*****************************************************************************/
/*									     */
/*				  TEXTSTRM.H				     */
/*									     */
/* (C) 1993-96	Ullrich von Bassewitz					     */
/*		Zwehrenbuehlstrasse 33					     */
/*		D-72070 Tuebingen					     */
/* EMail:	uz@ibb.schwaben.com					     */
/*									     */
/*****************************************************************************/



// $Id$
//
// $Log$
//
//



#ifndef __TEXTSTRM_H
#define __TEXTSTRM_H



#include "stream.h"



/*****************************************************************************/
/*				     Data				     */
/*****************************************************************************/



// Constants for use in InOutFlags
const unsigned tfHandleCtrls	= 0x0003;	// Mask for control handling
const unsigned tfReplaceCtrls	= 0x0001;	// Replace control chars
const unsigned tfDropCtrls	= 0x0002;	// Drop control chars
const unsigned tfReplaceTabs	= 0x0004;	// Replace tabs by spaces



/*****************************************************************************/
/*			     class TextFileStream			     */
/*****************************************************************************/



class TextFileStream: public FileStream {

protected:
    u32*	Index;		// Line index
    u32		Limit;		// Size of Index (in u32's)
    u32		Count;		// Count of lines in the file
    u32		Size;		// File size
    unsigned	TabSize;	// Size of tabs in files

public:
    unsigned	InOutFlags;	// Bitmapped Flags
    char	ReplaceChar;	// Control chars are replaced by this char if
				// tfReplaceCtrls is set


public:
    TextFileStream (const String& Name);
    // Open the stream in read-only mode. A non existent file is considered
    // as an error. InOutFlags is set to DropCtrls | ReplaceTabs, ReplaceChar
    // is set to a blank

    virtual ~TextFileStream ();
    // Close the file, delete the line index (if existant)

    void MakeLineIndex (int LineLen = 50);
    // Make a index into the file, storing the position of the beginning of
    // all lines. This speeds up seeking but consumes memory...
    // LineLen is the estimated length of a line in the file used for
    // estimating the size of the buffer.

    void LineSeek (u32 Line);
    // Seek to a line absolute. The first line has the number 0. If the given
    // number is greater than the number of lines in the file, the seek
    // positions at end of file.

    String GetLine ();
    // Read and return the next line from the file

    u32 LineCount () const;
    // Return the count of lines in the stream

    void SetTabSize (unsigned NewTabSize);
    // Set the new tabulator size (default is fixed 8 chars)

};



inline u32 TextFileStream::LineCount () const
// Return the count of lines in the stream
{
    return Count;
}



// End of TEXTSTRM.H

#endif


