/*****************************************************************************/
/*                                                                           */
/*                                  STREAM.CC                                */
/*                                                                           */
/* (C) 1993-96  Ullrich von Bassewitz                                        */
/*              Wacholderweg 14                                              */
/*              D-70597 Stuttgart                                            */
/* EMail:       uz@musoftware.com                                            */
/*                                                                           */
/*****************************************************************************/



// $Id$
//
// $Log$
//
//



#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#if defined (DOS) || defined (DOS32) || defined (OS2)
#  include <io.h>
#else
#  include <unistd.h>
#endif

#include "cpucvt.h"
#include "check.h"
#include "stream.h"




/*****************************************************************************/
/*                      Explicit template instantiation                      */
/*****************************************************************************/



#ifdef EXPLICIT_TEMPLATES
template class Collection<StreamableClass>;
template class SortedCollection<StreamableClass, u16>;
#endif



/*****************************************************************************/
/*                          class StreamableClasses                          */
/*****************************************************************************/



class StreamableClasses : private SortedCollection<StreamableClass, u16> {

    friend class StreamableClass;
    friend class Stream;

public:
    // Constructor
    StreamableClasses ();

    virtual int Compare (const u16* Key1, const u16* Key2);
    virtual const u16* KeyOf (const StreamableClass* Item);
    StreamableClass* Lookup (u16 ID);

};



// Database of all registered classes
static StreamableClasses* ClassBase = NULL;



StreamableClasses::StreamableClasses () :
        SortedCollection<StreamableClass, u16> (50, 25)
{
}



int StreamableClasses::Compare (const u16* Key1, const u16* Key2)
{
    if (*Key1 > *Key2) {
        return 1;
    } else if (*Key1 < *Key2) {
        return -1;
    } else {
        return 0;
    }
}



StreamableClass* StreamableClasses::Lookup (u16 ID)
{
    // This is not a virtual function so we can safely check "this" against
    // NULL. Return "Not found" (== NULL) in this case.
    if (this == NULL) {
        // The object does not exist
        return NULL;
    }

    // The object exists, search for the ID
    int Index;
    if (Search (&ID, Index)) {
        // Found, return pointer
        return At (Index);
    } else {
        // Not found
        return NULL;
    }
}



const u16* StreamableClasses::KeyOf (const StreamableClass* S)
{
    return &S->ID;
}



/*****************************************************************************/
/*                           class StreamableClass                           */
/*****************************************************************************/



StreamableClass::StreamableClass (Streamable* (BuildFunc) (), u16 ClassID) :
        ID (ClassID), Builder (BuildFunc)
{
    // Check if ClassBase already exists, if not, create it
    if (ClassBase == NULL) {
        // Create a ClassBase and insert the object
        ClassBase = new StreamableClasses;
        ClassBase->Insert (this);
    } else {
        // Check duplicates before inserting
        int Index;
        if (ClassBase->Search (&ClassID, Index)) {
            FAIL ("StreamableClass::StreamableClass: Duplicate class ID!");
        }
        // Insert self into database
        ClassBase->Insert (this);
    }
}



/*****************************************************************************/
/*                               class Stream                                */
/*****************************************************************************/



Stream::Stream () :
    BitCount (0),
    BitBuf (0),
    Status (0),
    ErrorInfo (0),
    StreamError (NULL)
{
}



void Stream::BitWrite (u32 Bits, unsigned Count)
// Write the given bits to the stream, LSB first. Count may not be
// greater than 32.
{
    static const unsigned char Mask [8] = {
        0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF
    };

    // Check the parameters
    PRECONDITION (Count <= CHAR_BIT * sizeof (Bits));

    // Write the bits
    while (Count) {

        // Flush the buffer if needed
        if (BitCount == CHAR_BIT) {
            BitFlush ();
        }

        // Calculate the count of bits to write within this chunk
        unsigned Chunk = Count;
        if (Chunk > CHAR_BIT - BitCount) {
            Chunk = CHAR_BIT - BitCount;
        }
        Count -= Chunk;

        // Put the bits into the buffer
        BitBuf |= (Bits & Mask [Chunk-1]) << BitCount;
        BitCount += Chunk;
        Bits >>= Chunk;

    }
}



