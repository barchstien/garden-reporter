#pragma once

#include "pin.h"

// Set Driver phase, WARNING RISK of inverse polarity !!
#define BALL_VALVE_POLARITY 1 //<--- TODO check with multimeter
#define BALL_VALVE_ON 1
#define BALL_VALVE_OFF 0

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
* WARNING PHASE is set once and for all to avoid reverse polarity !!
*/
struct ball_valve_t
{
  /**
   * @param flow_cnt_ptr pointer to flow pulse counter, to check that water flows or not
  */
  void init()
  {
    pinMode(VALVE_ENABLE, OUTPUT);
    pinMode(VALVE_ENABLE, BALL_VALVE_OFF);
    pinMode(VALVE_PHASE, OUTPUT);
    pinMode(VALVE_PHASE, BALL_VALVE_POLARITY);
  }

  void water_on() 
  {
    digitalWrite(VALVE_ENABLE, BALL_VALVE_ON);
  }

  bool water_off()
  {
    digitalWrite(VALVE_ENABLE, BALL_VALVE_OFF);
  }

  // keep ?
  bool is_on() const
  {
    return is_on_;
  }

private:
  bool is_on_{false};
};