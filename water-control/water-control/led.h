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
    digitalWrite(LED, 1);
    is_on_ = true;
  }

  void off()
  {
    digitalWrite(LED, 0);
    is_on_ = false;
  }

  // TODO blink using millisec

  bool is_on()
  {
    return is_on_;
  }

  void blink(uint32_t duration_sec, uint32_t period_msec, uint8_t dutty_cycle=50)
  {
    uint32_t start = millis();
    while(epoch_time_t::sec_between_millis(start, millis()) <= duration_sec)
    {
      on();
      delay(period_msec * dutty_cycle / 100);
      off();
      delay(period_msec * (100 - dutty_cycle) / 100);
    }
  }

private:
  volatile bool is_on_;
};