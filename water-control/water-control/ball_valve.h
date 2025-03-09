#pragma once

#include "pin.h"

// Set Driver phase, WARNING RISK of inverse polarity !!
// Looking at arduibox, left is GND, right is +15V
// Note: Been seeing some strange behavior with polarity/phase
//       as if polarity was stuck regardless of phase
//       But then it was opposite than what was measured without load
//       |--> could it be that proper measure need load ?
#define BALL_VALVE_POLARITY LOW

#define BALL_VALVE_ON HIGH
#define BALL_VALVE_OFF LOW

/**
* Controle DRV8874 Single Brushed DC Motor Driver Carrier
* Which is used as a driver a ball valve
* mini board : https://www.pololu.com/product/4035
* It uses 2 PIN to control
* Enable | Phase | OUT1 | OUT2 |
* ------------------------------
*    0   |   X   |   L  |  L   |
* ------------------------------
*    1   |   1   |   H  |  L   |
* ------------------------------
*    1   |   0   |   L  |  H   |
* ------------------------------
* Ball valves is closed by default
* It opens when applying DC 9-24V
* and remains open until DC is removed
* @WARNING WRONG PHASE means REVERSE POLARITY !!
*/
struct ball_valve_t
{
  /**
   * @param flow_cnt_ptr pointer to flow pulse counter, to check that water flows or not
  */
  void init()
  {
    pinMode(VALVE_ENABLE, OUTPUT);
    digitalWrite(VALVE_ENABLE, BALL_VALVE_OFF);
    pinMode(VALVE_PHASE, OUTPUT);
    digitalWrite(VALVE_PHASE, BALL_VALVE_POLARITY);
  }

  void water_on() 
  {
    // phase is fixed, coz we want fixed polarity
    digitalWrite(VALVE_PHASE, BALL_VALVE_POLARITY);
    digitalWrite(VALVE_ENABLE, BALL_VALVE_ON);
    is_on_ = true;
  }

  bool water_off()
  {
    // phase is fixed, coz we want fixed polarity
    digitalWrite(VALVE_PHASE, BALL_VALVE_POLARITY);
    digitalWrite(VALVE_ENABLE, BALL_VALVE_OFF);
    is_on_ = false;
  }

  bool is_on() const
  {
    return is_on_;
  }

private:
  bool is_on_{false};
};