u32 Stream::BitRead (unsigned Count)
// Read some bits from the stream. Count may not be greater than 32.
{
    static const unsigned char Mask [8] = {
        0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF
    };

    // Check the parameters
    PRECONDITION (Count <= CHAR_BIT * sizeof (u32));

    // Read the bits
    unsigned Val = 0;
    unsigned BitsRead = 0;
    while (Count) {

        // Read a new byte if needed
        if (BitCount == 0) {
            Read (&BitBuf, sizeof (BitBuf));
            BitCount = CHAR_BIT;
        }

        // Read B->Bits at max.
        unsigned Bits = Count;
        if (Bits > BitCount) {
            Bits = BitCount;
        }
        Count -= Bits;
        BitCount -= Bits;

        // Read the bits and put them into the result buffer
        Val |= (BitBuf & Mask [Bits-1]) << BitsRead;
        BitsRead += Bits;
        BitBuf >>= Bits;

    }

    // Return the bits read
    return Val;
}



void Stream::BitFlush ()
// Flush the bit buffer if needed.
{
    if (BitCount) {
        Write (&BitBuf, sizeof (BitBuf));
        BitBuf   = 0;
        BitCount = 0;
    }
}



void Stream::Flush ()
{
}



u32 Stream::GetPos ()
{
    ABSTRACT ();
    return 0L;
}



u32 Stream::GetSize ()
{
    ABSTRACT ();
    return 0L;
}



void Stream::Reset ()
{
    Status = stOk;
    ErrorInfo = 0;
}



void Stream::Read (void*, size_t)
{
    ABSTRACT ();
}



int Stream::Read ()
{
    ABSTRACT ();
    return 0;
}



void Stream::Seek (unsigned long)
{
    ABSTRACT ();
}



void Stream::SeekToEnd ()
// Set the stream pointer to the end of the stream
{
    Seek (GetSize ());
}



Stream::ErrorFunc Stream::SetErrorFunc (ErrorFunc F)
// Set a new StreamError function
{
    ErrorFunc OldFunc = StreamError;
    StreamError = F;
    return OldFunc;
}



void Stream::Truncate ()
{
    ABSTRACT ();
}



void Stream::Write (const void*, size_t)
{
    ABSTRACT ();
}



void Stream::CopyFrom (Stream& S, size_t Count)
{
    static const BufSize = 4096;            // Size of copy buffer


    // Check parameter
    PRECONDITION (Count > 0);

    // Check stream status
    if (Status != stOk) {
        return;
    }
    if (S.GetStatus () != stOk) {
        Error (stCopyError, S.GetStatus ());
        return;
    }

    // Allocate buffer memory
    char *Buffer = new char [BufSize];

    // Do the copy...
    size_t Bytes;
    while (Count) {
        Bytes = (Count > BufSize) ? BufSize : Count;
        S.Read (Buffer, Bytes);
        Write (Buffer, Bytes);
        Count -= Bytes;
    }

    // Free the allocated buffer
    delete [] Buffer;

}



Streamable* Stream::Get ()
{
    u16 ID;
    StreamableClass* C;


    // Check stream status
    if (Status != stOk) {
        return NULL;
    }

    // Read ID
    *this >> ID;

    // An ID of zero means a NULL object has been stored
    if (Status != stOk || ID == 0) {
        return NULL;
    }

    // Search the ID in the classbase
    if ((C = ClassBase->Lookup (ID)) == NULL) {
        // Error, class not registered
        Error (stGetError, ID);
        return NULL;
    }

    // Create a new (empty) instance
    Streamable* NewInstance = C->Builder ();

    // Read the instance data from stream
    *this >> *NewInstance;

    // Return the new instance
    return NewInstance;

}



void Stream::Put (const Streamable& O)
{
    u16 ID;

    // Check stream status
    if (Status != stOk) {
        return;
    }

    // Write the object to the stream
    if (&O == NULL) {

        // Special handling for a NULL pointer: Write a ID of zero
        ID = 0;
        *this << ID;

    } else {

        // Get class ID
        ID = O.StreamableID ();

        // Find class in database
        CHECK (ClassBase->Lookup (ID) != NULL);

        // Write ID and instance data
        *this << ID << O;

    }
}



char* Stream::ReadStr ()
{
    u16 Len;

    // Check stream status
    if (Status != stOk) {
        return NULL;
    }

    // Read string length
    Read (&Len, sizeof (Len));

    // If the string has size zero, return a NULL pointer
    if (Len == 0) {
        return NULL;
    }

    // Allocate memory for the string and read it
    char* S = new char [Len];
    Read ((void*) S, Len);

    // return the allocated string
    return S;
}



void Stream::WriteStr (const char* S)
{
    // Check stream status
    if (Status != stOk) {
        return;
    }

    // Check the length, allow NULL pointers for emtpy strings
    u16 Len = (u16) (S ? strlen (S) + 1 : 0);

    // Write the string length followed by the string itself
    Write (&Len, sizeof (Len));
    if (Len) {
        Write (S, Len);
    }

}



