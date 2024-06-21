#pragma once

#include "pin.h"

// Lithium battery caracs
// just for ref
#define BATT_V_MAX 4.2
// Should be safe to open water above that voltage
#define BATT_V_MIN 3.2

/**
 * Measure voltage of lithium battery, 
 * not voltage provided to ardui-box regulator
*/
struct battery_t
{
  float read_volt()
  {
    // Calibrate with multimeter vs read values
    // Measured V | ADC value
    //       4.13 |  520
    //       4.18 |  525
    //       4.06 |  516
    return analogRead(BATT_READ) * 0.00800;
  }

  /** @return true if enoug battery */
  bool can_use_water()
  {
    return read_volt() >= BATT_V_MIN;
  }

};