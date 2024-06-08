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
  uint64_t passed_sec = sec_between_millis(sec_since_epoch_millis_, millis());
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

uint64_t epoch_time_t::sec_between_millis(uint32_t first, uint32_t second)
{
  if (first > second)
  {
    // overflow happened
    return ((2^32ULL - (uint64_t)first) + (uint64_t)second) / (uint64_t)1000;
  }
  else
  {
    return ((uint64_t)second - (uint64_t)first) / (uint64_t)1000;
  }
};
