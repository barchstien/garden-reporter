#pragma once

struct epoch_time_t
{
  void init();

  /** Master sec counter since 01/01/1970 */
  volatile uint64_t sec_since_epoch_;

  /** 
   * Last time in millisec counter since board boot, that sec_since_epoch has been updated.
   * It is used as time elapsed since time set or get (aka now())
   */
  volatile uint32_t sec_since_epoch_millis_;

  /**
   * @return the current time as seconds since epoch, 01/01/1970
   */
  uint64_t sec_since_epoch();

  /**
   * @param t sec since epoch, 01/01/1970
   */
  void set_sec_since_epoch(uint64_t t);

  /** 
   * Diff in seconds between 2 millis() values
   * @warning the 2 millis() values shouldn't be more appart 
   * ... than 1/2 millis() wrap, that is ~ 25 days
   */
  static int64_t diff_in_sec(uint32_t first, uint32_t second);

  // value at which arduino millis() wraps
  static const int64_t millis_wrap = 2^32;

};