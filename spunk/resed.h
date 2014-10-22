/*****************************************************************************/
/*                                                                           */
/*                                  RESED.H                                  */
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



#ifndef _RESED_H
#define _RESED_H



#if defined(DOS) && defined(__BORLANDC__)
#include <alloc.h>
#endif

#include <stdio.h>

#include "strcoll.h"
#include "msgcoll.h"
#include "resource.h"
#include "statline.h"
#include "menue.h"
#include "menuitem.h"
#include "program.h"
#include "stdmenue.h"
#include "stdmsg.h"
#include "menuedit.h"
#include "listbox.h"
#include "streamid.h"



/*****************************************************************************/
/*                              Menue constants                              */
/*****************************************************************************/



const i16 miNone                =    1;

const i16 miSystem              = 1000;
const i16 miAbout               = 1100;
const i16 miQuit                = 1200;

const i16 miFile                = 2000;
const i16 miOpen                = 2100;
const i16 miClose               = 2200;
const i16 miPack                = 2300;
const i16 miRead                = 2400;
const i16 miWrite               = 2500;
const i16 miPrint               = 2600;
const i16 miNew                 = 2700;
const i16 miAddData             = 2800;
const i16 miAddStream           = 2900;
const i16 miMerge               = 3100;

const i16 miWindow              = 4000;
const i16 miHeader              = 4100;
const i16 miFooter              = 4200;
const i16 miSize                = 4300;
const i16 miColor               = 4400;
const i16 miBackgroundChar      = 4500;
const i16 miNumber              = 4600;
const i16 miFlags               = 4700;
const i16 miCanMove             = 4710;
const i16 miCanResize           = 4720;
const i16 miIgnoreAccept        = 4730;
const i16 miModal               = 4740;
const i16 miCenterX             = 4750;
const i16 miCenterY             = 4760;
const i16 miLRLink              = 4770;
const i16 miTest                = 4800;

const i16 miItems               = 5000;
const i16 miAddItem             = 5100;

const i16 miAddWindowItem       = 5110;
const i16 miAddTextItem         = 5112;
const i16 miAddItemLabel        = 5115;
const i16 miAddMenueItems       = 5120;
const i16 miAddMenueItem        = 5121;
const i16 miAddSubMenueItem     = 5122;
const i16 miAddMenueBarItem     = 5123;
const i16 miAddFloatItem        = 5124;
const i16 miAddExpFloatItem     = 5125;
const i16 miAddLongItem         = 5126;
const i16 miAddHexItem          = 5127;
const i16 miAddStringItem       = 5128;
const i16 miAddRStringItem      = 5129;
const i16 miAddToggleItem       = 5130;
const i16 miAddOnOffItem        = 5131;
const i16 miAddNoYesItem        = 5132;
const i16 miAddDateItem         = 5133;
const i16 miAddTimeItem         = 5134;

const i16 miAddMenueEdits       = 5140;
const i16 miAddEditLine         = 5141;
const i16 miAddFloatEdit        = 5142;
const i16 miAddExpFloatEdit     = 5143;
const i16 miAddLongEdit         = 5144;
const i16 miAddHexEdit          = 5145;
const i16 miAddPasswordEdit     = 5146;
const i16 miAddFileEdit         = 5147;
const i16 miAddDateEdit         = 5148;
const i16 miAddTimeEdit         = 5149;
const i16 miAddTextEdit         = 5150;

const i16 miAddListBox          = 5160;
const i16 miAddFloatListBox     = 5161;

const i16 miAddMenueLine        = 5170;

const i16 miDelete              = 5300;
const i16 miCopy                = 5400;
const i16 miEdit                = 5500;
const i16 miMoveArea            = 5600;
const i16 miOrder               = 5700;

const i16 miOptions             = 7000;
const i16 miVisible             = 7100;
const i16 miMode                = 7200;
const i16 miMono                = 7210;
const i16 mi80x25               = 7220;
const i16 mi80x30               = 7230;
const i16 mi80x34               = 7240;
const i16 mi94x34               = 7250;
const i16 mi2A                  = 7260;

const i16 miEditActions         = 8000;
const i16 miEditResource        = 8100;
const i16 miMergeMsgText        = 8200;



/*****************************************************************************/
/*                             class ResEditApp                              */
/*****************************************************************************/



class ResEditApp: public Program {

    ResourceFile*       ResFile;
    String              ResFileName;
    Streamable*         Res;
    String              ResName;
    int                 ResChanged;
    u16                 ResID;

private:
    static int PrintOneItem (ListNode<WindowItem>* Node, void* _F);
    // Print the data of one menue item

