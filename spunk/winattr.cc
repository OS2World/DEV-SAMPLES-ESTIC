/*****************************************************************************/
/*									     */
/*				   WINATTR.CC				     */
/*									     */
/* (C) 1993-95	Ullrich von Bassewitz					     */
/*		Zwehrenbuehlstrasse 33					     */
/*		D-72070 Tuebingen					     */
/* EMail:	uz@ibb.schwaben.com					     */
/*									     */
/*****************************************************************************/



// $Id$
//
// $Log$
//
//



#include "wincolor.h"
#include "palette.h"
#include "winattr.h"



/*****************************************************************************/
/*				   Palettes				     */
/*****************************************************************************/



// Two mono palettes
static unsigned char plMono1 [] = {
    ((coBlack	  << 4) + coNormal		),	// Frame: Passive
    ((coBlack	  << 4) + coNormal + coHigh	),	//	  Active
    ((coBlack	  << 4) + coNormal + coHigh	),	//	  Resizing
    ((coBlack	  << 4) + coNormal		),	// Text:  Normal
    ((coNormal	  << 4) + coBlack		),	//	  Invers
    ((coBlack	  << 4) + coUnderline + coHigh	),	//	  Selected
    ((coBlack	  << 4) + coUnderline + coHigh	),	//	  High
    ((coNormal	  << 4) + coBlack		),	//	  HighInv.
    ((coBlack	  << 4) + coNormal		),	//	  Grayed
    ((coNormal	  << 4) + coBlack		),	//	  GrayedInv
    ((coNormal	  << 4) + coBlack		),	// Edit:  Normal
    ((coNormal	  << 4) + coBlack		),	//	  High
    ((coBlack	  << 4) + coNormal + coHigh	)	//	  Bar
};



static unsigned char plMono2 [] = {
    ((coNormal	  << 4) + coBlack		),	// Frame: Passive
    ((coNormal	  << 4) + coBlack		),	//	  Active
    ((coNormal	  << 4) + coBlack		),	//	  Resizing
    ((coNormal	  << 4) + coBlack		),	// Text:  Normal
    ((coBlack	  << 4) + coNormal		),	//	  Invers
    ((coNormal	  << 4) + coUnderline + coBlack ),	//	  Selected
    ((coNormal	  << 4) + coUnderline + coBlack ),	//	  High
    ((coBlack	  << 4) + coNormal + coHigh	),	//	  HighInv.
    ((coNormal	  << 4) + coBlack		),	//	  Grayed
    ((coBlack	  << 4) + coNormal		),	//	  GrayedInv
    ((coBlack	  << 4) + coNormal		),	// Edit:  Normal
    ((coBlack	  << 4) + coNormal + coHigh	),	//	  High
    ((coNormal	  << 4) + coBlack		)	//	  Bar
};



// Color palettes
static unsigned char plBlue [] = {
    ((coBlue	  << 4) + coLightGray ),    // Frame: Passive
    ((coBlue	  << 4) + coWhite     ),    //	      Active
    ((coBlue	  << 4) + coLightGreen),    //	      Resizing
    ((coBlue	  << 4) + coLightGray ),    // Text:  Normal
    ((coLightGray << 4) + coBlue      ),    //	      Invers
    ((coBlue	  << 4) + coWhite     ),    //	      Selected
    ((coBlue	  << 4) + coYellow    ),    //	      High
    ((coLightGray << 4) + coRed       ),    //	      HighInv
    ((coBlue	  << 4) + coDarkGray  ),    //	      Grayed
    ((coLightGray << 4) + coDarkGray  ),    //	      GrayedInv
    ((coLightGray << 4) + coBlack     ),    // Edit:  Normal
    ((coLightGray << 4) + coWhite     ),    //	      High
    ((coCyan	  << 4) + coBlack     )     //	      Bar
};



