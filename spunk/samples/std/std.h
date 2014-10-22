#include <fcntl.h>
#include <unistd.h>
#include "program.h"
#include "statline.h"
#include "stream.h"
#include "streamid.h"

static char rcsidh1[] __attribute__ ((unused)) =
"$Id: std.h,v 0.17 1995/11/27 14:51:26 ralf Exp $";
//=======================================================================
int fastlog2 (int);

const STD_ID = ID_USER+1288;          // persistent objects' IDs
const STD_ID_Options = STD_ID;

enum startmenu_iID { iStartDemo=1, iContDemo, iSingle, iTablet, 
                     iOptions, iExit };

enum optionsmenu_iID { iPortnr=1, iStartStop=3, iDir, iDrive, iSPause,
                     iLPause, iLeft, iRight=19, iUp=25, iDown=34, iSKey=44,
                     iDauer=48, iRelayNr };

//---------------------------Options------------------------------------

class Options : public Streamable
{
public:
  i16 portnr;
  i16 dir, startstop, drive, pause, longpause;
  i16 R_N, R_W, R_E, R_S, R_x;

  Options () 
  : portnr(1), dir(350), startstop(400), drive(50), pause(100), longpause(700), 
    R_N(2), R_W(1), R_E(8), R_S(4), R_x(16) {}
  Options (StreamableInit)                  {}
    
  virtual void Load (Stream& S) { S >> portnr >> dir >> startstop 
   >> drive >> pause >> longpause >> R_N >> R_W >> R_E >> R_S >> R_x; }
  virtual void Store (Stream& S) const { S.Seek(0); S << portnr << dir 
   << startstop << drive << pause << longpause << R_N << R_W << R_E << R_S << R_x; }
  virtual u16 StreamableID () const         { return STD_ID_Options; }
  static Streamable* Build ()               { return new Options; }
};

//===============================Std=====================================
class Std : public Program 
{ 
public:
  Std(int, char**);
  ~Std();
  
  int Run();
  static BottomStatusLine* CreateStatusLine();
  
  void StartDemo();
  void ContDemo();
  void Single();
  void Tablet();
  void Config();

private:
  Options opt_;
  FileStream optf_;
};


//--------------------------ParallelPort--------------------------------
#ifdef LINUX
class ParallelPort
{
public:
  ParallelPort (int portnr) 
  { 
    char dev[] = "/dev/lp0"; 
    dev[7] += portnr;
    fd_ = open (dev, O_WRONLY);
    if (fd_<0) { perror(strerror(errno)); exit(1); }
  }
  ~ParallelPort()                 { close (fd_); }
  void operator << (char c) const { write (fd_, &c, 1); }

private:
  int fd_;
};
#endif
#ifdef DOS32
class ParallelPort
{
public:
  ParallelPort (int portnr) 
  { 
    char dev[] = "prn";
//    dev[3] += portnr;
    fd_ = open (dev, O_WRONLY);
    if (fd_<0) { perror(strerror(errno)); exit(1); }
  }
  ~ParallelPort()                 { close (fd_); }
  void operator << (char c) const { write (fd_, &c, 1); }

private:
  int fd_;
};
#endif

//--------------------------------RI------------------------------------
const R_1 = 0x01;
const R_2 = 0x02;
const R_3 = 0x04;
const R_4 = 0x08;
const R_5 = 0x10;
const R_6 = 0x20;
const R_7 = 0x40;
const R_8 = 0x80;
const R_Pause = 0x1000;

inline void wait_for (int ms) { usleep (1000*ms); }

struct RISignal 
{ 
  int relais, ms; 
  RISignal (int r, int t) : relais(r), ms(t) {}
};

class RI  // class to switch on individual relais on a relaisinterface
          // for a specific time.  NOTE:  Signals are handled serially.
          // If you want to change relais in parallel, you have to keep the
          // RI state in the class, write a timer...
{
public:
  RI (int portnr) : pp_(portnr) {}
  ~RI()                         {}
  void relais_on (char r) const { pp_ << r; }
  void relais_off ()      const { pp_ << 0; }
  
  friend const RI& operator<< (const RI& ri, RISignal t) 
  { 
    ri.relais_on (char(t.relais)); 
    wait_for (t.ms);
    ri.relais_off ();
    return ri;
  }

private:
  const ParallelPort pp_;
};

//---------------------------------ST-------------------------------------
enum TCommand { tcStartStop=0, tcDrive, tcMode, tcSW, tcS, tcSE, tcW, tcE,
                tcNW, tcN, tcNE };
enum SCommand { };                

const portnr=1;

class ST
{
public:
  ST (Options &, int);
  ~ST() {}
  friend ST& operator<< (ST&, TCommand);
  friend ST& operator<< (ST&, SCommand);

private:
  const Options& opt_;
  const RI ri_;
};