#pragma once

#include "pin.h"

#define V_DIVIDER 0.454
#define V_MAX     3.3
#define ADC_MAX   1023

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
    // ADC * V_MAX / ADC_MAX * V_DIVIDER;
    // |--> don't know V divider... Above const are bullcrap I believe
    // Instead calibrate with multimeter
    //  4.13V |  520
    //  4.18V |  525
    // debug
    Serial.print("ADC: ");
    Serial.print(analogRead(BATT_READ));
    Serial.print(" ");
    return analogRead(BATT_READ) * 0.00800;
  }

  /** @return true if enoug battery */
  bool can_use_water()
  {
    return read_volt() >= BATT_V_MIN;
  }

};