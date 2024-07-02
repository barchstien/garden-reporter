#pragma once

#include "RTClib.h"


/**
 * Local clock derived from millis()
 * @warning Logic is in milliseconds !
 * @warning DRIFTS a lot ! Found 10% to 30%
 * Does arythmetic that handle uint32_t counter wrap (every 50 days)
*/
struct local_clock_t
{
  local_clock_t() :
    value_(-1)
  {}

  bool is_valid() const
  {
    return value_ != -1;
  }

  static local_clock_t now();

  // Const durations
  static int32_t seconds(unsigned int s)
  {
    return (int32_t)(s * 1000);
  }

  int32_t operator-(const local_clock_t &rhe) const;

  local_clock_t& operator+=(local_clock_t const &other);

  bool operator<(local_clock_t const &rhe) const;
  bool operator>(local_clock_t const &rhe) const;
  bool operator==(local_clock_t const &rhe) const;
  
  operator int() const
  {
    return value_;
  }

private:
  /** As returned by millis() */
  int64_t value_;

  /** 
   * Count millis() wraps, assuming epoch_time_sync_t::now() 
   * is called more than twice every 49 days 
   */
  static int64_t wrap_cnt_;

  /** To detect millis() wraps */
  static int64_t last_millis_value_;
};


/** Seconds since epoch */
typedef int64_t epoch_time_t;


/**
 * Keep a counter of second since epoch
 * @warning Logic is in seconds !
 * Use RTC to be steady, get epoch time from server
 * Adjust RTC if error > 30 sec
*/
struct epoch_time_sync_t
{
  void init();

  /**
   * @return the current time as seconds since epoch, 01/01/1970
   */
  epoch_time_t now();

  /**
   * @param t msec since epoch, 01/01/1970
   * Called whit time received from server
   */
  void set_now(epoch_time_t t);

  uint32_t uptime_sec();

private:

  static const int MAX_SERVER_RTC_OFFSET_SEC = 30;

  RTC_DS3231 rtc_;

  epoch_time_t start_time_;
};