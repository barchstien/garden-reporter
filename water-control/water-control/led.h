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

private:
  volatile bool is_on_;
};