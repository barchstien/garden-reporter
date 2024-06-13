#include <Arduino.h> 

#include "epoch_time_t.h"

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
  return (*this - rhe) < 0;
}
bool local_clock_t::operator>(local_clock_t const &rhe) const
{
  return (*this - rhe) > 0;
}
bool local_clock_t::operator==(local_clock_t const &rhe) const
{
  return (*this - rhe) == 0;
}

//-------------------------


void epoch_time_sync_t::init()
{
  sec_since_epoch_ = 0;
  sec_since_epoch_in_local_clock_ = local_clock_t::now();
  Serial.print("init epoch_time: ");
  Serial.print(sec_since_epoch_);
  Serial.print(" @ millis(): ");
  Serial.println(sec_since_epoch_in_local_clock_);
}

epoch_time_t epoch_time_sync_t::sec_since_epoch()
{
  // calculate number of seconds passed since last sync from server
  int32_t passed_msec = local_clock_t::now() - sec_since_epoch_in_local_clock_;
  uint64_t s = sec_since_epoch_ + (uint64_t)(passed_msec/1000ULL);
  // update local time ref if older than 1000 sec
  if (passed_msec >= local_clock_t::seconds(1000))
  {
    sec_since_epoch_ += 1000;
    // yes, it may overflow, just like millis()
    sec_since_epoch_in_local_clock_ += local_clock_t::seconds(1000);
  }
  return s;
}

void epoch_time_sync_t::set_sec_since_epoch(epoch_time_t t)
{
  sec_since_epoch_ = t;
  sec_since_epoch_in_local_clock_ = millis();
} 

