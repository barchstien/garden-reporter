#include <Arduino.h> 

#include "epoch_time_t.h"

//---- local_clock_t

local_clock_t local_clock_t::now()
{
  return local_clock_t((int64_t)millis());
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
  msec_since_epoch_ = 0;
  local_timestamp_ = local_clock_t::now();
  Serial.print("init epoch_time: ");
  Serial.print(msec_since_epoch_);
  Serial.print(" @ millis(): ");
  Serial.println(local_timestamp_);
}

epoch_time_t epoch_time_sync_t::now()
{
  // calculate time passed since last sync from server or mast call to now()
  local_clock_t now = local_clock_t::now();
  int32_t passed_msec = now - local_timestamp_;
  // detect millis() wrap
  if (now.has_wrapped_since(local_timestamp_))
  {
    wrap_cnt_ ++;
    Serial.println("---- WRAP detected");
  }
  // increment both epoch time and local timestamp
  msec_since_epoch_ += passed_msec;
  local_timestamp_ = now;
  // round to seconds
  return (msec_since_epoch_ + 500) / 1000;
}

void epoch_time_sync_t::set_now(epoch_time_t t)
{
  // save before update to compute drift
  local_clock_t previous_ts = local_timestamp_;
  int64_t previous_msec_since_epoch = msec_since_epoch_;
  // update master counter and timestamp
  local_timestamp_ = local_clock_t::now();
  msec_since_epoch_ = t * 1000;
  // calculate drift
  int64_t drift = (local_timestamp_ - previous_ts) - (msec_since_epoch_ - previous_msec_since_epoch);
  Serial.print("epoch_time_sync_t drift msec: ");
  Serial.println(drift);
}

uint32_t epoch_time_sync_t::uptime_sec()
{
  return wrap_cnt_ * ((int64_t)2^32 / (int64_t)1000) + (local_clock_t::now() / 1000);
}

