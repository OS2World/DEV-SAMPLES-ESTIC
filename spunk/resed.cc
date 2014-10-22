/*****************************************************************************/
/*                                                                           */
/*                                 RESED.CC                                  */
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



#include <stdio.h>

#include "resed.h"
#include "screen.h"
#include "filepath.h"
#include "progutil.h"



/*****************************************************************************/
/*                      Explicit template instantiation                      */
/*****************************************************************************/



#ifdef EXPLICIT_TEMPLATES
template class ListBox<ResourceIndex>;
#endif



/*****************************************************************************/
/*                           class ResourceListBox                           */
/*****************************************************************************/



class ResourceListBox: public ListBox<ResourceIndex> {

protected:
    virtual void Print (int Index, int X, int Y, u16 Attr);
    // Display one of the listbox entries

public:
    ResourceListBox (i16 aID, const Point & aSize, WindowItem *NextItem);

};



ResourceListBox::ResourceListBox (i16 aID, const Point & aSize,
                                  WindowItem * NextItem) :
        ListBox<ResourceIndex> ("", aID, aSize, NextItem)
{
}



void ResourceListBox::Print (int Index, int X, int Y, u16 Attr)
{
    ResourceIndex *P = Coll->At (Index);
    String S (P->GetName ());
    S.Ins (0, ' ');

    S.Pad (String::Right, Size.X, ' ');
    Owner->Write (X, Y, S, Attr);
}



/*****************************************************************************/
/*                             class ResEditApp                              */
/*****************************************************************************/



ResEditApp::ResEditApp (int argc, char** argv):
    Program (argc, argv, CreateMenueBar, CreateStatusLine, "resed"),
    ResFile (NULL),
    Res (NULL),
    ResChanged (0),
    ResID (0)
{
    // If a filename was given, load that file
    if (ArgCount > 1) {
        ResFileName = ArgVec [1];
        AddDefaultExtension (ResFileName, ".res");
        OpenResourceFile (ResFileName);
    }
}



void ResEditApp::ReadResource (const String& Name)
{
    // Allow saving if the current resource has changed
    if (!SaveResource ()) {
        return;
    }

    // Remember the new resource name
    ResName = Name;

    // Try to load the resource
    Streamable* NewRes = LoadRes (ResName);

    // Check if the load has been successful
    if (!NewRes) {
        return;
    }

    // Delete the old and assign the new resource
    AssignRes (NewRes);

    // If possible, edit the loaded resource
    EditResource ();
}



void ResEditApp::WriteResource (const String & Name)
{
    // save the resource
    ResFile->Put (Res, Name);

    // Safety: Flush the resource file (writing out index and header). If
    // anything goes wrong, the chance of an uncorrupted resource file is
    // higher this way...
    ResFile->Flush ();
}



Streamable* ResEditApp::LoadRes (const String& Name)
// Load the resource with the given namen from the resource file. Return
// a NULL pointer in case of errors.
{
    // Try to load the resource
    Streamable* Res = ResFile->Get (Name);

    // Check if the load has been successful
    if (Res == 0) {
        ErrorMsg (String ("\"") + Name + String ("\" not found"));
    }

    // Return the result
    return Res;
}



