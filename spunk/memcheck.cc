/*****************************************************************************/
/*                                                                           */
/*                                MEMCHECK.CC                                */
/*                                                                           */
/* (C) 1995     Ullrich von Bassewitz                                        */
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



// Poor man's memory checker. Overloads the global operators new and delete
// and does some additional checks if the variable MemCheck is set to true:
//
//      * Check if an allocated block is already allocated (heap corrupt)
//      * Check if a block that should be freed is allocated
//      * Check if there have been writes outside the blocks bounds (by
//        adding a signature to the end)
//      * Check if new does not provide a NULL pointer.



#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "machine.h"
#include "check.h"



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



// Signature of a memory block
static u32 MemSig = 0x12785634;

// Switch memory checking on or off
int MemCheck = 0;

// Switch memory filling on or off
int MemFill = 0;

// Statistics
u32 MemNewCount = 0;
u32 MemDelCount = 0;
u32 MemDelNULLCount = 0;
u32 MemNewCheckCount = 0;
u32 MemDelCheckCount = 0;
u32 MemLargestBlock = 0;
u32 MemUsage = 0;
u32 MemMaxUsage = 0;

// This is the fill value for memory blocks if MemFill is true. On intel
// architectures, this is the code for "INT 3", an instruction that is
// often used by debuggers as a breakpoint.
const unsigned char FillVal = 0xCC;



/*****************************************************************************/
/*                             struct BlockInfo                              */
/*****************************************************************************/



struct BlockInfo {

    unsigned char*  Ptr;
    u32             Size;

};

//
const int FirstChunk    = 2000;
const int Delta         = 1000;

// Variables needed
static int              IsInitialized = 0;
static int              BlockCount = 0;
static int              BlockLimit = 0;
static BlockInfo*       Blocks = NULL;



/*****************************************************************************/
/*                            class BlockInfoColl                            */
/*****************************************************************************/



static void MemSetCount (int NewCount)
// Make shure, there is space for NewSize blocks in Blocks
{
    if (NewCount > BlockLimit) {
        // OOPS, need realloc
        if (BlockLimit == 0 && NewCount <= FirstChunk) {
            BlockLimit = FirstChunk;
        } else {
            BlockLimit = ((NewCount / Delta) + 1) * Delta;
        }
        Blocks = (BlockInfo*) realloc (Blocks, BlockLimit * sizeof (BlockInfo));
    }
    BlockCount = NewCount;
}



static int MemSearch (const unsigned char* Ptr, int& Index)
// Search for the block. Return 1 if the block is found (Index holds the
// block index in this case). Return 0 if the block is not found and return
// in Index the index where the block should be inserted.
{
    // do a binary search
    int First = 0;
    int Last = BlockCount - 1;
    int Current;
    int S = 0;

    while (First <= Last) {

        // Set current to mid of range
        Current = (Last + First) / 2;

        // Do a compare
        if (Blocks [Current].Ptr < Ptr) {
            First = Current + 1;
        } else {
            Last = Current - 1;
            if (Blocks [Current].Ptr == Ptr) {
                // Found.
                S = 1;  // function result
                // Set condition to terminate loop
                First = Current;
            }
        }

    }

    Index = First;
    return S;
}



static void MemDelBlock (int Index)
// Delete the block with the given index
{
    BlockCount--;
    memmove (Blocks+Index, Blocks+Index+1, (BlockCount-Index) * sizeof (BlockInfo));
}



static void MemInsBlock (int Index, unsigned char* Ptr, u32 Size)
{
    // Set the new size
    MemSetCount (BlockCount + 1);

    // We can insert the element. If the item is not inserted at the end
    // of the collection, we must create a "hole"
    if (Index != BlockCount - 1) {
        memmove (Blocks + Index + 1,
                 Blocks + Index,
                 (BlockCount - 1 - Index) * sizeof (BlockInfo));
    }

    // store the new data
    Blocks [Index].Ptr  = Ptr;
    Blocks [Index].Size = Size;
}



u32 MemBlocksInUse ()
{
    return (u32) BlockCount;
}



void MemLogBlocksInUse (const char* Name)
{
    FILE* F = fopen (Name, "w+t");
    if (F == NULL) {
        // This is a debug function, so ignore the error
        return;
    }

    // Get the block count and log some statistics
    fprintf (F, "Blocks currently in use:               %8lu\n\n"
                "Calls to operator new:                 %8lu\n"
                "Calls to operator delete:              %8lu\n"
                "Checked calls to new:                  %8lu\n"
                "Checked calls to delete:               %8lu\n"
                "Calls to delete with a NULL arg:       %8lu\n\n"
                "Largest block allocated:               %8lu\n"
                "Maximum memory usage:                  %8lu\n\n",
                (unsigned long) BlockCount,
                (unsigned long) MemNewCount,
                (unsigned long) MemDelCount,
                (unsigned long) MemNewCheckCount,
                (unsigned long) MemDelCheckCount,
                (unsigned long) MemDelNULLCount,
                (unsigned long) MemLargestBlock,
                (unsigned long) MemMaxUsage);

    // Log the blocks
    BlockInfo* Block = Blocks;
    for (int I = 0; I < BlockCount; I++, Block++) {

        // Print a line describing the block (convert pointers to hex values)
        fprintf (F, "Block %5u: Loc = 0x%08lX, Size = %5lu\n",
                 I, (unsigned long) Block->Ptr, (unsigned long) Block->Size);

    }

    // Close the file
    fclose (F);
}



