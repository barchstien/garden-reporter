#pragma once

#include "pin.h"

// Lithium battery caracs
// just for ref
#define BATT_V_MAX 4.2
// At this level, system can shutdown anytime
#define BATT_V_EMPTY 3.2
// This is about 1/3 of the time left
#define BATT_V_SAFE 3.75
// Smoother read curve
#define NUM_OF_VALUE_TO_AVG 5
// ADC read to Volt
#define V_PER_QUANTUM 0.00800

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
    float sum = 0;
    for (int i=0; i<NUM_OF_VALUE_TO_AVG; i++)
    {
      sum += (analogRead(BATT_READ) * V_PER_QUANTUM);
      delay(50);
    }
    return sum / float(NUM_OF_VALUE_TO_AVG);
  }

  /** @return true if enoug battery */
  bool can_use_water()
  {
    return read_volt() >= BATT_V_SAFE;
  }

};