String ResEditApp::ChooseRes ()
// Choose a resource from the file and return it's name. The function
// returns the empty string on a user abort.
{
    // The name of the resource
    String Name;


    // Check if any resources are available
    if (ResFile == 0) {
        ErrorMsg ("No resource file - cannot load resource");
        return Name;
    }
    if (ResFile->GetCount () == 0) {
        ErrorMsg (ResFileName + " is empty");
        return Name;
    }

    // Make a window not covering the menuebar and the statusline
    Rect WinBounds (Background->OuterBounds ());
    WinBounds.A.Y = 1;
    WinBounds.B.Y--;

    // create an empty listbox
    Point BoxSize (WinBounds.XSize () - 2, WinBounds.YSize () - 2);
    ResourceListBox* R = new ResourceListBox (1, BoxSize, NULL);

    // Assign the collection for the listbox
    R->SetColl (ResFile->GetIndex ());

    // Create the window and set the header
    ItemWindow *Win = new ItemWindow (WinBounds, wfFramed, paBlack, 0, R);
    Win->SetHeader (" Contents of " + ResFileName + ' ');

    // Make a statusline and activate the window
    StatusLine->Push (siEsc_Abort | siDel_Delete);
    Win->Activate ();

    // Ask for the keys and handle them
    Key K;
    int Done = 0;
    while (!Done) {

        K = KbdGet ();
        R->HandleKey (K);

        switch (K) {

            case vkAbort:
                Done = 1;
                break;

            case vkDel:
                if (R->GetSelected () >= 0 && AskAreYouShure () == 2) {
                    ResFile->Delete (ResFile->KeyAt (R->GetSelected ()));
                    R->Reset ();
                    R->Draw ();
                }
                break;

            case vkAccept:
            case kbEnter:
                if (R->GetSelected () >= 0) {
                    Name = ResFile->KeyAt (R->GetSelected ());
                    Done = 1;
                }
                break;

        }

    }

    // Take the collection from the listbox
    R->SetColl (NULL);

    // Delete the window including the listbox
    delete Win;

    // Restore the old status line
    StatusLine->Pop ();

    // Return the result
    return Name;
}



void ResEditApp::Read ()
{
    // Choose the resource
    String ResName = ChooseRes ();

    // Use the new resource
    if (!ResName.IsEmpty ()) {
        ReadResource (ResName);
    }
}



void ResEditApp::Write ()
{
    int Abort;
    NamePrompt (" Resource name", ResName, Abort);
    if (!Abort) {
        WriteResource (ResName);
        ResChanged = 0;
    }
}



void ResEditApp::New ()
// Create a new resource
{
    GenericMenue* M;

    // Allow a save if the resource has changed
    if (!SaveResource ()) {
        return;
    }

    switch (SimpleMenue ("New Resource", "@TopMenueBar\n@Menue\nMsg@Collection")) {

        case 0:
            // Aborted
            return;

        case 1:
            // TopMenueBar
            M = new TopMenueBar (NULL);
            M->MoveAbs (Point (0, 1));          // Otherwise editing difficult
            AssignRes (M);
            return;

        case 2:
            // Menue
            M = new Menue (Point (10, 10), "", NULL);
            M->Resize (Rect (10, 10, 20, 15));
            M->Show ();
            AssignRes (M);
            break;

        case 3:
            // MsgColl
            AssignRes (new MsgCollection (20, 10));
            EditResource ();
            break;
    }
}



void ResEditApp::GetEvent ()
{
    // Call derived function
    Program::GetEvent ();

    // Add free heap count to the menueline
#if defined(DOS) && defined(__BORLANDC__)
    MainMenue->CWrite (MainMenue->MaxX () - 21, 0, FormatStr ("Core left: %8d", coreleft()));
#endif
}



static MenueBarItem* GetMenueBarItem (char* ItemText, i16 ItemID,
                                      WindowItem* MenueList,
                                      WindowItem* NextItem)
{
    return new MenueBarItem (ItemText, ItemID, MenueList, NextItem);
}



static SubMenueItem* GetSubMenueItem (char* ItemText, i16 ItemID,
                                      WindowItem* MenueList,
                                      WindowItem* NextItem)
{
    return new SubMenueItem (ItemText, ItemID, MenueList, NextItem);
}



static MenueItem* GetMenueItem (char* ItemText, i16 ItemID, Key AccelKey, WindowItem* NextItem)
{
    return (MenueItem*) SetAccelKey (new MenueItem (ItemText,
                                                    ItemID,
                                                    NextItem),
                                     AccelKey);
}



static MenueLine* GetMenueLine (WindowItem* NextItem)
{
    return new MenueLine (miNone, NextItem);
}



