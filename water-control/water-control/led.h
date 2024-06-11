#pragma once

#include "pin.h"

struct led_t
{
  void init()
  {
    pinMode(LED, OUTPUT);
    on();
  }

  void on()
  {
    analogWrite(LED, 255);
    is_on_ = true;
  }

  void off()
  {
    analogWrite(LED, 0);
    is_on_ = false;
  }

  bool is_on()
  {
    return is_on_;
  }

  void blink(uint32_t duration_sec, uint32_t period_msec, uint8_t dutty_cycle=50)
  {
    uint32_t start = millis();
    while(epoch_time_t::diff_in_sec(millis(), start) <= duration_sec)
    {
      on();
      delay(period_msec * dutty_cycle / 100);
      off();
      delay(period_msec * (100 - dutty_cycle) / 100);
    }
  }

  void fade(uint32_t duration_sec, uint32_t period_msec)
  {
    uint32_t start = millis();
    while(epoch_time_t::diff_in_sec(millis(), start) <= duration_sec)
    {
      for (int i=0; i<25; i++)
      {
        analogWrite(LED, i*255/25);
        delay((period_msec)/50);
      }
      for (int i=0; i<25; i++)
      {
        analogWrite(LED, (25-i)*255/25);
        delay((period_msec)/50);
      }
    }
  }

private:
  volatile bool is_on_;
};