    static int PrintOneMsg (Msg* M, void* _F);
    // Print one message from a message collection

protected:
    virtual void GetEvent ();
    //

    void CloseResourceFile ();
    void OpenResourceFile (const String & Name);
    void ReadResource (const String &Name);
    void WriteResource (const String &Name);
    int  SaveResource ();
    void NamePrompt (const String &Name, String &Val, int &Abort);
    void FloatPrompt (const String& Text, double& Val, double Min, double Max, int& Abort);
    void LongPrompt (const String& Text, i32& Val, i32 Min, i32 Max, int& Abort);
    void IDPrompt (i16& ID, int& Abort);
    void EditIDPrompt (i16& ID, int& Abort);
    void MinMaxPrompt (i32& Min, i32& Max, i32 MinVal, i32 MaxVal, int& Abort);
    void ResEditApp::FMinMaxPrompt (double& Min, double& Max, double MinVal,
                                double MaxVal, int& Abort);
    void TextIDPrompt (String& Text, i16& ID, int& Abort);
    void DeleteRes ();
    void AssignRes (Streamable *);
    WindowItem * ChooseItem ();
    i16 NextID (i16 StartID = 1);
    i16 SearchItemYPos ();
    i16 SearchItemXPos ();
    int SearchMsgID (MsgCollection *M, u16 StartNum = 0);
    void AddItem (WindowItem *Item);
    void EditMsgCollection ();
    void PrintMsgCollection (FILE* F);
    void PrintMenue (FILE* F);
    void MoveResizeInside (Window* W, GenericMenue* M, int Resize, int& Abort);

    String GetItemText (WindowItem* Item);
    // Extract and return the text of a window item.

    void SetItemText (WindowItem* Item, String NewText);
    // Set the new item text, handle @ as the hotkey position.

    unsigned GetItemState (WindowItem* Item);
    // Extract and return the item state 0..2

    void SetItemState (WindowItem* Item, unsigned State);
    // Set the state according to the state given (0..2)

    String GetCharSetString (WindowItem* I);
    // Get a character set from a window item as a string

    void SetCharSetString (WindowItem* I, String Set);
    // Set a character set of a window item from a string

    const CharSet& GetCharSet (WindowItem* I);
    // Get a character set from an item

    void SetCharSet (WindowItem* I, const CharSet& CS);
    // Set a character set

    String ChooseRes ();
    // Choose a resource from the file and return it's name. The function
    // returns the empty string on a user abort.

    Streamable* LoadRes (const String& Name);
    // Load the resource with the given namen from the resource file. Return
    // a NULL pointer in case of errors.

public:
    ResEditApp (int argc, char** argv);

    static TopMenueBar *CreateMenueBar ();
    static BottomStatusLine *CreateStatusLine ();
    virtual int Run ();

    void Open ();
    void Close ();
    void Pack ();
    void Read ();
    void Write ();
    void Print ();
    void New ();
    void AddData ();
    void AddStream ();
    void Merge ();

    void Header ();
    void Footer ();
    void Size ();
    void Color ();
    void BackgroundChar ();
    void Number ();
    void CanMove (int On);
    void CanResize (int On);
    void IgnoreAccept (int On);
    void Modal (int On);
    void LRLink (int On);
    void CenterX (int On);
    void CenterY (int On);
    void Test ();

    void AddWindowItem ();
    void AddTextItem ();
    void AddItemLabel ();

    void AddMenueItem ();
    void AddSubMenueItem ();
    void AddMenueBarItem ();
    void AddFloatItem ();
    void AddExpFloatItem ();
    void AddLongItem ();
    void AddHexItem ();
    void AddStringItem ();
    void AddRStringItem ();
    void AddToggleItem ();
    void AddOffOnItem ();
    void AddNoYesItem ();
    void AddDateItem ();
    void AddTimeItem ();

    void AddEditLine ();
    void AddFloatEdit ();
    void AddExpFloatEdit ();
    void AddLongEdit ();
    void AddHexEdit ();
    void AddPasswordEdit ();
    void AddFileEdit ();
    void AddDateEdit ();
    void AddTimeEdit ();
    void AddTextEdit ();
    void AddMenueLine ();

    void Delete ();
    void Copy ();
    void Edit ();
    void MoveArea ();
    void Order ();

    void ItemMenue (WindowItem* Item);
    void ChangeLimits (WindowItem* Item);
    void ChangeSubMenue (WindowItem* Item);
    void ChangeCharset (WindowItem* Item);

    void Visible (int On);

    void EditResource ();
    void MergeMsgText ();       // Merge a text file containing messages

};



// End of RESED.H

#endif


