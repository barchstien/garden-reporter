#pragma once

#include "pin.h"

struct button_t
{
  void init()
  {
    // buttons
    pinMode(BIT0, INPUT_PULLUP);
    pinMode(BIT1, INPUT_PULLUP);
    pinMode(BIT2, INPUT_PULLUP);
    pinMode(ALLOW_WATER, INPUT_PULLUP);
  }

  // TODO read
};