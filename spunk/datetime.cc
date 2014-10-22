/*****************************************************************************/
/*                                                                           */
/*                                DATETIME.CC                                */
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



#include "msgid.h"
#include "national.h"
#include "streamid.h"
#include "progutil.h"
#include "datetime.h"



/*****************************************************************************/
/*                             message constants                             */
/*****************************************************************************/



const u16 msLongNameOfDayBase           = MSGBASE_DATETIME +  0;
const u16 msShortNameOfDayBase          = MSGBASE_DATETIME +  7;
const u16 msLongNameOfMonthBase         = MSGBASE_DATETIME + 14;
const u16 msShortNameOfMonthBase        = MSGBASE_DATETIME + 26;



/*****************************************************************************/
/*                             Number days per month                         */
/*****************************************************************************/



static unsigned LeapYearTab [12] = {
    31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};
static unsigned NormYearTab [12] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};



/*****************************************************************************/
/*                              utilities                                    */
/*****************************************************************************/



String LongNameOfDay (WeekDay aDay)
{
    return LoadMsg (u16 (msLongNameOfDayBase + aDay));
}



String ShortNameOfDay (WeekDay aDay)
{
    return LoadMsg (u16 (msShortNameOfDayBase + aDay));
}



String LongNameOfMonth (unsigned aMonth)
{
    CHECK ((aMonth >= 1) && (aMonth <= 12));
    return LoadMsg (u16 (msLongNameOfMonthBase + aMonth - 1));
}



String ShortNameOfMonth (unsigned aMonth)
{
    CHECK ((aMonth >= 1) && (aMonth <= 12));
    return LoadMsg (u16 (msShortNameOfMonthBase + aMonth - 1));
}



int IsLeapYear (unsigned aYear)
{
    return ((aYear % 4) == 0) && ((aYear % 100) != 0) ? 1 : 0;
}



int IsLeapYear (const Time& aTime)
{
    return IsLeapYear (aTime.GetYear());
}



unsigned DaysOfYear (unsigned aYear)
{
    return IsLeapYear (aYear) ? 366 : 365;
}



unsigned DaysOfYear (const Time& aTime)
{
    return DaysOfYear (aTime.GetYear ());
}



unsigned DaysOfMonth (unsigned aMonth, unsigned aYear)
{
    CHECK ((aMonth >= 1) && (aMonth <= 12));
    return IsLeapYear (aYear) ? LeapYearTab [aMonth-1] : NormYearTab [aMonth-1];
}



unsigned DaysOfMonth (const Time& aTime)
{
    return DaysOfMonth (aTime.GetMonth (), aTime.GetYear ());
}



/*****************************************************************************/
/*                              class TimeDiff                               */
/*****************************************************************************/



void TimeDiff::Load (Stream& S)
{
    S >> Diff;
}



void TimeDiff::Store (Stream& S) const
{
    S << Diff;
}



u16 TimeDiff::StreamableID () const
{
    return ID_TimeDiff;
}



Streamable* TimeDiff::Build ()
{
    return new TimeDiff (Empty);
}



/*****************************************************************************/
/*                                class Time                                 */
/*****************************************************************************/



Time::Time (tm& TM)
{
    T = mktime (&TM);
}



Time::Time (unsigned Year, unsigned Month, unsigned Day, unsigned Hour,
            unsigned Min, unsigned Sec)
{
    // Check the given parameters
    PRECONDITION (Year >= 1970 && Year <= 2030);
    PRECONDITION (Month > 0 && Month <= 12);
    PRECONDITION (Day > 0 && Day <= 31);
    PRECONDITION (Hour < 24 && Min < 60 && Sec < 60);

    // Assign the values
    struct tm TM;
    TM.tm_sec   = Sec;
    TM.tm_min   = Min;
    TM.tm_hour  = Hour;
    TM.tm_mday  = Day;
    TM.tm_mon   = Month - 1;
    TM.tm_year  = Year - 1900;

    // Do the conversion
    T = mktime (&TM);
}



void Time::Load (Stream &S)
{
    S >> T;
}



void Time::Store (Stream &S) const
{
    S << T;
}



u16 Time::StreamableID () const
{
    return ID_Time;
}



Streamable* Time::Build ()
{
    return new Time (Empty);
}



