/*****************************************************************************/
/*                                                                           */
/*                                  STREAM.H                                 */
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



// Some words on the bit read and write functions for class Stream:
// The functions
//
//      BitWrite
//      BitRead
//      BitFlush
//
// depend on the normal Read and Write functions and may *not* be used
// in a mixed fashion. BitWrite and BitRead manipulate byte sized buffers
// that are written to / read from the stream using Read and Write when
// needed. You *must* call BitFlush before calling any other non-"Bit"
// function, or your data will get corrupted. BitWrite and BitRead use
// the same buffer byte, so they may not be used together without calling
// BitFlush first.



#ifndef _STREAM_H
#define _STREAM_H



#include <stdio.h>
#include <string.h>

#include "strmable.h"
#include "coll.h"
#include "str.h"



static const stOk           =  0;   // things are allright
static const stInitError    =  1;   // error initializing the stream
static const stReadError    =  2;   // error reading from the stream
static const stWriteError   =  3;   // error writing to the stream
static const stGetError     =  4;   // get found not registered class
static const stPutError     =  5;   // put found not registered class
static const stMemoryError  =  6;   // not enough memory
static const stStoreError   =  7;   // Keine Store-Methode angegeben
static const stLoadError    =  8;   // Keine Load-Methode oder Fehler bei Load
static const stCopyError    =  9;   // CopyFrom: error of source stream
static const stSeekError    = 10;   // error using Seek, GetPos etc.
static const stReadTimeout  = 11;   // Timeout on read (CharacterStream only)
static const stWriteTimeout = 12;   // Timeout on write (CharcterStream only)



// Register a class
#define LINK(Class, ID) static StreamableClass __r##Class (Class::Build, ID)



/*****************************************************************************/
/*                           class StreamableClass                           */
/*****************************************************************************/



// Object to keep track of registered streamable objects
class StreamableClass : public Object {

    friend class StreamableClasses;
    friend class Stream;

private:
    u16 ID;                             // ID of streamable class
    Streamable* (*Builder) ();          // Pointer to Build member function

public:
    StreamableClass (Streamable* (*BuildFunc) (), u16 ClassID);

};



/*****************************************************************************/
/*                               class Stream                                */
/*****************************************************************************/



class Stream : public Streamable {

public:
    typedef     void (*ErrorFunc) (Stream&);
    // Type of function that is called from Error

private:
    // Bit writing stuff
    unsigned            BitCount;
    unsigned char       BitBuf;

protected:
    int         Status;
    int         ErrorInfo;

    ErrorFunc   StreamError;
    // Function that is called in case of errors

    virtual void Error (int Code, int Info);
    // Sets the error and status codes and calls StreamError

public:
    Stream ();

    void BitWrite (u32 Bits, unsigned Count);
    // Write the given bits to the stream, LSB first. Count may not be
    // greater than 32.

    u32 BitRead (unsigned Count);
    // Read some bits from the stream. Count may not be greater than 32.

    void BitFlush ();
    // Flush the bit write buffer if needed.

    void BitReset ();
    // Reset the bit buffer to empty without writing anything. This is needed
    // to reset to a byte boundary between two BitReads.

    void CopyFrom (Stream& S, size_t Count);
    // Copy data from another stream

    virtual void Flush ();
    // Flush associated buffers

    Streamable* Get ();
    // Read an object instance from the stream

    virtual u32 GetPos ();
    // Get the current value of the stream pointer

    virtual u32 GetSize ();
    // Return the size of the stream in bytes

    int GetStatus () const;
    // Return the stream status

    int GetErrorInfo () const;
    // Return the error information

    void Put (const Streamable&);
    void Put (const Streamable*);
    // Write an object instance into a stream

    virtual void Read (void* Buf, size_t Count);
    // Read from the stream

    virtual int Read ();
    // Read a character from the stream, return EOF in case of errors or
    // end of stream (need Reset to continue)

    char* ReadStr ();
    // Read a char* from the stream

    virtual void Reset ();
    // Reset the error codes

    virtual void Seek (unsigned long Pos);
    // Set the stream pointer to the specified position

