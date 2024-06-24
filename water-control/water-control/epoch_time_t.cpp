#include <Arduino.h> 

#include "epoch_time_t.h"

//---- local_clock_t

int64_t local_clock_t::wrap_cnt_ = 0;
int64_t local_clock_t::last_millis_value_;

// Tested with 0x1ffff and len 17 --> wraps every 130 sec
#define WRAP_MASK 0xffffffff
#define WRAP_BIT_LEN 32

local_clock_t local_clock_t::now()
{
  local_clock_t l;
  // wrap mask here to be able to test wrap logic with less bit
  l.value_ = (int64_t)millis() & WRAP_MASK;

  if (l.value_ < last_millis_value_)
  {
    wrap_cnt_ += 1;
    Serial.println("---- WRAP detected");
  }
  last_millis_value_ = l.value_;
  l.value_ = (wrap_cnt_ << WRAP_BIT_LEN) | l.value_;

  return l;
}

uint32_t local_clock_t::uptime_sec()
{
  return now().value_ / 1000;
}

int32_t local_clock_t::operator-(const local_clock_t &rhe) const
{
  return this->value_ - rhe.value_;
}

 local_clock_t& local_clock_t::operator+=(local_clock_t const &other)
{
  value_ += other.value_;
  return *this;
}

bool local_clock_t::operator<(local_clock_t const &rhe) const
{
  return *this < rhe;
}
bool local_clock_t::operator>(local_clock_t const &rhe) const
{
  return *this > rhe;
}
bool local_clock_t::operator==(local_clock_t const &rhe) const
{
  return *this  == rhe;
}


//---- epoch_time_sync_t

void epoch_time_sync_t::init()
{
  rtc_.begin();
}

epoch_time_t epoch_time_sync_t::now()
{
  return rtc_.now().unixtime();
}

void epoch_time_sync_t::set_now(epoch_time_t t)
{
  epoch_time_t rtc_now = now();
  // calculate offset
  int offset = abs(rtc_now - t);
  if (offset > MAX_SERVER_RTC_OFFSET_SEC)
  {
    Serial.print("RTC to Server sec offset over threshold: ");
    Serial.println(offset);
    Serial.println("  |--> Adjust RTC !!");
    rtc_.adjust(DateTime(t));
  }
}


