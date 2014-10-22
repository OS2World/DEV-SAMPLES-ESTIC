/*****************************************************************************/
/*                                                                           */
/*                                  CONT.CC                                  */
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



#include <stdio.h>
#include <sys/stat.h>

#include "streamid.h"
#include "cont.h"



// Register class Container
LINK(Container, ID_Container);



/*****************************************************************************/
/*                              class Container                              */
/*****************************************************************************/



Container::Container (const Container& C)
{
    if (C.Data == NULL) {
        // Other container is empty
        Data = NULL;
        Size = 0;
    } else {
        // Second container is not empty
        Size = C.Size;
        Data = new char [Size];
        memcpy (Data, C.Data, Size);
    }
}



Container::~Container ()
{
    FreeData ();
}



void Container::Load (Stream& S)
{
    // Get Size of Data
    S >> Size;

    // Maybe, size is 0
    if (Size == 0) {
        Data = NULL;
    } else {
        Data = new char [Size];
        S.Read (Data, Size);
    }
}



void Container::Store (Stream& S) const
{
    S << Size;
    S.Write (Data, Size);
}



u16 Container::StreamableID () const
{
    return ID_Container;
}



Streamable* Container::Build ()
{
    return new Container (Empty);
}



void Container::FreeData ()
{
    delete [] Data;
    Data = NULL;
    Size = 0L;
}



void* Container::GetData ()
// Don't inline this function - it's needed in this module for correct linking
{
    return Data;
}



const void* Container::GetData () const
// Don't inline this function - it's needed in this module for correct linking
{
    return Data;
}



void* Container::RetrieveData ()
// Return the current data and empty the container. After this call,
// the calling function is responsible for managing/deleting the data.
{
    void* D = Data;
    Data = NULL;
    Size = 0;
    return D;
}



void Container::PutData (void* DataPtr, u32 DataSize)
{
    // Free old data
    FreeData ();

    // Get control of new data
    Data = DataPtr;
    Size = DataSize;
}



Container& Container::CopyFrom (void* DataPtr, u32 DataSize)
// Free the current data, create a new data block, copy given data
{
    // If compiled under DOS: Check for maximum block size
#ifdef DOS
    CHECK (DataSize < 0xFFF0L);
#endif

    // Free the current data
    FreeData ();

    // Create a new data block and copy the given data
    Data = memcpy (new unsigned char [DataSize], DataPtr, DataSize);

    // Remember the size
    Size = DataSize;

    // Return a reference to this
    return *this;
}



int Container::StoreData (const String& Filename) const
{
    FILE* F;

    // If compiled under DOS: Honor the maximum data size for fwrite
#ifdef DOS
    CHECK (Size < 0xFFF0L);
#endif

    // Create a new file for writing
    if ((F = fopen (Filename.GetStr (), "wb")) == NULL) {
        // Cannot open file
        return 0;
    }

    // Write the container data to the file
    fwrite (Data, (size_t) Size, 1, F);

    // Close the file, return success
    fclose (F);
    return 1;
}



int Container::LoadData (const String& Filename)
{
    FILE* F;
    u32 FileSize;

    // Get the size of the file
    struct stat StatBuf;
    if (stat (Filename.GetStr (), &StatBuf) != 0) {
        // No file
        return 0;
    }
    FileSize = StatBuf.st_size;

    // When running under DOS, check for enough memory as new is not
    // able to handle requests > 0xFFF0 bytes.
#ifdef DOS
    if (FileSize > 0xFFF0) {
        // File is too big.
        return 0;
    }
#endif


    // Try to open the file for reading
    if ((F = fopen (Filename.GetStr (), "rb")) == NULL) {
        // Error
        return 0;
    }

    // Now as chances are good to access the file, we drop any old data
    // and allocate memory for the new data
    FreeData ();
    if (FileSize > 0) {
        Size = (u32) FileSize;
        Data = new char [FileSize];

        // Now read in the file data
        (void) fread (Data, (size_t) Size, 1, F);
    }

    // Close the file, ignore errors (don't know how to handle them)
    fclose (F);

    // Return success
    return 1;
}



Container& Container::operator = (const Container &rhs)
{
    // Beware of C = C
    if (&rhs != this) {
        // Free current data
        FreeData ();

        // Get data from right hand side
        if (rhs.Data == NULL) {
            Data = NULL;
            Size = 0;
        } else {
            Size = rhs.Size;
            Data = new char [Size];
            memcpy (Data, rhs.Data, Size);
        }
    }
    return *this;
}




