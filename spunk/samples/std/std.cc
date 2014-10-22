// *********************************************************************
// SuperTrans Demo  (c) 1995,1996 Ralf Stephan <ralf@ark.franken.de>

static char rcsid[] __attribute__ ((unused)) =
"$Id: std.cc,v 0.17 1995/11/27 14:51:26 ralf Exp $";
// *********************************************************************

#include <ctype.h>

#include "std.h"
#include "menue.h"
#include "window.h"
#include "kbd.h"

__link (Options, STD_ID_Options);

//===========================================================
main (int argc, char** argv)
{
  Std std (argc, argv);
  return std.Run();  
}

//===========================================================
Std::Std(int c, char** v) 
: Program (c,v,0,&CreateStatusLine,"std"), optf_("std.cfg")
{
  if (optf_.GetSize()>0)
    optf_ >> opt_;
}

Std::~Std()
{

}

BottomStatusLine* Std::CreateStatusLine ()
{
  const u32 Flags = siEnd | siSelectChooseKeys;
  return new BottomStatusLine (Flags);
}

int Std::Run()
{
  Menue* M = (Menue*) LoadResource ("@STD.startmenu");
  M->GrayItem (iStartDemo);
  M->GrayItem (iContDemo);
  M->GrayItem (iSingle);
  M->Activate ();

  while (true) 
  {
    int Sel = M->GetChoice ();
    if (!Sel || Sel==iExit) break;
    switch (Sel) 
    {
      case iStartDemo: StartDemo(); break;
      case iContDemo : ContDemo();  break;
      case iSingle   : Single();    break;
      case iTablet   : Tablet();    break;
      case iOptions  : Config();   break;
    }
  }

  delete M;
  return 0;
}

//================================================================
void Std::StartDemo()
{

}

//----------------------------------------------------------------
void Std::ContDemo()
{

}

//----------------------------------------------------------------
void Std::Single()
{

}

//----------------------------------------------------------------
void Std::Tablet()
{
  Rect screen = Background->OuterBounds();
  Rect r(0,0,37,12);
  r.Center(screen,cfCenterAll);
  
  Window w(r);
  w.SetHeader("Tablettsteuerung");
  w.Clear();

  int x=0, y=0;
  w.Write(x,++y,"   Fahren");
  w.Write(x,++y,"  \\   |   /    0     - Start/Stop");
  w.Write(x,++y,"   \\  |  /");
  w.Write(x,++y,"    7 8 9      +     - Drive");
  w.Write(x,++y,"  --4   6--");
  w.Write(x,++y,"    1 2 3      Enter - Mode");
  w.Write(x,++y,"   /  |  \\");
  w.Write(x,++y,"  /   |   \\    Esc   - Beenden");

  w.Activate();

  ST st (opt_,opt_.portnr);
  while (1)
  {
    Key key = Kbd->Get();
    if (key == vkAbort) 
      break;
    if (!isdigit(key) && !(key==kbEnter || key=='+'))
      continue;

    switch (key)
    {
    case kbEnter: st << tcMode; break;
    case '+'    : st << tcDrive; break;
    case '0'    : st << tcStartStop; break;
    case '1'    : st << tcSW; break;
    case '2'    : st << tcS; break;
    case '3'    : st << tcSE; break;
    case '4'    : st << tcW; break;
    case '6'    : st << tcE; break;
    case '7'    : st << tcNW; break;
    case '8'    : st << tcN; break;
    case '9'    : st << tcNE; break;
    default     : break;
    }
  }
}

//----------------------------------------------------------------
void Std::Config()
{
  Menue* M = (Menue*) LoadResource ("@STD.options");
  M->DeactivateItem (iDauer);
  M->DeactivateItem (iRelayNr);

  M->SetToggleValue (iPortnr, opt_.portnr-1);
  M->SetLongValue (iStartStop, opt_.startstop);
  M->SetLongValue (iDrive, opt_.drive);
  M->SetLongValue (iDir, opt_.dir);
  M->SetLongValue (iSPause, opt_.pause);
  M->SetLongValue (iLPause, opt_.longpause);
  M->SetToggleValue (iLeft, fastlog2(opt_.R_W));
  M->SetToggleValue (iRight, fastlog2(opt_.R_E));
  M->SetToggleValue (iUp, fastlog2(opt_.R_N));
  M->SetToggleValue (iDown, fastlog2(opt_.R_S));
  M->SetToggleValue (iSKey, fastlog2(opt_.R_x));

  M->Activate ();
  
  while (true) 
  {
    int Sel = M->GetChoice ();
    if (!Sel) break;
  }
  
  opt_.portnr = M->GetToggleValue (iPortnr)+1;
  opt_.startstop = M->GetLongValue (iStartStop);
  opt_.drive = M->GetLongValue (iDrive);
  opt_.dir = M->GetLongValue (iDir);
  opt_.pause = M->GetLongValue (iSPause);
  opt_.longpause = M->GetLongValue (iLPause);
  opt_.R_W = 1 << (M->GetToggleValue (iLeft));
  opt_.R_E = 1 << (M->GetToggleValue (iRight));
  opt_.R_N = 1 << (M->GetToggleValue (iUp));
  opt_.R_S = 1 << (M->GetToggleValue (iDown));
  opt_.R_x = 1 << (M->GetToggleValue (iSKey));

  optf_ << opt_;
  delete M;
}

ST::ST (Options& o, int portnr) : opt_(o), ri_(portnr) {}


//============================ friends =====================================

ST& operator<< (ST& st, TCommand t)
{
  const RI& ri = st.ri_;
  const Options& opt = st.opt_;
  const dir = opt.dir, startstop = opt.startstop, drive = opt.drive, 
        pause = opt.pause, longpause = opt.longpause,
        R_N=opt.R_N, R_W=opt.R_W, R_E=opt.R_E, R_S=opt.R_S, R_x=opt.R_x;
  
  switch (t)
  {
  case tcStartStop: ri << RISignal(R_x,     startstop);  break;
  
  case tcDrive    : ri << RISignal(R_x,     drive)
                       << RISignal(R_Pause, longpause);  break;
  
  case tcMode     : ri << RISignal(R_x,     drive)
                       << RISignal(R_Pause, pause)
                       << RISignal(R_x,     drive)     
                       << RISignal(R_Pause, longpause);  break;
  
  case tcSW       : ri << RISignal(R_S|R_W, dir);        break;
  case tcS        : ri << RISignal(R_S,     dir);        break;
  case tcSE       : ri << RISignal(R_S|R_E, dir);        break;
  case tcW        : ri << RISignal(R_W,     dir);        break;
  case tcE        : ri << RISignal(R_E,     dir);        break;
  case tcNW       : ri << RISignal(R_N|R_W, dir);        break;
  case tcN        : ri << RISignal(R_N,     dir);        break;
  case tcNE       : ri << RISignal(R_N|R_E, dir);        break;
  }

  return st;
}

int fastlog2(int n) 
{
  int num_bits, power = 0;

  if((n < 2) || (n % 2 != 0)) return(0);
  num_bits = sizeof(int) * 8;   /* How big are ints on this machine? */

  while(power <= num_bits) {
    n >>= 1;
    ++power;
    if(n & 0x01) {
      if(n > 1)	continue;
      else return(power);
    }
  }
  return(0);
}
