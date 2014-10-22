/*****************************************************************************/
/*                                                                           */
/*                                 RESFILE.H                                 */
/*                                                                           */
/* (C) 1993-95  Ullrich von Bassewitz                                        */
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



#include "cont.h"
#include "filepath.h"
#include "filesel.h"

#include "resed.h"



/*****************************************************************************/
/*                             class ResEditApp                              */
/*****************************************************************************/



void ResEditApp::CloseResourceFile ()
{
    if (ResFile) {
        delete ResFile;
        ResFile = NULL;
        MainMenue->GrayItem (miClose);
        MainMenue->GrayItem (miPack);
        MainMenue->GrayItem (miWrite);
        MainMenue->GrayItem (miRead);
        MainMenue->GrayItem (miAddStream);
        MainMenue->GrayItem (miAddData);
        MainMenue->GrayItem (miMerge);
    }
}



void ResEditApp::OpenResourceFile (const String& Name)
{
    // Close the resource file if one is open
    CloseResourceFile ();

    // Now try to open the resource file
    ResFile = new ResourceFile (new FileStream (Name));
    if (ResFile->GetStatus () != reOk) {
        ErrorMsg (String ("Error opening \"") + Name + '\"');
        delete ResFile;
        ResFile = NULL;
        return;
    }

    // Activate the corresponding menue entries
    MainMenue->ActivateItem (miClose);
    MainMenue->ActivateItem (miRead);
    MainMenue->ActivateItem (miPack);
    MainMenue->ActivateItem (miAddStream);
    MainMenue->ActivateItem (miAddData);
    MainMenue->ActivateItem (miMerge);
    if (Res) {
        MainMenue->ActivateItem (miWrite);
    }
}



void ResEditApp::Open ()
{
    FileSelector FS (" Open a file ", ".res", fsFileMayNotExist);
    String Sel = FS.GetChoice ("*.res");
    if (!Sel.IsEmpty ()) {
        ResFileName = Sel;
        OpenResourceFile (ResFileName);
    }
}



void ResEditApp::Close ()
{
    CloseResourceFile ();
}



void ResEditApp::Pack ()
{
    // This may last a while, pop up a message
    Window *Win = PleaseWaitWindow ();

    // Pack the file
    ResFile->Pack ();

    // Safety: Write out the index to the resource file. Otherwise the file
    // is unusable if the editor crashes (who knows?)
    ResFile->Flush ();

    // Delete the mesage window
    delete Win;
}



void ResEditApp::AddData ()
// Add a container object with arbitrary data
{
    int Abort;

    // Get file name
    FileSelector FS (" Open a file ");
    String Filename = FS.GetChoice ("*.dat");
    if (Filename.IsEmpty ()) {
        return;
    }

    // Create container and try to load the data
    Container C;
    if (C.LoadData (Filename) == 0) {
        ErrorMsg ("Error loading data from " + Filename);
        return;
    }

    // Now write the container to the stream (use the filename as default
    // for the resource name)
    NamePrompt (" Resource name", Filename, Abort);
    if (!Abort) {
        ResFile->Put (C, Filename);
    }
}



void ResEditApp::AddStream ()
// Add an object from a file stream to the resource file
{
    // Get name of the stream
    FileSelector FS (" Open a file ");
    String FileName = FS.GetChoice ();
    if (FileName.IsEmpty ()) {
        return;
    }

    // Get the name of the resource
    int Abort;
    String Name;
    NamePrompt (" Name of resource", Name, Abort);
    if (Abort) {
        return;
    }

    // Try to open the stream
    FileStream* S = new FileStream (FileName, "rb");
    if (S->GetStatus () != stOk) {
        ErrorMsg (String ("Error opening \"") + FileName + '\"');
        delete S;
        return;
    }

    // Read an object from the stream, check status
    Streamable* Obj = S->Get ();
    if (S->GetStatus () != stOk) {
        ErrorMsg ("Error reading from stream");
        delete S;
        return;
    }

    // Write the object into the resource file
    ResFile->Put (Obj, Name);

    // Delete the object and the stream
    delete S;
    delete Obj;

}



static Key MapKey (Key K)
// Map a extended cursor key fix to it's virtual counterpart
{
    switch (K) {

        case kbUp:
            return vkUp;

        case kbDown:
            return vkDown;

        case kbLeft:
            return vkLeft;

        case kbRight:
            return vkRight;

        default:
            return K;

    }
}



void ResEditApp::Merge ()
// Merge two resource files
{
    // Get name of second resource file
    FileSelector FS (" Open a file ", ".res");
    String Name = FS.GetChoice ("*.res");
    if (Name.IsEmpty ()) {
        return;
    }

    // Try to open second resource file
    ResourceFile* NewResFile = new ResourceFile (new FileStream (Name));
    if (NewResFile->GetStatus () != reOk) {
        ErrorMsg (String ("Error opening \"") + Name + '\"');
        delete NewResFile;
        return;
    }

    // Check if the new resource file contains resources
    int ResCount = NewResFile->GetCount ();
    if (ResCount == 0) {
        ErrorMsg (Name + " is empty");
        delete NewResFile;
        return;
    }

    // Show the user that we are working hard...
    String Msg ("Copying " + FormatStr ("%i", ResCount) + " resources from\n" + Name);
    Window* Win = MsgWindow (Msg, "", paCyan);

    // Copy the resources
    Streamable* Resource;
    for (int ResNum = 0; ResNum < ResCount; ResNum++) {

        // Get the name of the resource
        const String& Key = NewResFile->KeyAt (ResNum);

        // Load the resource from the new file
        Resource = NewResFile->Get (Key);

        // Compatibility hack: If the resource is a menue, adjust the cursor
        // key variables. Remove that in the near future!
        if (Resource->StreamableID () == ID_Menue) {
            Menue* M = (Menue*) Resource;
            M->PrevKey = MapKey (M->PrevKey);
            M->NextKey = MapKey (M->NextKey);
            M->AltPrevKey = MapKey (M->AltPrevKey);
            M->AltNextKey = MapKey (M->AltNextKey);
        }

        // Write the resource to the old resource file (overwriting any
        // resource object with the same name)
        ResFile->Put (Resource, Key);

        // Delete the object
        delete Resource;

    }

    // Delete the new resource file
    delete NewResFile;

    // Clear the message window
    delete Win;
}



