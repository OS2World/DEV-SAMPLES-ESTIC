/*****************************************************************************/
/*									     */
/*				  WILDARGS.CC				     */
/*									     */
/* (C) 1996	Ullrich von Bassewitz					     */
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



// This is a special module for DOS and OS/2. It is used to expand the
// argument vector if any of the arguments has wildcards in it.



#include <stdio.h>

#include "chartype.h"
#include "filecoll.h"
#include "filepath.h"
#include "wildargs.h"



/*****************************************************************************/
/*			Explicit template instantiation			     */
/*****************************************************************************/



#ifdef EXPLICIT_TEMPLATES
template class Collection<String>;
#endif



/*****************************************************************************/
/*				     Code				     */
/*****************************************************************************/



static void ExpandWildArg (const String& Arg, Collection<String>& NewArgs)
// Search for all files that match Arg and insert the names into NewArgs
{
    // Separate path and filename
    String Dir, Name;
    FSplit (Arg, Dir, Name);

    // Read the files into a filename collection
    FileInfoColl Files;
    Files.ReadFiles (Dir, Name, 0xFFFF, 0x0000);

    // Now transfer the filenames. Beware: If the name does not expand to
    // anything insert just the name of the file.
    int Count = Files.GetCount ();
    if (Count == 0) {

	// Insert the original name
	NewArgs.Insert (new String (Arg));

    } else {

	// Transfer all files
	for (int I = 0; I < Count; I++) {

	    NewArgs.Insert (new String (Files [I]->Name));

	}
    }
}



void ExpandArgs (int& ArgCount, _PPCHAR& ArgVec)
// Expand the argument list
{
    // Initialize ...
    char** OldArgs = ArgVec;
    Collection<String> NewArgs (20, 20, 1);

    // Copy the first argument without looking at it, since this is the
    // name of the executable under DOS and OS/2
    NewArgs.Insert (new String (*OldArgs++));

    // Loop over all arguments, remember if we did an expand
    int DidExpand = 0;
    while (*OldArgs != NULL) {

	// Get the argument into a string
	String Arg = *OldArgs;

	// Here is a special convention: If an argument is preceeded by a
	// '@', it holds the name of a text file with one argument per
	// line. This is used to circumvent the 128 bytes command line
	// restriction of dos.
	FILE* F;
	if (Arg [0] == '@' && (F = fopen (&(*OldArgs) [1], "rt")) != NULL) {

	    // We could open the file, read the lines
	    char Buf [512];
	    while (fgets (Buf, sizeof (Buf), F) != NULL) {

		// Assign the line to the argument string
		Arg = Buf;

		// Remove leading and trailing white space
		Arg.Remove (WhiteSpace, rmLeading | rmTrailing);

		// Check if we have to expand the string
		if (Arg [0] != '-' && FHasWildcards (Arg)) {

		    // Expand the argument
		    ExpandWildArg (Arg, NewArgs);

		} else {

		    // No wildcards, no switch: store a copy
		    NewArgs.Insert (new String (Arg));

		}

		// We expanded the file
		DidExpand = 1;
	    }

	    // Close the response file
	    (void) fclose (F);

	} else if (Arg [0] != '-' && FHasWildcards (Arg)) {

	    // Expand the argument
	    ExpandWildArg (Arg, NewArgs);
	    DidExpand = 1;

	} else {

	    // No wildcards, no switch: store a copy
	    NewArgs.Insert (new String (Arg));

	}

	// Next argument
	OldArgs++;
   }

    // If we did not expand anything, we are done now, else copy the new
    // arguments
    if (DidExpand) {

	// Create a new char* argument vector. Don't forget the trailing NULL
	ArgVec = new char* [NewArgs.GetCount () + 1];

	// Transfer the strings
	ArgCount = 0;
	for (ArgCount = 0; ArgCount < NewArgs.GetCount (); ArgCount++) {

	    // Get the string argument
	    String* Arg = NewArgs [ArgCount];

	    // Create a copy on the heap
	    ArgVec [ArgCount] = new char [Arg->Len () + 1];
	    memcpy (ArgVec [ArgCount], Arg->GetStr (), Arg->Len () + 1);

	}

	// Add the trailing NULL
	ArgVec [ArgCount] = NULL;
    }
}

