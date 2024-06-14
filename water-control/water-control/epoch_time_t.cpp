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
  sec_since_epoch_ = 0;
  local_timestamp_ = local_clock_t::now();
  Serial.print("init epoch_time: ");
  Serial.print(sec_since_epoch_);
  Serial.print(" @ millis(): ");
  Serial.println(local_timestamp_);
}

// Been tryong to add uptime by counting wraps....
// ... but screwed up timing....
epoch_time_t epoch_time_sync_t::now()
{
  // calculate time passed since last sync from server or mast call to now()
  local_clock_t now = local_clock_t::now();
  int32_t passed_msec = now - local_timestamp_;
  // increment both epoch time and local timestamp
  sec_since_epoch_ += (passed_msec + 500) / 1000;
  local_timestamp_ = now;

  return sec_since_epoch_;
  
  //epoch_time_t s = sec_since_epoch_ + (epoch_time_t)(passed_msec/1000ULL);
  // update local time ref if older than local_clock_sync_max_age sec
  //if (passed_msec >= local_clock_t::seconds(local_clock_sync_max_age_sec))
  //{
  //  sec_since_epoch_ += local_clock_sync_max_age_sec;
  //  // yes, it may overflow, just like millis()
  //  sec_since_epoch_in_local_clock_ += local_clock_t::seconds(local_clock_sync_max_age_sec);
  //}
  
  // update local reference
  //sec_since_epoch_  += (epoch_time_t)(passed_msec/1000);
  //local_clock_t last_local = sec_since_epoch_in_local_clock_;
  //sec_since_epoch_in_local_clock_ += passed_msec;
  //if (last_local > sec_since_epoch_in_local_clock_)
  //{
  //  Serial.println(" -- WRAP --");
  //  wrap_cnt_ ++;
  //}
  //return sec_since_epoch_;
}

void epoch_time_sync_t::set_now(epoch_time_t t)
{
  local_timestamp_ = local_clock_t::now();
  sec_since_epoch_ = t;
}

//int32_t epoch_time_sync_t::uptime_sec()
//{
//  return wrap_cnt_ * (int64_t)2^32 / (int64_t)1000 + local_clock_t::now() / 1000;
//}