    virtual void SeekToEnd ();
    // Set the stream pointer to the end of the stream

    ErrorFunc SetErrorFunc (ErrorFunc);
    // Set a new StreamError function

    virtual void Truncate ();
    // Truncate the stream at the current position

    virtual void Write (const void* Buf, size_t Count);
    // Write to the stream

    void WriteStr (const char* S);
    // Write a char* to the stream

    friend inline Stream& operator << (Stream&, Stream& (*F) (Stream&));
    // Manipulator support function

    friend Stream& operator << (Stream&, char);
    friend Stream& operator << (Stream&, unsigned char);
    friend Stream& operator << (Stream&, signed char);
    friend Stream& operator << (Stream&, i16);
    friend Stream& operator << (Stream&, u16);
    friend Stream& operator << (Stream&, i32);
    friend Stream& operator << (Stream&, u32);
    friend Stream& operator << (Stream&, float);
    friend Stream& operator << (Stream&, double);
    friend Stream& operator << (Stream&, char*);

    friend Stream& operator >> (Stream&, char&);
    friend Stream& operator >> (Stream&, unsigned char&);
    friend Stream& operator >> (Stream&, signed char&);
    friend Stream& operator >> (Stream&, i16&);
    friend Stream& operator >> (Stream&, u16&);
    friend Stream& operator >> (Stream&, i32&);
    friend Stream& operator >> (Stream&, u32&);
    friend Stream& operator >> (Stream&, float&);
    friend Stream& operator >> (Stream&, double&);
    friend Stream& operator >> (Stream&, char*);
};



inline void Stream::BitReset ()
// Reset the bit buffer to empty without writing anything. This is needed
// to reset to a byte boundary between two BitReads.
{
    BitCount = 0;
    BitBuf   = 0;
}



inline int Stream::GetStatus () const
{
    return Status;
}



inline int Stream::GetErrorInfo () const
{
    return ErrorInfo;
}



inline void Stream::Put (const Streamable* X)
{
    Put (*X);
}



inline Stream& operator << (Stream& S, Stream& (*Func) (Stream&))
// Manipulator support function
{
    return Func (S);
}



/*****************************************************************************/
/*                     Manipulators for class Stream                         */
/*****************************************************************************/



Stream& Flush (Stream& S);
// Flush the stream



/*****************************************************************************/
/*                             class FileStream                              */
/*****************************************************************************/



class FileStream : public Stream {

private:
    void SetBuf (size_t BufSize);
    // Set the file buffer. For use in the constructors only!


protected:
    FILE        *F;
    String      Name;


public:
    FileStream (const String& FileName, size_t BufSize = BUFSIZ);
    // Open a file stream. If the file with the given name does not exist,
    // it is created.

    FileStream (const String& FileName, const String& Mode, size_t BufSize = BUFSIZ);
    // Open a file stream using the given mode.

    FileStream (size_t BufSize = BUFSIZ);
    // Create a temporary file stream. The corresponding file will be deleted
    // when the stream is closed. You cannot get the name of a temporary file.

    virtual ~FileStream ();

    virtual void Flush ();
    // Flush associated buffers

    virtual u32 GetPos ();
    // Get the current value of the stream pointer

    virtual u32 GetSize ();
    // Return the size of the stream in bytes

    virtual void Read (void *Buf, size_t Count);
    // Read from the stream

    virtual int Read ();
    // Read a byte from the stream. Returns EOF if end of file is reached.
    // EOF is no error condition (as in all other reads)

    virtual void Reset ();

    virtual void Seek (unsigned long Pos);
    // Set the stream pointer to the given value

    void SeekToEnd ();
    // Set the stream pointer to the end of the stream

    virtual void Truncate ();
    // Truncate the stream at the current stream pointer position

    virtual void Write (const void *Buf, size_t Count);
    // Write data to the stream

    const String& GetName () const;
    // Return the filename

    const FILE* GetFile () const;
    // Return the file variable

};



inline const String& FileStream::GetName () const
{
    return Name;
}



inline const FILE* FileStream::GetFile () const
{
    return F;
}



// End of STREAM.H

#endif

