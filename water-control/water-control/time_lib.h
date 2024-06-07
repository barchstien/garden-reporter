/**
 * Big time based on TimeLib.h .cpp : https://github.com/PaulStoffregen/Time
 * Changes :
 *  - Fix compiler warnings (many hack didn't make sense, lots of un-used func)
 *  - Remove sync ptr and interval. It took space and I am not interrested in auto sync.
 *    I want to sync time when I know I have connectivity
 *  - Fix some algo, to make it more sober (use less RAM, less CPU)
 *  - Remove all hours(t), minute(t), etc. They rely on a cache that also takes space.
 *    And in the end, it just hides call to breakTime()
 *  - Remove all hours(), minute(), etc. They give inconsistent results, as pointed out in TimeLib doc.
      But rather than defining the above removed function (hours(t), etc), one should use breaktime()
 *  - Remove 12 hours format, useless especially when not displaying time
 *  - Remove many (MANY !) un-used functions. Clarity is paramount !
 *
 * TODO : get time from http server, that already solve local
 * 
 * Note: Most functions are not used, only set_time() and now()
 */

#pragma once

#define SUMMER_TIME_UTC_OFFSET 2
#define WINTER_TIME_UTC_OFFSET 1

typedef enum struct time_status_t
{
  not_set, need_sync, ok
};

struct time_element_t
{
  uint8_t Second;
  uint8_t Minute;
  uint8_t Hour;
  uint8_t Wday;   // day of week, sunday is day 1
  uint8_t Day;
  uint8_t Month;
  uint8_t Year;   // offset from 1970;

  arduino::String to_string();
};

// convenience macros to convert to and from tm years 
#define tmYearToCalendar(Y) ((Y) + 1970)  // full four digit year 
#define CalendarYrToTm(Y)   ((Y) - 1970)

// Useful Constants
#define SECS_PER_MIN  ((time_t)(60UL))
#define SECS_PER_HOUR ((time_t)(3600UL))
#define SECS_PER_DAY  ((time_t)(SECS_PER_HOUR * 24UL))
#define DAYS_PER_WEEK ((time_t)(7UL))
#define SECS_PER_WEEK ((time_t)(SECS_PER_DAY * DAYS_PER_WEEK))
#define SECS_PER_YEAR ((time_t)(SECS_PER_DAY * 365UL)) // TODO: ought to handle leap years
#define SECS_YR_2000  ((time_t)(946684800UL)) // the time at the start of y2k


/**
 * @return the current time as seconds since Jan 1 1970 
 */
time_t now();

/**
 * @return the current time as structured elements
 */
time_element_t now_element();

/**
 * @param t sec since 01/01/1970
 */
void set_time(time_t t);

/**
 * @param t sec
 */
void adjustTime(long adjustment);

/**
 * @param t time in sec since 01/01/1970
 * @param tm t broke into year/month/etc components
 */
time_element_t break_to_element(const time_t sec_since_1970);

/**
 * @param tm time structured into year/month/etc components
 * @return t time in sec since 01/01/1970
 */
time_t sec_since_1970_from(const time_element_t &t);

