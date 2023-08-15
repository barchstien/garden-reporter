#include <Arduino.h> 

#include "time_lib.h"

#include <sstream>

/*============================================================================*/	
/* functions to convert to and from system time */
/* These are for interfacing with time services and are not normally needed in a sketch */

// leap year calculator expects year argument as years offset from 1970
#define LEAP_YEAR(Y)     ( ((1970+(Y))>0) && !((1970+(Y))%4) && ( ((1970+(Y))%100) || !((1970+(Y))%400) ) )

// API starts months from 1, this array starts from 0
static const uint8_t monthDays[] = {31,28,31,30,31,30,31,31,30,31,30,31};

arduino::String time_element_t::to_string()
{
  arduino::String s;
  // YYYY/MM/DD HH:MM:SS --> 20 (including ending \0)
  s.reserve(20);
  s += (Year + 1970);
  s += "/";
  if (Month < 10)
    s += "0";
  s += Month;
  s += "/";
  if (Day < 10)
    s += "0";
  s += Day;
  s += " ";
  if (Hour < 10)
    s += "0";
  s += Hour;
  s += ":";
  if (Minute < 10)
    s += "0";
  s += Minute;
  s += ":";
  if (Second < 10)
    s += "0";
  s += Second;
  return s;
}

time_element_t break_to_element(const time_t time_input)
{
  // break the given time_t into time components
  // this is a more compact version of the C library localtime function
  // note that year is offset from 1970 !!!

  uint8_t year;
  uint8_t month, monthLength;
  uint32_t time;
  unsigned long days;
  time_element_t tm;

  time = (uint32_t)time_input;
  tm.Second = time % 60;
  time /= 60; // now it is minutes
  tm.Minute = time % 60;
  time /= 60; // now it is hours
  tm.Hour = time % 24;
  time /= 24; // now it is days
  tm.Wday = ((time + 4) % 7) + 1;  // Sunday is day 1 
  
  year = 0;  
  days = 0;
  while((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time)
  {
    year++;
  }
  tm.Year = year; // year is offset from 1970 
  
  days -= LEAP_YEAR(year) ? 366 : 365;
  time  -= days; // now it is days in this year, starting at 0
  
  days=0;
  month=0;
  monthLength=0;
  for (month=0; month<12; month++)
  {
    if (month==1)
    {
      // february
      if (LEAP_YEAR(year))
        monthLength=29;
      else
        monthLength=28;
    }
    else
      monthLength = monthDays[month];
    
    if (time >= monthLength)
      time -= monthLength;
    else
      break;
  }
  // jan is month 1  
  tm.Month = month + 1;
  // day of month
  tm.Day = time + 1;

  return tm;
}

time_t sec_since_1970_from(const time_element_t &t)
{   
  // assemble time elements into time_t 
  // note year argument is offset from 1970 (see macros in time.h to convert to other formats)
  // previous version used full four digit year (or digits since 2000),i.e. 2009 was 2009 or 9
  
  int i;
  uint32_t seconds;

  // seconds from 1970 till 1 jan 00:00:00 of the given year
  seconds= t.Year*(SECS_PER_DAY * 365);
  for (i = 0; i < t.Year; i++)
  {
    if (LEAP_YEAR(i))
    {
      seconds += SECS_PER_DAY;   // add extra days for leap years
    }
  }
  
  // add days for this year, months start from 1
  for (i = 1; i < t.Month; i++)
  {
    if ( (i == 2) && LEAP_YEAR(t.Year))
    {
      seconds += SECS_PER_DAY * 29;
    }
    else
    {
      //monthDay array starts from 0
      seconds += SECS_PER_DAY * monthDays[i-1];
    }
  }
  seconds+= (t.Day-1) * SECS_PER_DAY;
  seconds+= t.Hour * SECS_PER_HOUR;
  seconds+= t.Minute * SECS_PER_MIN;
  seconds+= t.Second;
  return (time_t)seconds; 
}

/** Master second counter since 01/01/1970 */
static uint32_t sysTime = 0;

/** 
 * Last time in millisec counter since board boot, that sysTime has been updated.
 * It is used as time elapsed since time set or get (aka now())
 */
static uint32_t prevMillis = 0;

static time_status_t Status = time_status_t::timeNotSet;

time_t now()
{
  // calculate number of seconds passed since last call to now()
  // millis() and prevMillis are both unsigned ints thus the subtraction will always be the absolute value of the difference
  unsigned long passed_sec = (millis() - prevMillis) / 1000UL;
  if (passed_sec >= 1000)
  {
    sysTime += passed_sec;
    prevMillis += passed_sec * 1000UL;
  }
  return (time_t)sysTime;
}

time_element_t now_element()
{
  time_t t = now();
  return break_to_element(t);
}

void set_time(time_t t)
{
  sysTime = (uint32_t)t;
  Status = time_status_t::timeSet;
  prevMillis = millis();
} 

void set_time(int hr,int min,int sec,int dy, int mnth, int yr)
{
 // Year can be given as full four digit year or two digts (2010 or 10 for 2010);  
 // It is converted to years since 1970
  if( yr > 99)
      yr = yr - 1970;
  else
      yr += 30;
  time_element_t tm;
  tm.Year = yr;
  tm.Month = mnth;
  tm.Day = dy;
  tm.Hour = hr;
  tm.Minute = min;
  tm.Second = sec;
  set_time(sec_since_1970_from(tm));
}

void adjustTime(long adjustment)
{
  sysTime += adjustment;
}

// indicates if time has been set and recently synchronized
time_status_t timeStatus()
{
  now(); // required to actually update the status
  return Status;
}
