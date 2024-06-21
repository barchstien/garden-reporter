#include <Arduino.h> 

#include "epoch_time_t.h"

//---- local_clock_t

//RTCZero local_clock_t::rtc_zero_;

local_clock_t local_clock_t::now()
{
  local_clock_t l;
  l.value_ = (int64_t)millis();
  //l.value_ = (int64_t)(rtc_zero_.getEpoch()) * 1000;
  return l;
}

void local_clock_t::init()
{
  //local_clock_t::rtc_zero_.begin();
  //local_clock_t::rtc_zero_.setEpoch(0);
}

int32_t local_clock_t::operator-(const local_clock_t &rhe) const
{
  int64_t diff = (int64_t)this->value_ - (int64_t)rhe.value_;
  if (diff < -1 * millis_wrap / 2)
  {
    diff += millis_wrap;
  }
  else if (diff > millis_wrap / 2)
  {
    diff -= millis_wrap;
  }
  return diff;
}

bool local_clock_t::operator<(local_clock_t const &rhe) const
{
  int32_t diff = *this - rhe;
  //Serial.print("       ++++");
  //Serial.println(diff);
  return (*this - rhe) < 0;
}
bool local_clock_t::operator>(local_clock_t const &rhe) const
{
  int32_t diff = *this - rhe;
  //Serial.print("       ++++");
  //Serial.println(diff);
  return (*this - rhe) > 0;
}
bool local_clock_t::operator==(local_clock_t const &rhe) const
{
  return (*this - rhe) == 0;
}


//---- epoch_time_sync_t

void epoch_time_sync_t::init()
{
  //rtc_zero_.begin();
  local_clock_t::init();
  epoch_timestamp_ = 0;
  local_timestamp_ = local_clock_t::now();
  Serial.print("init epoch_time: ");
  Serial.print(epoch_timestamp_);
  Serial.print(" @ millis(): ");
  Serial.println(local_timestamp_);
}

epoch_time_t epoch_time_sync_t::now()
{
  // calculate time passed since last sync from server or mast call to now()
  local_clock_t now = local_clock_t::now();
  int32_t passed_msec = now - local_timestamp_;
  // detect millis() wrap
  //if (now.has_wrapped_since(local_timestamp_))
  //{
  //  wrap_cnt_ ++;
  //  Serial.println("---- WRAP detected");
  //}
  // increment both epoch time and local timestamp
  epoch_timestamp_ += passed_msec;
  local_timestamp_ = now;
  // round to seconds
  return (epoch_timestamp_ + 500) / 1000;
}

void epoch_time_sync_t::set_now(epoch_time_t t, local_clock_t l)
{
  // save before update to compute drift
  local_clock_t previous_ts = local_timestamp_;
  epoch_time_t previous_epoch_timestamp = epoch_timestamp_;
  //time_t previous_rtc_timestamp = rtc_timestamp_;
  // update master counter and timestamp
  local_timestamp_ = l;//local_clock_t::now();
  epoch_timestamp_ = t * 1000;
  //rtc_timestamp_ = rtc_zero_.getEpoch();
  //rtc_zero_.setEpoch(t);
  // calculate drift
  int64_t drift = (local_timestamp_ - previous_ts) - (epoch_timestamp_ - previous_epoch_timestamp);
  if (never_been_set_)
  {
    never_been_set_ = false;
    //rtc_zero_.setEpoch(t);
  }
  else
  {
    drift_sum_ += drift;
  }
  Serial.print(t);
  Serial.print(" - drift msec: ");
  Serial.print(drift);
  Serial.print(" SUM: ");
  Serial.println(drift_sum_);
  // drift from RTC
  //int64_t drift_rtc = (local_timestamp_/1000 - previous_ts/1000) - (rtc_timestamp_ - previous_rtc_timestamp);
  //Serial.print(t);
  //Serial.print(" -   rtc msec: ");
  //Serial.println(drift_rtc);
}

uint32_t epoch_time_sync_t::uptime_sec()
{
  return wrap_cnt_ * ((int64_t)2^32 / (int64_t)1000) + (local_clock_t::now() / 1000);
}

