/*****************************************************************************/
/*                                                                           */
/*                                   CONT.H                                  */
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



#ifndef _CONT_H
#define _CONT_H



#include <stdlib.h>

#include "strmable.h"
#include "stream.h"



/*****************************************************************************/
/*                              class Container                              */
/*****************************************************************************/



class Container : public Streamable {

private:
    void*       Data;           // Pointer to data
    u32         Size;           // Size of data

protected:
    Container (StreamableInit X);
    // Build constructor

public:
    Container ();
    Container (const Container&);
    virtual ~Container ();

    // Derived from class Streamable
    virtual void Load (Stream&);
    virtual void Store (Stream&) const;
    virtual u16 StreamableID () const;

    static Streamable* Build ();
    // Build constructor

    void FreeData ();
    // Free the data. Set the data pointer to NULL and size to zero.

    void* GetData ();
    // Return a pointer to the data

    const void* GetData () const;
    // Return a const pointer to the data

    void* RetrieveData ();
    // Return the current data and empty the container. After this call,
    // the calling function is responsible for managing/deleting the data.

    u32 DataSize () const;
    // Return the data size

    void PutData (void* DataPtr, u32 DataSize);
    // Free the current data and store the new data in the container.

    Container& CopyFrom (void* DataPtr, u32 DataSize);
    // Free the current data, create a new data block, copy given data

    int StoreData (const String& Filename) const;
    // Store the current data in the given file. Return 1 on success, 0 on
    // failure.

    int LoadData (const String& Filename);
    // Delete the current data, read new data from a file. The function
    // returns 1 on success, 0 on failure.

    Container& operator = (const Container& C);
    // Assignment operator for containers
};



inline Container::Container (StreamableInit)
{
}



inline Container::Container () :
    Data (NULL), Size (0)
{
}



inline u32 Container::DataSize () const
{
    return Size;
}



// End of CONT.H

#endif

