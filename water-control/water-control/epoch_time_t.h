#pragma once

/**
 * Local clock derived from millis()
 * Does arythmetic that handle uint32_t counter wrap (every 50 days)
 * @warning always assume that all local_clock_t are not older than wrap / 2
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

  bool has_wrapped_since(const local_clock_t previous) const
  {
    // if previous hard (without wrap logic) is bigger
    // ... it means this has been taken after WRAP
    return value_ < previous.value_;
  }

  static local_clock_t now();

  static int32_t days(unsigned int d)
  {
    return (int32_t)(d * 24 * 60 * 60 * 1000);
  }
  static int32_t hours(unsigned int h)
  {
    return (int32_t)(h * 60 * 60 * 1000);
  }
  static int32_t minutes(unsigned int m)
  {
    return (int32_t)(m * 60 * 1000);
  }
  static int32_t seconds(unsigned int s)
  {
    return (int32_t)(s * 1000);
  }

  int32_t operator-(const local_clock_t &rhe) const;

  local_clock_t& operator+=(local_clock_t const &other)
  {
    value_ += other.value_;
    value_ = value_ % millis_wrap;
    return *this;
  }

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

  /** value at which arduino millis() wraps */
  static const int64_t millis_wrap = 2^32;

  local_clock_t(int64_t n) : // private ??
    value_(n)
  {}
};

/** Seconds since epoch */
typedef int64_t epoch_time_t;

/**
 * Keep a counter of second since epoch
 * Binds an epoch time to the local clock
 * Those 2 values are regularly updated because :
 *  - arduino drifts, server is NTP locked
 *  - arduino local clock wraps every 49 days
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
   */
  void set_now(epoch_time_t t);

  uint32_t uptime_sec();

private:

  /** 
   * Master millisec counter since 01/01/1970 
   * Using millisec instead of sec to avoid drift due to rounding
   * between local msec clock and master clock counter
   */
  int64_t msec_since_epoch_;

  /** 
   * Local clock timestamp that correspond ti sec_since_epoch_
   */
  local_clock_t local_timestamp_;

  /** 
   * Count millis() wraps, assuming epoch_time_sync_t::now() 
   * is called more than twice every 49 days 
   */
  uint32_t wrap_cnt_ = 0;
};