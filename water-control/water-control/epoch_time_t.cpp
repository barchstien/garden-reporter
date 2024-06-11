#include <Arduino.h> 

#include "epoch_time_t.h"

void epoch_time_t::init()
{
  sec_since_epoch_ = 0;
  sec_since_epoch_millis_ = millis();
  Serial.print("init epoch_time: ");
  Serial.print(sec_since_epoch_);
  Serial.print(" @ millis(): ");
  Serial.println(sec_since_epoch_millis_);
}

uint64_t epoch_time_t::sec_since_epoch()
{
  // calculate number of seconds passed since last call to now()
  int64_t passed_sec = diff_in_sec(millis(), sec_since_epoch_millis_);
  // update local time ref if older than 1000 sec
  if (passed_sec >= 1000)
  {
    sec_since_epoch_ += passed_sec;
    // yes, it may overflow, just like millis()
    sec_since_epoch_millis_ += passed_sec * (uint64_t)1000;
  }
  return (uint64_t)sec_since_epoch_ + passed_sec;
}

void epoch_time_t::set_sec_since_epoch(uint64_t t)
{
  sec_since_epoch_ = t;
  sec_since_epoch_millis_ = millis();
} 

int64_t epoch_time_t::diff_in_sec(uint32_t first, uint32_t second)
{
  int64_t diff = (int64_t)first - (int64_t)second;
  if (diff < -1 * millis_wrap / 2)
  {
    diff += millis_wrap;
  }
  else if (diff > millis_wrap / 2)
  {
    diff -= millis_wrap;
  }
  return diff/1000;
};