TopMenueBar* ResEditApp::CreateMenueBar ()
{
    TopMenueBar* M = new TopMenueBar (
      GetMenueBarItem       ("@Resed",              miSystem,
        GetMenueItem        ("@About",              miAbout,            kbNoKey,
        GetMenueLine        (
        GetMenueItem        ("@Quit",               miQuit,             vkQuit,
        NULL
      ))),
      GetMenueBarItem       ("@File",               miFile,
        GetMenueItem        ("@Open resourcefile",  miOpen,             kbNoKey,
        GetMenueItem        ("@Close resourcefile", miClose,            kbNoKey,
        GetMenueItem        ("P@ack resourcefile",  miPack,             kbNoKey,
        GetMenueLine        (
        GetMenueItem        ("@Read resource",      miRead,             vkOpen,
        GetMenueItem        ("@Write resource",     miWrite,            vkSave,
        GetMenueItem        ("@Print resource",     miPrint,            kbNoKey,
        GetMenueItem        ("@New resource",       miNew,              kbNoKey,
        GetMenueLine        (
        GetMenueItem        ("Add @datafile",       miAddData,          kbNoKey,
        GetMenueItem        ("Add @stream",         miAddStream,        kbNoKey,
        GetMenueItem        ("@Merge resourcefile", miMerge,            kbNoKey,
        NULL
      )))))))))))),
      GetMenueBarItem       ("@Window",             miWindow,
        GetMenueItem        ("@Header",             miHeader,           kbNoKey,
        GetMenueItem        ("@Footer",             miFooter,           kbNoKey,
        GetMenueItem        ("@Color",              miColor,            kbNoKey,
        GetMenueItem        ("@Background",         miBackgroundChar,   kbNoKey,
        GetMenueItem        ("@Size/Move",          miSize,             kbMetaS,
        GetMenueItem        ("@Number",             miNumber,           kbNoKey,
        GetSubMenueItem     ("F@lags",              miFlags,
          new NoYesItem     ("Can @move",           miCanMove,
          new NoYesItem     ("Can @resize",         miCanResize,
          new NoYesItem     ("@Ignore accept key",  miIgnoreAccept,
          new NoYesItem     ("@Modal",              miModal,
          new NoYesItem     ("@LR link",            miLRLink,
          new NoYesItem     ("Save @visible",       miVisible,
          new OffOnItem     ("Center@X",            miCenterX,
          new OffOnItem     ("Center@Y",            miCenterY,
          NULL
        )))))))),
        GetMenueLine        (
        GetMenueItem        ("@Test",               miTest,             kbMetaT,
        NULL
      ))))))))),
      GetMenueBarItem       ("@Items",              miItems,
        GetSubMenueItem     ("@Add",                miAddItem,
          GetMenueItem      ("@WindowItem",         miAddWindowItem,    kbNoKey,
          GetMenueItem      ("@TextItem",           miAddTextItem,      kbNoKey,
          GetMenueItem      ("Item@Label",          miAddItemLabel,     kbNoKey,
          GetMenueItem      ("Menue@Line",          miAddMenueLine,     kbNoKey,
          GetSubMenueItem   ("Menue@Items",         miAddMenueItems,
            GetMenueItem    ("@MenueItem",          miAddMenueItem,     kbNoKey,
            GetMenueItem    ("S@ubMenueItem",       miAddSubMenueItem,  kbNoKey,
            GetMenueItem    ("Menue@BarItem",       miAddMenueBarItem,  kbNoKey,
            GetMenueItem    ("@FloatItem",          miAddFloatItem,     kbNoKey,
            GetMenueItem    ("E@xpFloatItem",       miAddExpFloatItem,  kbNoKey,
            GetMenueItem    ("@LongItem",           miAddLongItem,      kbNoKey,
            GetMenueItem    ("@HexItem",            miAddHexItem,       kbNoKey,
            GetMenueItem    ("@DateItem",           miAddDateItem,      kbNoKey,
            GetMenueItem    ("@TimeItem",           miAddTimeItem,      kbNoKey,
            GetMenueItem    ("@StringItem",         miAddStringItem,    kbNoKey,
            GetMenueItem    ("@RStringItem",        miAddRStringItem,   kbNoKey,
            GetMenueItem    ("T@oggleItem",         miAddToggleItem,    kbNoKey,
            GetMenueItem    ("@OffOnItem",          miAddOnOffItem,     kbNoKey,
            GetMenueItem    ("@NoYesItem",          miAddNoYesItem,     kbNoKey,
            NULL
          )))))))))))))),
          GetSubMenueItem   ("Menue@Edits",         miAddMenueEdits,
            GetMenueItem    ("@EditLine",           miAddEditLine,      kbNoKey,
            GetMenueItem    ("@TextEdit",           miAddTextEdit,      kbNoKey,
            GetMenueItem    ("@FloatEdit",          miAddFloatEdit,     kbNoKey,
            GetMenueItem    ("E@xpFloatEdit",       miAddExpFloatEdit,  kbNoKey,
            GetMenueItem    ("@LongEdit",           miAddLongEdit,      kbNoKey,
            GetMenueItem    ("@HexEdit",            miAddHexEdit,       kbNoKey,
            GetMenueItem    ("@DateEdit",           miAddDateEdit,      kbNoKey,
            GetMenueItem    ("T@imeEdit",           miAddTimeEdit,      kbNoKey,
            GetMenueItem    ("@PasswordEdit",       miAddPasswordEdit,  kbNoKey,
            GetMenueItem    ("Fi@leEdit",           miAddFileEdit,      kbNoKey,
            NULL
          )))))))))),
          NULL
        )))))),
        GetMenueItem        ("@Delete",             miDelete,           kbNoKey,
        GetMenueItem        ("@Copy",               miCopy,             kbNoKey,
        GetMenueItem        ("@Edit",               miEdit,             kbMetaM,
        GetMenueItem        ("Move A@rea",          miMoveArea,         kbMetaR,
        GetMenueItem        ("@Order",              miOrder,            kbNoKey,
        NULL
      )))))),
      GetMenueBarItem       ("@Edit",               miEditActions,
        GetMenueItem        ("@Go",                 miEditResource,     kbMetaG,
        GetMenueItem        ("@Merge message file", miMergeMsgText,     kbNoKey,
        NULL
      )),
      GetMenueBarItem       ("@Options",            miOptions,
        GetSubMenueItem     ("@VideoMode",          miMode,
          GetMenueItem      ("@Mono",               miMono,             kbNoKey,
          GetMenueItem      ("@Color",              mi80x25,            kbNoKey,
          GetMenueItem      ("Color 80x@30",        mi80x30,            kbNoKey,
          GetMenueItem      ("Color 80x3@4",        mi80x34,            kbNoKey,
          GetMenueItem      ("Color @94x34",        mi94x34,            kbNoKey,
          GetMenueItem      ("@100x40 (ET4000)",    mi2A,               kbNoKey,
          NULL
        )))))),
        NULL
      ),
      NULL
    )))))));

    // Gray currently unused items
    M->GrayItem (miClose);
    M->GrayItem (miPack);
    M->GrayItem (miRead);
    M->GrayItem (miWrite);
    M->GrayItem (miPrint);
    M->GrayItem (miWindow);
    M->GrayItem (miItems);
    M->GrayItem (miEditActions);
    M->GrayItem (miAddStream);
    M->GrayItem (miAddData);
    M->GrayItem (miMerge);

    // Return the result
    return M;
}



