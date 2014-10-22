/*****************************************************************************/
/*                                                                           */
/*                                MEMCHECK.H                                 */
/*                                                                           */
/* (C) 1995     Ullrich von Bassewitz                                        */
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



// Poor man's memory checker. Overloads the global operators new and delete
// and does some additional checks if the variable MemCheck is set to true:
//
//      * Check if an allocated block is already allocated (heap corrupt)
//      * Check if a block that should be freed is allocated
//      * Check if there have been writes outside the blocks bounds (by
//        adding a signature to the end)
//
// When using Watcom-C, the simple existance of the overloaded operators is
// enough to use them (no need to include this file).
// This may not be the case with other compilers (not checked yet).
//
// Beware: This module is _not_ reentrant if MemCheck is set to true! Don't
//         use it under a multithreaded environment (e.g. OS/2)!
//



#ifndef _MEMCHECK_H
#define _MEMCHECK_H



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



// Switch the memory checking on/off
extern int MemCheck;

// Switch memory filling on or off
extern int MemFill;

// Some statistics. It is usually not necessary to access these.
extern u32 MemNewCount;
extern u32 MemDelCount;
extern u32 MemDelNULLCount;
extern u32 MemNewCheckCount;
extern u32 MemDelCheckCount;
extern u32 MemLargestBlock;
extern u32 MemUsage;
extern u32 MemMaxUsage;



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



void* operator new (size_t);
void operator delete (void*);
// Overloaded operators new and delete

u32 MemBlocksInUse ();
// Returns the number of memory blocks currently allocated

void MemLogBlocksInUse (const String& Name);
// Write a list of all used blocks into a file



// End of MEMCHECK.H

#endif