void Stream::Error (int Code, int Info)
{
    Status = Code;
    ErrorInfo = Info;

    // If an error function is defined, call this function. Otherwise handle
    // all errors that are considered fatal (which means that they are caused
    // by some sort of programming error).
    if (StreamError != NULL) {

        StreamError (*this);

    } else {

        char Buf [128];
        switch (Code) {

            case stGetError:
                // Unregistered class found in call to Get
                sprintf (Buf,
                        "Stream::Error: unregistered class with ID 0x%04X",
                        Info);
                FAIL (Buf);
                break;

            case stPutError:
                // Unregistered class found in call to Put
                sprintf (Buf,
                        "Stream::Error: unregistered class with ID 0x%04X",
                        Info);
                FAIL (Buf);
                break;

        }
    }
}



Stream& operator >> (Stream& S, char* X)
{
    // Read a copy of the string
    char* Y = S.ReadStr ();

    // Copy it to the destination
    strcpy (X, Y);

    // free the string
    delete [] Y;

    return S;
}



// Beware: Bug in BC++ for DOS - char and signed char are not distinct!
#if !defined (DOS) || !defined (__BORLANDC__)
Stream& operator << (Stream& S, char X)
{
    S.Write (&X, sizeof (X));
    return S;
}
#endif



Stream& operator << (Stream& S, unsigned char X)
{
    S.Write (&X, sizeof (X));
    return S;
}



Stream& operator << (Stream& S, signed char X)
{
    S.Write (&X, sizeof (X));
    return S;
}



Stream& operator << (Stream& S, i16 X)
{
    // Convert to little endian
    ToLittleEndian (X);

    // Write into the stream
    S.Write (&X, sizeof (X));
    return S;
}



Stream& operator << (Stream& S, u16 X)
{
    // Convert to little endian
    ToLittleEndian (X);

    // Write into the stream
    S.Write (&X, sizeof (X));
    return S;
}



Stream& operator << (Stream& S, i32 X)
{
    // Convert to little endian
    ToLittleEndian (X);

    // Write into the stream
    S.Write (&X, sizeof (X));
    return S;
}



Stream& operator << (Stream& S, u32 X)
{
    // Convert to little endian
    ToLittleEndian (X);

    // Write into the stream
    S.Write (&X, sizeof (X));
    return S;
}



Stream& operator << (Stream& S, float X)
{
    // Write float as a double
    S << double (X);
    return S;
}



Stream& operator << (Stream& S, double X)
{
    // Convert to little endian
    _double Y = { X };
    ToLittleEndian (Y);

    S.Write (&Y.I, sizeof (Y.I));
    return S;
}



Stream& operator << (Stream& S, char* X)
{
    S.WriteStr (X);
    return S;
}



// Beware: Bug in BC++ for DOS - char and signed char are not distinct!
#if !defined (DOS) || !defined (__BORLANDC__)
Stream& operator >> (Stream& S, char & X)
{
    S.Read (&X, sizeof (X));
    return S;
}
#endif



Stream& operator >> (Stream& S, unsigned char& X)
{
    S.Read (&X, sizeof (X));
    return S;
}



Stream& operator >> (Stream& S, signed char& X)
{
    S.Read (&X, sizeof (X));
    return S;
}



Stream& operator >> (Stream& S, i16& X)
{
    // Read from the stream
    S.Read (&X, sizeof (X));

    // Convert from little endian to native format
    FromLittleEndian (X);

    // Return the used stream
    return S;
}



Stream& operator >> (Stream& S, u16& X)
{
    // Read from the stream
    S.Read (&X, sizeof (X));

    // Convert from little endian to native format
    FromLittleEndian (X);

    // Return the used stream
    return S;
}



Stream& operator >> (Stream& S, i32& X)
{
    // Read from the stream
    S.Read (&X, sizeof (X));

    // Convert from little endian to native format
    FromLittleEndian (X);

    // Return the used stream
    return S;
}



Stream& operator >> (Stream& S, u32& X)
{
    // Read from the stream
    S.Read (&X, sizeof (X));

    // Convert from little endian to native format
    FromLittleEndian (X);

    // Return the used stream
    return S;
}



Stream& operator >> (Stream& S, float& X)
{
    // Read a double from the stream
    double Y;
    S >> Y;
    X = Y;

    // Return the used stream
    return S;
}



Stream& operator >> (Stream& S, double& X)
{
    // Read from the stream
    _double Y;
    S.Read (&Y.I, sizeof (Y.I));

    // Convert from little endian to native format
    FromLittleEndian (Y);
    X = Y.F;

    // Return the used stream
    return S;
}