static unsigned char plCyan [] = {
    ((coCyan	  << 4) + coBlue      ),    // Frame: Passive
    ((coCyan	  << 4) + coWhite     ),    //	      Active
    ((coCyan	  << 4) + coLightGreen),    //	      Resizing
    ((coCyan	  << 4) + coBlack     ),    // Text:  Normal
    ((coGreen	  << 4) + coWhite     ),    //	      Invers
    ((coCyan	  << 4) + coWhite     ),    //	      Selected
    ((coCyan	  << 4) + coYellow    ),    //	      High
    ((coGreen	  << 4) + coRed       ),    //	      HighInv
    ((coCyan	  << 4) + coLightGray ),    //	      Grayed
    ((coGreen	  << 4) + coDarkGray  ),    //	      GrayedInv
    ((coLightGray << 4) + coBlue      ),    // Edit:  Normal
    ((coLightGray << 4) + coGreen     ),    //	      High
    ((coMagenta   << 4) + coWhite     )     //	      Bar
};



static unsigned char plGray [] = {
    ((coLightGray << 4) + coBlack     ),    // Frame: Passive
    ((coLightGray << 4) + coWhite     ),    //	      Active
    ((coLightGray << 4) + coLightGreen),    //	      Resizing
    ((coLightGray << 4) + coBlack     ),    // Text:  Normal
    ((coGreen	  << 4) + coBlack     ),    //	      Invers
    ((coLightGray << 4) + coWhite     ),    //	      Selected
    ((coLightGray << 4) + coRed       ),    //	      High
    ((coGreen	  << 4) + coRed       ),    //	      HighInv
    ((coLightGray << 4) + coDarkGray  ),    //	      Grayed
    ((coGreen	  << 4) + coDarkGray  ),    //	      GrayedInv
    ((coBlue	  << 4) + coLightGray ),    // Edit:  Normal
    ((coBlue	  << 4) + coWhite     ),    //	      High
    ((coMagenta   << 4) + coWhite     )     //	      Bar
};



static unsigned char plRed [] = {
    ((coRed	  << 4) + coYellow    ),    // Frame: Passive
    ((coRed	  << 4) + coWhite     ),    //	      Active
    ((coRed	  << 4) + coLightGreen),    //	      Resizing
    ((coRed	  << 4) + coLightGray ),    // Text:  Normal
    ((coLightGray << 4) + coRed       ),    //	      Invers
    ((coRed	  << 4) + coWhite     ),    //	      Selected
    ((coRed	  << 4) + coYellow    ),    //	      High
    ((coLightGray << 4) + coYellow    ),    //	      HighInv
    ((coLightGray << 4) + coDarkGray  ),    //	      Grayed
    ((coLightGray << 4) + coDarkGray  ),    //	      GrayedInv
    ((coLightGray << 4) + coYellow    ),    // Edit:  Normal
    ((coLightGray << 4) + coWhite     ),    //	      High
    ((coBlack	  << 4) + coWhite     )     //	      Bar
};



static unsigned char plBlack [] = {
    ((coBlack	  << 4) + coLightGray ),    // Frame: Passive
    ((coBlack	  << 4) + coWhite     ),    //	      Active
    ((coBlack	  << 4) + coLightGreen),    //	      Resizing
    ((coBlack	  << 4) + coLightGray ),    // Text:  Normal
    ((coLightGray << 4) + coBlack     ),    //	      Invers
    ((coBlack	  << 4) + coWhite     ),    //	      Selected
    ((coBlack	  << 4) + coYellow    ),    //	      High
    ((coLightGray << 4) + coYellow    ),    //	      HighInv
    ((coBlack	  << 4) + coDarkGray  ),    //	      Grayed
    ((coLightGray << 4) + coDarkGray  ),    //	      GrayedInv
    ((coLightGray << 4) + coBlack     ),    // Edit:  Normal
    ((coLightGray << 4) + coWhite     ),    //	      High
    ((coMagenta   << 4) + coBlack     )     //	      Bar
};



static unsigned char plEH [] = {
    ((coLightGray << 4) + coRed       ),    // Frame: Passive
    ((coLightGray << 4) + coRed       ),    //	      Active
    ((coLightGray << 4) + coLightGreen),    //	      Resizing
    ((coLightGray << 4) + coBlack     ),    // Text:  Normal
    ((coBlack	  << 4) + coLightGray ),    //	      Invers
    ((coLightGray << 4) + coWhite     ),    //	      Selected
    ((coLightGray << 4) + coRed       ),    //	      High
    ((coGreen	  << 4) + coRed       ),    //	      HighInv
    ((coLightGray << 4) + coDarkGray  ),    //	      Grayed
    ((coGreen	  << 4) + coDarkGray  ),    //	      GrayedInv
    ((coBlue	  << 4) + coBlack     ),    // Edit:  Normal
    ((coBlue	  << 4) + coWhite     ),    //	      High
    ((coMagenta   << 4) + coWhite     )     //	      Bar
};



