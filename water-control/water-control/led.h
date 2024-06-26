#pragma once

#include "pin.h"
#include "epoch_time_t.h"

struct led_t
{
  led_t(epoch_time_sync_t* epoch_time_sync)
    : epoch_time_sync_(epoch_time_sync)
  {}

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
    epoch_time_t start = epoch_time_sync_->now();
    while(epoch_time_sync_->now() - start <= duration_sec)
    {
      on();
      delay(period_msec * dutty_cycle / 100);
      off();
      delay(period_msec * (100 - dutty_cycle) / 100);
    }
  }

  void fade(uint32_t duration_sec, uint32_t period_msec)
  {
    epoch_time_t start = epoch_time_sync_->now();
    while(epoch_time_sync_->now() - start <= duration_sec)
    {
      for (int i=1; i<=25; i++)
      {
        analogWrite(LED, i*255/25);
        delay((period_msec)/50);
      }
      for (int i=1; i<=25; i++)
      {
        analogWrite(LED, (25-i)*255/25);
        delay((period_msec)/50);
      }
    }
  }

private:
  volatile bool is_on_;
  epoch_time_sync_t* epoch_time_sync_;
};