/*****************************************************************************/
/*                     Manipulators for class Stream                         */
/*****************************************************************************/



Stream& Flush (Stream& S)
// Flush the stream
{
    S.Flush ();
    return S;
}



/*****************************************************************************/
/*                             class FileStream                              */
/*****************************************************************************/



FileStream::FileStream (size_t BufSize):
    F (NULL)
// Create a temporary file stream. The corresponding file will be deleted
// when the stream is closed. You cannot get the name of a temporary file.
{
    // Try to open the file
    if ((F = tmpfile ()) == NULL) {
        Error (stInitError, errno);
        return;
    }

    // Set the buffer
    SetBuf (BufSize);
}



FileStream::FileStream (const String& FileName, const String& Mode, size_t BufSize):
    F (NULL),
    Name (FileName)
// Open a file stream using the given mode.
{
    // Try to open the file
    if ((F = fopen (FileName.GetStr (), Mode.GetStr ())) == NULL) {
        Error (stInitError, errno);
        return;
    }

    // Set the buffer
    SetBuf (BufSize);
}



FileStream::FileStream (const String& FileName, size_t BufSize):
    F (NULL),
    Name (FileName)
{
    // Try to open an existing file for r/w
    if ((F = fopen (FileName.GetStr (), "r+b")) == NULL) {
        // Error opening file. Try to create a new file
        if ((F = fopen (FileName.GetStr (), "w+b")) == NULL) {
            Error (stInitError, errno);
            return;
        }
    }

    // Set the buffer
    SetBuf (BufSize);
}



FileStream::~FileStream ()
{
    // If there is an open file, try to close it but ignore errors
    if (F) {
        (void) fclose (F);
    }
}



void FileStream::SetBuf (size_t BufSize)
// Set the file buffer. For use in the constructors only!
{
    // Set the buffer
    if (BufSize) {
        if (setvbuf (F, NULL, _IOFBF, BufSize) != 0) {
            Error (stInitError, 0);
            return;
        }
    } else {
        setvbuf (F, NULL, _IONBF, 0);
    }
}



void FileStream::Flush ()
{
    if (Status == stOk) {
        fflush (F);
    }
}



u32 FileStream::GetPos ()
{
    long Pos;

    if (Status == stOk) {
        if ((Pos = ftell (F)) == -1L) {
            Error (stSeekError, errno);
            return 0L;
        }
        return Pos;
    }
    return 0L;
}



u32 FileStream::GetSize ()
{
    if (Status == stOk) {
        // Remember current file position
        long CurPos = ftell (F);

        // Seek to the end
        fseek (F, 0L, SEEK_END);

        // Get file length
        long Length = ftell (F);

        // Seek back to the previous position
        fseek (F, CurPos, SEEK_SET);

        // Check for errors
        if (Length == -1L) {
            Error (stSeekError, errno);
            return 0L;
        } else {
            return Length;
        }
    }
    return 0L;
}



void FileStream::Read (void* Buf, size_t Count)
{
    if (Status == stOk && Count > 0) {
        if (fread (Buf, 1, Count, F) != Count) {
            Error (stReadError, Count);
        }
    }
}



int FileStream::Read ()
{
    int C = EOF;
    if (Status == stOk) {
        if ((C = fgetc (F)) == EOF) {
            Error (stReadError, 0);
        }
    }
    return C;
}



void FileStream::Reset ()
{
    Stream::Reset ();
    clearerr (F);
}



void FileStream::Seek (unsigned long Pos)
{
    if (Status == stOk) {
        if (fseek (F, Pos, SEEK_SET) != 0) {
            Error (stSeekError, 0);
        }
    }
}



void FileStream::SeekToEnd ()
// Set the stream pointer to the end of the stream
{
    if (Status == stOk) {
        if (fseek (F, 0, SEEK_END) != 0) {
            Error (stSeekError, 0);
        }
    }
}



void FileStream::Truncate ()
{
    if (Status == stOk) {

        // Flush the stream
        Flush ();

        // Truncate the file
#if defined(__WATCOMC__) || (defined(DOS) && defined(__BORLANDC__))
        chsize (fileno (F), GetPos ());
#else
        ftruncate (fileno (F), GetPos ());
#endif

    }
}



void FileStream::Write (const void* Buf, size_t Count)
{
    if (Status == stOk && Count > 0) {
        if (fwrite (Buf, 1, Count, F) != Count) {
            Error (stWriteError, 0);
        }
    }
}