BottomStatusLine* ResEditApp::CreateStatusLine ()
{
    return new BottomStatusLine (siAltX_Exit);
}



int ResEditApp::Run ()
{

    MainMenue->Activate ();
    while (!Quitting ()) {

        switch (MainMenue->GetChoice ()) {

            case miAbout:
                InformationMsg ("\n"
                                "\x01Resource Editor\n"
                                "\n"
                                "\x01(C) 1994 Ullrich von Bassewitz\n"
                                "\n");
                break;

            case miQuit:
                if (AskReallyQuit () == 2) {
                    // Allow saving resource
                    if (SaveResource ()) {
                        Quit = 1;
                    }
                }
                break;

            case miOpen:
                Open ();
                break;

            case miClose:
                Close ();
                break;

            case miPack:
                Pack ();
                break;

            case miRead:
                Read ();
                break;

            case miWrite:
                Write ();
                break;

            case miPrint:
                Print ();
                break;

            case miNew:
                New ();
                break;

            case miAddData:
                AddData ();
                break;

            case miAddStream:
                AddStream ();
                break;

            case miMerge:
                Merge ();
                break;

            case miHeader:
                Header ();
                break;

            case miFooter:
                Footer ();
                break;

            case miSize:
                Size ();
                break;

            case miColor:
                Color ();
                break;

            case miBackgroundChar:
                BackgroundChar ();
                break;

            case miNumber:
                Number ();
                break;

            case miCanMove:
                CanMove (0);
                break;

            case miCanMove+1:
                CanMove (1);
                break;

            case miCanResize:
                CanResize (0);
                break;

            case miCanResize+1:
                CanResize (1);
                break;

            case miIgnoreAccept:
                IgnoreAccept (0);
                break;

            case miIgnoreAccept+1:
                IgnoreAccept (1);
                break;

            case miModal:
                Modal (0);
                break;

            case miModal+1:
                Modal (1);
                break;

            case miLRLink:
                LRLink (0);
                break;

            case miLRLink+1:
                LRLink (1);
                break;

            case miCenterX:
                CenterX (0);
                break;

            case miCenterX+1:
                CenterX (1);
                break;

            case miCenterY:
                CenterY (0);
                break;

            case miCenterY+1:
                CenterY (1);
                break;

            case miTest:
                Test ();
                break;

            case miAddWindowItem:
                AddWindowItem ();
                break;

            case miAddTextItem:
                AddTextItem ();
                break;

            case miAddItemLabel:
                AddItemLabel ();
                break;

            case miAddMenueItem:
                AddMenueItem ();
                break;

            case miAddSubMenueItem:
                AddSubMenueItem ();
                break;

            case miAddMenueBarItem:
                AddMenueBarItem ();
                break;

            case miAddFloatItem:
                AddFloatItem ();
                break;

            case miAddExpFloatItem:
                AddExpFloatItem ();
                break;

            case miAddLongItem:
                AddLongItem ();
                break;

            case miAddHexItem:
                AddHexItem ();
                break;

            case miAddStringItem:
                AddStringItem ();
                break;

            case miAddRStringItem:
                AddRStringItem ();
                break;

            case miAddToggleItem:
                AddToggleItem ();
                break;

            case miAddOnOffItem:
                AddOffOnItem ();
                break;

            case miAddNoYesItem:
                AddNoYesItem ();
                break;

            case miAddDateItem:
                AddDateItem ();
                break;

            case miAddTimeItem:
                AddTimeItem ();
                break;

            case miAddEditLine:
                AddEditLine ();
                break;

            case miAddFloatEdit:
                AddFloatEdit ();
                break;

            case miAddExpFloatEdit:
                AddExpFloatEdit ();
                break;

            case miAddLongEdit:
                AddLongEdit ();
                break;

            case miAddHexEdit:
                AddHexEdit ();
                break;

            case miAddPasswordEdit:
                AddPasswordEdit ();
                break;

            case miAddFileEdit:
                AddFileEdit ();
                break;

            case miAddDateEdit:
                AddDateEdit ();
                break;

            case miAddTimeEdit:
                AddTimeEdit ();
                break;

            case miAddTextEdit:
                AddTextEdit ();
                break;

            case miAddListBox:
                break;

            case miAddFloatListBox:
                break;

            case miAddMenueLine:
                AddMenueLine ();
                break;

            case miDelete:
                Delete ();
                break;

            case miCopy:
                Copy ();
                break;

            case miEdit:
                Edit ();
                break;

            case miMoveArea:
                MoveArea ();
                break;

            case miOrder:
                Order ();
                break;

            case miVisible:
                Visible (0);
                break;

            case miVisible+1:
                Visible (1);
                break;

            case miMono:
                ChangeVideoMode (vmMono);
                break;

            case mi80x25:
                ChangeVideoMode (vmCO80);
                break;

            case mi80x30:
                ChangeVideoMode (vmVGA_80x30);
                break;

            case mi80x34:
                ChangeVideoMode (vmVGA_80x34);
                break;

            case mi94x34:
                ChangeVideoMode (vmVGA_94x34);
                break;

            case mi2A:
                ChangeVideoMode (vmET4_100x40);
                break;

            case miEditResource:
                EditResource ();
                break;

            case miMergeMsgText:
                MergeMsgText ();
                break;

        }

    }

    // Delete the current resource
    DeleteRes ();

    // Close the resource file
    CloseResourceFile ();

    return 0;
}




int main (int argc, char* argv [])
{
    ResEditApp MyApp (argc, argv);
    return MyApp.Run ();
}