static void MemDone ()
// Log the memory blocks if requested. Does *not* delete the block array
// since the startup code may release memory after calling the exit functions
// and in this case we will work with a freed block, if we free the block
// array here
{
    // If the environment variable SPUNK_MEMLOGBLOCKS is set to something, use
    // this "something" as a filename to log a list of still allocated blocks
    const char* Name = getenv ("SPUNK_MEMLOGBLOCKS");
    if (Name) {
        MemLogBlocksInUse (Name);
    }
}



static void MemInit ()
// Initialize the memory checker
{
    // Get the defaults for the memory checker
    MemCheck = getenv ("SPUNK_MEMCHECK") != NULL;
    MemFill = getenv ("SPUNK_MEMFILL") != NULL;

    // Register the exit function
    atexit (MemDone);

    // Initialized now
    IsInitialized = 1;
}



#ifdef __WATCOMC__
/*****************************************************************************/
/*                            class MemCheckInit                             */
/*****************************************************************************/



// This section is needed to initialize the memory checker *before* any other
// library modules. It does work with Watcom-C++ only, but probably other
// compilers have similar ways to ensure library initialization.
//
// Create a class with a static instance. The constructor of the class will
// call the memory initialization, a #pragma assures that the module
// initialization code is executed very early.
#pragma initialize library;


class MemCheckInit {
public:
    MemCheckInit ();
};



inline MemCheckInit::MemCheckInit ()
{
    if (IsInitialized == 0) {
        MemInit ();
    }
}



static MemCheckInit Initializer;



// End of WATCOM specific code
#endif



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



void* operator new (size_t Size)
{
    // Initialize the memory checker on the first call
    if (IsInitialized == 0) {
        MemInit ();
    }

    // Count the calls to new
    MemNewCount++;

    // Update largest block info
    if (Size > MemLargestBlock) {
        MemLargestBlock = Size;
    }
    unsigned char* Ptr;
    if (MemCheck) {

        // Count the checked calls
        MemNewCheckCount++;

        // Update memory usage
        MemUsage += Size;
        if (MemUsage > MemMaxUsage) {
            MemMaxUsage = MemUsage;
        }

        // Get a memory block
        Ptr = (unsigned char*) malloc (Size + sizeof (MemSig));

        // Make a signature at the end of the block
        memcpy (Ptr + Size, &MemSig, sizeof (MemSig));

        // Search for the block
        int Index;
        if (MemSearch (Ptr, Index) != 0) {
            // An item with this key exists. This means that the heap is
            // corrupted
            FAIL ("MemCheck: Duplicate block!");
        } else {
            // The returned pointer is not in the collection of already
            // allocated blocks, but it may point inside of an already
            // allocated block. Check this.
            // Note: Index is the index of the item _before the given
            // pointer, so simply check the range of the entry with index
            // Index.
            if (Index > 0) {
                // There is a block that's memory address is less than the
                // one returned by malloc
                const BlockInfo* BB = Blocks + Index - 1;
                if (Ptr < BB->Ptr + BB->Size) {
                    // Pointer points inside the block below - heap corrupted
                    FAIL ("MemCheck: Heap corrupt!");
                }
            }

            // Heap ok, insert the new block
            MemInsBlock (Index, Ptr, Size);
        }

    } else {

        // No memory checking. Allocate a memory block, but beware: New is
        // defined so that "new char [0]" points to a distinct object every
        // time it is called, so one cannot return NULL for a size of 0!
        Ptr = (unsigned char*) malloc (Size ? Size : 1);

    }

    // Check if we got memory, fail otherwise
    if (Ptr == NULL) {
        FAIL ("MemCheck: Out of memory");
    }

    // Fill the memory block if requested
    if (MemFill) {
        memset (Ptr, FillVal, Size);
    }

    // Return a pointer to the memory block
    return Ptr;
}



void operator delete (void* P)
{
    // We cannot call delete if the memory system is not initialized
    if (IsInitialized == 0) {
        FAIL ("MemCheck: Trying to delete a block before the first call to new!");
    }

    // Count the calls to delete
    MemDelCount++;

    // Deleting NULL pointers is always ok, nothing has to be done
    if (P == 0) {
        MemDelNULLCount++;
        return;
    }

    if (MemCheck) {

        // Count the calls
        MemDelCheckCount++;

        // Cast the pointer
        unsigned char* Ptr = (unsigned char*) P;

        // Search for the block
        int Index;
        if (MemSearch (Ptr, Index) != 0) {

            // The block exists. Check the signature, then delete it
            BlockInfo* BI = Blocks + Index;
            if (memcmp (Ptr + BI->Size, &MemSig, sizeof (MemSig)) != 0) {
                // Signature overwritten
                FAIL ("MemCheck: Block signature overwritten");
            }

            // Fill the memory block if requested
            if (MemFill) {
                memset (Ptr, FillVal, BI->Size);
            }

            // Update memory usage
            MemUsage -= BI->Size;

            // Delete the entry
            MemDelBlock (Index);

            // Delete the memory block
            free (P);

        } else {
            // Trying to free a block that is not allocated
            FAIL ("MemCheck: Trying to free a block that is not allocated");
        }
    } else {

        // Free the block without checks
        free (P);

    }
}