static unsigned char plRoot [] = {
    ((coBlack	  << 4) + coLightGray ),    // Frame: Passive
    ((coBlack	  << 4) + coWhite     ),    //	      Active
    ((coBlack	  << 4) + coLightGreen),    //	      Resizing
    ((coBlack	  << 4) + coLightGray ),    // Text:  Normal
    ((coLightGray << 4) + coBlack     ),    //	      Invers
    ((coBlack	  << 4) + coWhite     ),    //	      Selected
    ((coBlack	  << 4) + coLightGreen),    //	      High
    ((coLightGray << 4) + coWhite     ),    //	      HighInv
    ((coBlack	  << 4) + coDarkGray  ),    //	      Grayed
    ((coLightGray << 4) + coDarkGray  ),    //	      GrayedInv
    ((coLightGray << 4) + coBlack     ),    // Edit:  Normal
    ((coLightGray << 4) + coLightGreen),    //	      High
    ((coMagenta   << 4) + coWhite     )    //	     Bar
};



static unsigned char plHelp [] = {
    ((coLightGray << 4) + coBlack     ),    // Frame: Passive
    ((coLightGray << 4) + coWhite     ),    //	      Active
    ((coLightGray << 4) + coLightGreen),    //	      Resizing
    ((coLightGray << 4) + coBlack     ),    // Text:  Normal
    ((coBlack	  << 4) + coLightGray ),    //	      Invers
    ((coLightGray << 4) + coWhite     ),    //	      Selected
    ((coLightGray << 4) + coLightBlue ),    //	      High
    ((coGreen	  << 4) + coRed       ),    //	      HighInv
    ((coLightGray << 4) + coDarkGray  ),    //	      Grayed
    ((coGreen	  << 4) + coDarkGray  ),    //	      GrayedInv
    ((coBlue	  << 4) + coBlack     ),    // Edit:  Normal
    ((coBlue	  << 4) + coWhite     ),    //	      High
    ((coMagenta   << 4) + coWhite     )     //	      Bar
};



static unsigned char plFSel [] = {
    ((coLightGray << 4) + coBlack     ),    // Frame: Passive
    ((coLightGray << 4) + coWhite     ),    //	      Active
    ((coLightGray << 4) + coLightGreen),    //	      Resizing
    ((coLightGray << 4) + coBlack     ),    // Text:  Normal
    ((coBlue	  << 4) + coLightGray ),    //	      Invers
    ((coLightGray << 4) + coWhite     ),    //	      Selected
    ((coLightGray << 4) + coYellow    ),    //	      High
    ((coBlue	  << 4) + coYellow    ),    //	      HighInv
    ((coLightGray << 4) + coDarkGray  ),    //	      Grayed
    ((coBlue	  << 4) + coLightGray ),    //	      GrayedInv
    ((coBlue	  << 4) + coLightGray ),    // Edit:  Normal
    ((coBlue	  << 4) + coWhite     ),    //	      High
    ((coMagenta   << 4) + coWhite     ),    // Balken im Fenster
};



/*****************************************************************************/
/*			     Module initialization			     */
/*****************************************************************************/



void InitWinAttr ()
// Initialize window attributes
{
    // Create a new palette
    Pal = new Palette;

    // Add palette entries
    Pal->Add (plMono1, plBlue);
    Pal->Add (plMono1, plGray);
    Pal->Add (plMono2, plCyan);
    Pal->Add (plMono2, plRed);
    Pal->Add (plMono1, plBlack);
    Pal->Add (plMono2, plEH);
    Pal->Add (plMono1, plRoot);
    Pal->Add (plMono1, plHelp);
    Pal->Add (plMono1, plFSel);
}



void DoneWinAttr ()
// Clean up after use
{
    // Delete the used palette
    delete Pal;
    Pal = NULL;
}