String Time::DateStr () const
{
    // Convert time to tm structure
    time_t Val = (time_t) T;
    struct tm* TM = localtime (&Val);

    // Setup for conversion
    int Month = TM->tm_mon + 1;
    int Year  = TM->tm_year + 1900;
    int Day   = TM->tm_mday;

    // Set up string according to local date format
    switch (NLSData.Date) {

        case 0:
            return FormatStr ("%02d%c%02d%c%04d",
                              Month, NLSData.DateSep,
                              Day,   NLSData.DateSep,
                              Year);

        case 1:
            return FormatStr ("%02d%c%02d%c%04d",
                              Day,   NLSData.DateSep,
                              Month, NLSData.DateSep,
                              Year);

        case 2:
            return FormatStr ("%04d%c%02d%c%02d",
                              Year,  NLSData.DateSep,
                              Month, NLSData.DateSep,
                              Day);

        default:
            FAIL ("Unknown value for NLSData.Date");

    }

    // Never reached
    return String ();
}



String Time::TimeStr (int Seconds) const
{
    // Convert time to tm structure
    time_t Val = (time_t) T;
    struct tm* TM = localtime (&Val);

    // Set up string according to local time format
    int PM = 0;
    switch (NLSData.Time) {

        case 0:
            // am/pm
            if (TM->tm_hour >= 12) {
                TM->tm_hour -= 12;
                PM = 1;
            }
            if (Seconds) {
                // Include seconds in string
                return FormatStr ("%02d%c%02d%c%02d %s",
                                  TM->tm_hour, NLSData.TimeSep,
                                  TM->tm_min,  NLSData.TimeSep,
                                  TM->tm_sec,  PM ? "pm" : "am");
            } else {
                // No seconds
                return FormatStr ("%02d%c%02d %s",
                                  TM->tm_hour, NLSData.TimeSep,
                                  TM->tm_min,  PM ? "pm" : "am");
            }

        case 1:
            if (Seconds) {
                // Include seconds
                return FormatStr ("%02d%c%02d%c%02d",
                                  TM->tm_hour, NLSData.TimeSep,
                                  TM->tm_min,  NLSData.TimeSep,
                                  TM->tm_sec);

            } else {
                // String without seconds
                return FormatStr ("%02d%c%02d",
                                  TM->tm_hour, NLSData.TimeSep,
                                  TM->tm_min);
            }

        default:
            FAIL ("Unknown value for NLSData.Time");

    }

    // Never reached
    return String ();
}



String Time::DateTimeStr (int Seconds) const
{
    return DateStr () + ' ' + TimeStr (Seconds);
}



String Time::DateTimeStr (const String& Fmt) const
{
    char Buf [512];

    // Convert to string
    time_t Val = (time_t) T;
    size_t Result = strftime (Buf, sizeof (Buf), Fmt.GetStr (), localtime (&Val));
    CHECK (Result != 0);

    // return the result
    return String (Buf);
}



void Time::SetYear (unsigned Year)
{
    // Check the given parameter
    PRECONDITION (Year >= 1970 && Year <= 2030);

    // Convert time to tm structure
    time_t Val = (time_t) T;
    struct tm* TM = localtime (&Val);

    // Assign new value for year
    TM->tm_year = Year - 1900;

    // Convert back to time_t
    T = mktime (TM);
}



void Time::SetMonth (unsigned Month)
{
    // Check the given parameter
    PRECONDITION (Month > 0 && Month <= 12);

    // Convert time to tm structure
    time_t Val = (time_t) T;
    struct tm* TM = localtime (&Val);

    // Assign new value for month
    TM->tm_mon = Month - 1;

    // Convert back to time_t
    T = mktime (TM);
}



void Time::SetDay (unsigned Day)
{
    // Check the given parameter
    PRECONDITION (Day > 0 && Day <= 31);

    // Convert time to tm structure
    time_t Val = (time_t) T;
    struct tm* TM = localtime (&Val);

    // Assign new value for day
    TM->tm_mday = Day;

    // Convert back to time_t
    T = mktime (TM);
}



void Time::SetHour (unsigned Hour)
{
    // Check the given parameter
    PRECONDITION (Hour < 24);

    // Convert time to tm structure
    time_t Val = (time_t) T;
    struct tm* TM = localtime (&Val);

    // Assign new value for hour
    TM->tm_hour = Hour;

    // Convert back to time_t
    T = mktime (TM);
}



void Time::SetMin (unsigned Min)
{
    // Check the given parameter
    PRECONDITION (Min < 60);

    // Convert time to tm structure
    time_t Val = (time_t) T;
    struct tm* TM = localtime (&Val);

    // Assign new value for minute
    TM->tm_min = Min;

    // Convert back to time_t
    T = mktime (TM);
}



void Time::SetSec (unsigned Sec)
{
    // Check the given parameter
    PRECONDITION (Sec < 60);

    // Convert time to tm structure
    time_t Val = (time_t) T;
    struct tm* TM = localtime (&Val);

    // Assign new value for second
    TM->tm_sec = Sec;

    // Convert back to time_t
    T = mktime (TM);
}



