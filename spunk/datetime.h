/*****************************************************************************/
/*                                                                           */
/*                                DATETIME.H                                 */
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



#ifndef _DATETIME_H
#define _DATETIME_H



#include <math.h>
#include <time.h>

#include "stream.h"



// Forwards
class TimeDiff;
class Time;



/*****************************************************************************/
/*                              Utilities                                    */
/*****************************************************************************/



// Beware: The following must match the values in struct tm.tm_wday!
enum WeekDay { Son, Mon, Tue, Mid, Thu, Fri, Sat };


// Name of days and months
String LongNameOfDay (WeekDay aDay);
String ShortNameOfDay (WeekDay aDay);
String LongNameOfMonth (unsigned aMonth);
String ShortNameOfMonth (unsigned aMonth);

int IsLeapYear (unsigned aYear);
int IsLeapYear (const Time& aTime);

unsigned DaysOfYear (unsigned aYear);
unsigned DaysOfYear (const Time& aTime);

unsigned DaysOfMonth (unsigned aMonth, unsigned aYear);
unsigned DaysOfMonth (const Time& aTime);



/*****************************************************************************/
/*                              class TimeDiff                               */
/*****************************************************************************/



class TimeDiff: public Streamable {

    friend class Time;

protected:
    double      Diff;

    TimeDiff (StreamableInit);
    // Build constructor


public:
    TimeDiff (double = 0);
    // Constructor

    TimeDiff (const TimeDiff&);
    // Copy constructor

    // Derived from class Streamable
    virtual void Load (Stream& S);
    virtual void Store (Stream& S) const;
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    TimeDiff& operator = (const TimeDiff& rhs);
    // Assignment operator

    u32 GetSec ();
    // Get time diff in seconds

    // This functions are also declared as friends of class Time!
    friend inline Time operator - (const Time& lhs, const TimeDiff& rhs);
    friend inline Time operator + (const Time& lhs, const TimeDiff& rhs);

};



inline TimeDiff::TimeDiff (StreamableInit)
{
}



inline TimeDiff::TimeDiff (double Val) :
    Diff (Val)
{
}



inline TimeDiff::TimeDiff (const TimeDiff& X):
    Diff (X.Diff)
{
}



inline TimeDiff& TimeDiff::operator = (const TimeDiff& rhs)
{
    Diff = rhs.Diff;
    return *this;
}



inline u32 TimeDiff::GetSec ()
// Get time diff in seconds
{
    return (u32) Diff;
}



/*****************************************************************************/
/*                                class Time                                 */
/*****************************************************************************/



class Time: public Streamable {

protected:
    double      T;

public:
    Time ();
    Time (time_t Val);
    Time (tm& TM);
    Time (unsigned Year, unsigned Month, unsigned Day, unsigned Hour = 0,
          unsigned Min = 0, unsigned Sec = 0);
    Time (const Time&);
    Time (StreamableInit X);


    // Derived from class Streamable
    virtual void Load (Stream& S);
    virtual void Store (Stream& S) const;
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    // Setting variables
    void SetYear (unsigned Year);
    void SetMonth (unsigned Month);
    void SetDay (unsigned Day);
    void SetHour (unsigned Hour);
    void SetMin (unsigned Min);
    void SetSec (unsigned Sec);
    void SetTime (unsigned Hour, unsigned Min, unsigned Sec);
    void SetTime (unsigned SecondOfDay);
    void SetDate (unsigned Year, unsigned Month, unsigned Day);

    // Getting variables
    unsigned GetYear () const;
    unsigned GetMonth () const;
    unsigned GetDay () const;
    unsigned GetHour () const;
    unsigned GetMin () const;
    unsigned GetSec () const;
    void GetTime (unsigned& Hour, unsigned& Min, unsigned& Sec) const;
    u32 GetTime () const;
    void GetDate (unsigned& Year, unsigned& Month, unsigned& Day) const;
    void GetDateTime (unsigned& Year, unsigned& Month, unsigned& Day,
                      unsigned& WeekDay,
                      unsigned& Hour, unsigned& Min, unsigned& Sec) const;
    WeekDay GetDayOfWeek () const;

