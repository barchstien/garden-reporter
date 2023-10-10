#pragma once

#include "pin.h"

#define V_DIVIDER 0.454
#define V_MAX     3.3
#define ADC_MAX   1023

// just a 100% ref
#define BATT_V_MAX 4.2
// Should be safe to open water above that voltage
#define BATT_V_MIN 3.2

struct battery_t
{
  uint32_t read_volt()
  {
    return analogRead(BATT_READ);// * V_MAX / ADC_MAX * V_DIVIDER;
  }

  /** @return true if enoug battery */
  bool can_use_water()
  {
    return read_volt() >= BATT_V_MIN;
  }

};