void Time::SetTime (unsigned Hour, unsigned Min, unsigned Sec)
{
    // Check the given parameters
    PRECONDITION (Hour < 24 && Min < 60 && Sec < 60);

    // Convert time to tm structure
    time_t Val = (time_t) T;
    struct tm* TM = localtime (&Val);

    // Assign new values
    TM->tm_hour = Hour;
    TM->tm_min  = Min;
    TM->tm_sec  = Sec;

    // Convert back to time_t
    T = mktime (TM);
}



void Time::SetTime (unsigned SecondOfDay)
{
    // Check the given parameters
    PRECONDITION (SecondOfDay <= 23*3600 + 59*60 + 59);

    // Convert time to tm structure
    time_t Val = (time_t) T;
    struct tm* TM = localtime (&Val);

    // Assign new values
    TM->tm_sec  = SecondOfDay % 60;
    SecondOfDay /= 60;
    TM->tm_min  = SecondOfDay % 60;
    SecondOfDay /= 60;
    TM->tm_hour = SecondOfDay;

    // Convert back to time_t
    T = mktime (TM);
}



void Time::SetDate (unsigned Year, unsigned Month, unsigned Day)
{
    // Check the given parameters
    PRECONDITION (Year >= 1970 && Year <= 2030);
    PRECONDITION (Month >= 1 && Month <= 12);
    PRECONDITION (Day >= 1 && Day <= 31);

    // Convert time to tm structure
    time_t Val = (time_t) T;
    struct tm* TM = localtime (&Val);

    // Assign new values
    TM->tm_year = Year - 1900;
    TM->tm_mon  = Month - 1;
    TM->tm_mday = Day;

    // Convert back to time_t
    T = mktime (TM);
}



unsigned Time::GetYear () const
{
    // Convert time to tm structure
    time_t Val = (time_t) T;
    struct tm* TM = localtime (&Val);

    // Return year
    return TM->tm_year + 1900;
}



unsigned Time::GetMonth () const
{
    // Convert time to tm structure
    time_t Val = (time_t) T;
    struct tm* TM = localtime (&Val);

    // Return month
    return TM->tm_mon + 1;
}



unsigned Time::GetDay () const
{
    // Convert time to tm structure
    time_t Val = (time_t) T;
    struct tm* TM = localtime (&Val);

    // Return day
    return TM->tm_mday;
}



unsigned Time::GetHour () const
{
    // Convert time to tm structure
    time_t Val = (time_t) T;
    struct tm* TM = localtime (&Val);

    // Return hour
    return TM->tm_hour;
}



unsigned Time::GetMin () const
{
    // Convert time to tm structure
    time_t Val = (time_t) T;
    struct tm* TM = localtime (&Val);

    // Return minute
    return TM->tm_min;
}



unsigned Time::GetSec () const
{
    // Convert time to tm structure
    time_t Val = (time_t) T;
    struct tm* TM = localtime (&Val);

    // Return second
    return TM->tm_sec;
}



void Time::GetTime (unsigned& Hour, unsigned& Min, unsigned& Sec) const
{
    // Convert time to tm structure
    time_t Val = (time_t) T;
    struct tm* TM = localtime (&Val);

    // Set return values
    Hour = TM->tm_hour;
    Min  = TM->tm_min;
    Sec  = TM->tm_sec;
}



u32 Time::GetTime () const
{
    // Convert time to tm structure
    time_t Val = (time_t) T;
    struct tm* TM = localtime (&Val);

    // Calculate and return time in seconds
    return u32 (TM->tm_hour) * 3600 + u32 (TM->tm_min) * 60 + TM->tm_sec;
}



void Time::GetDate (unsigned& Year, unsigned& Month, unsigned& Day) const
{
    // Convert time to tm structure
    time_t Val = (time_t) T;
    struct tm* TM = localtime (&Val);

    // Set return values
    Year  = TM->tm_year + 1900;
    Month = TM->tm_mon + 1;
    Day   = TM->tm_mday;
}



void Time::GetDateTime (unsigned& Year, unsigned& Month, unsigned& Day,
                        unsigned& WeekDay,
                        unsigned& Hour, unsigned& Min, unsigned& Sec) const
{
    // Convert time to tm structure
    time_t Val = (time_t) T;
    struct tm* TM = localtime (&Val);

    // Set return values
    Year        = TM->tm_year + 1900;
    Month       = TM->tm_mon + 1;
    Day         = TM->tm_mday;
    WeekDay     = TM->tm_wday;
    Hour        = TM->tm_hour;
    Min         = TM->tm_min;
    Sec         = TM->tm_sec;
}



WeekDay Time::GetDayOfWeek () const
{
    // Convert time to tm structure
    time_t Val = (time_t) T;
    struct tm* TM = localtime (&Val);

    // return value
    return WeekDay (TM->tm_wday);
}