    // Conversion routines
    String DateStr () const;
    String TimeStr (int Seconds = 1) const;
    String DateTimeStr (int Seconds = 1) const;
    String DateTimeStr (const String& Fmt) const;

    // Name of days and months
    String LongNameOfDay () const;
    String ShortNameOfDay () const;
    String LongNameOfMonth () const;
    String ShortNameOfMonth () const;

    int IsLeapYear () const;
    unsigned DaysOfYear () const;
    unsigned DaysOfMonth () const;

    //
    Time& operator = (const Time& rhs);
    Time& operator -= (const TimeDiff& rhs);
    Time& operator += (const TimeDiff& rhs);

    //
    friend inline TimeDiff operator - (const Time& lhs, const Time& rhs);
    friend inline Time operator - (const Time& lhs, const TimeDiff& rhs);
    friend inline Time operator + (const Time& lhs, const TimeDiff& rhs);

    friend inline int operator == (const Time& lhs, const Time& rhs);
    friend inline int operator != (const Time& lhs, const Time& rhs);
    friend inline int operator > (const Time& lhs, const Time& rhs);
    friend inline int operator < (const Time& lhs, const Time& rhs);
    friend inline int operator >= (const Time& lhs, const Time& rhs);
    friend inline int operator <= (const Time& lhs, const Time& rhs);

};



inline Time::Time (StreamableInit)
{
}



inline Time::Time () :
    T (time (NULL))
{
}



inline Time::Time (time_t Val) :
    T (Val)
{
}



inline Time::Time (const Time& X) :
    T (X.T)
{
}



inline String Time::LongNameOfDay () const
{
    return ::LongNameOfDay (GetDayOfWeek ());
};



inline String Time::ShortNameOfDay () const
{
    return ::ShortNameOfDay (GetDayOfWeek ());
};



inline String Time::LongNameOfMonth () const
{
    return ::LongNameOfMonth (GetMonth());
}



inline String Time::ShortNameOfMonth () const
{
    return ::ShortNameOfMonth (GetMonth());
}



inline int Time::IsLeapYear () const
{
    return ::IsLeapYear (GetYear ());
}



inline unsigned Time::DaysOfYear () const
{
    return ::DaysOfYear (GetYear ());
}



inline unsigned Time::DaysOfMonth () const
{
    return ::DaysOfMonth (GetMonth (), GetYear ());
}



inline Time& Time::operator = (const Time& rhs)
{
    T = rhs.T;
    return *this;
}



inline Time& Time::operator -= (const TimeDiff& rhs)
{
    T -= rhs.Diff;
    return *this;
}



inline Time& Time::operator += (const TimeDiff& rhs)
{
    T += rhs.Diff;
    return *this;
}



inline TimeDiff operator - (const Time& lhs, const Time& rhs)
{
    return TimeDiff (lhs.T - rhs.T);
}



inline Time operator - (const Time& lhs, const TimeDiff& rhs)
{
    return Time ((time_t) (lhs.T - rhs.Diff));
}



inline Time operator + (const Time& lhs, const TimeDiff& rhs)
{
    return Time ((time_t) (lhs.T + rhs.Diff));
}



inline int operator == (const Time& lhs, const Time& rhs)
{
    return fabs (lhs.T - rhs.T) < 1;
}



inline int operator != (const Time& lhs, const Time& rhs)
{
    return fabs (lhs.T - rhs.T) >= 1;
}



inline int operator > (const Time& lhs, const Time& rhs)
{
    return (lhs.T - rhs.T) >= 1;
}



inline int operator < (const Time& lhs, const Time& rhs)
{
    return (lhs.T - rhs.T) <= -1;
}



inline int operator >= (const Time& lhs, const Time& rhs)
{
    return (lhs.T - rhs.T) > -1;
}



inline int operator <= (const Time& lhs, const Time& rhs)
{
    return (lhs.T - rhs.T) < 1;
}



inline Time Now ()
// Return the current time
{
    return Time (time (NULL));
}



// End of DATETIME.H

#endif



