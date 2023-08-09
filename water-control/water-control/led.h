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
  }

  void off()
  {
    digitalWrite(LED, 0);
  }

  // TODO blink using millisec
};