#pragma once

#include "pin.h"

/**
 * From forums, latched solenoid valve need 20 to 50 msec puls
 * delay() is running @150%
 * 50 msec * 1.5 = 75 msec   1st fails
 * 40 msec * 1.5 = 60 msec   1st fails
 * 30 msec * 1.5 = 45 msec   No work
 * 20 msec * 1.5 = 30 msec   No work
 *                100 msec   1st fails, many fails after
 * 
 * Looks like 1st water fails after arduino is prog with USB ?
 * After 30+ tries (with different delays) :
 *  - 75 msec looks good
 *  - doesn't fails on 1st cycle anymore
 *  - no relation with USB prog
 * --> should wires we water protected ?
 * 
 * Few days later... impossible to open the valve
 * Two suspects : 
 *  1. corrosion on wires
 *  2. got to re-try open/close and log any problem
 * 
 * Wires are now twisted together and clamped
 * Been having an infinite loopn trying to close in vain !
 * --> Increase pulse duration ! Never had that before
 * Use 1000 msec, like before, was reliable
 * No much gain to have battery wise, because system would exhasut battery in few days only anyways
 * |--> Doesn't help ! Still can't close valve
 * |--> must be due to new wire twist + clamp
 * |--> Attach wires and solder
 * 
 * |--> Order of what to do
 *   1. clean by openinig bleeder
 *   2. clean by opening solenoid
 *   3. clean by opening valve body
 *   4. buy new valve (solenoid alone abd valve + solenoid cost the same...)
 * 
 *  None if the above (1. -> 4.) made it work
 *  |---> Working thanks to :
 *  I.  pressure reductor before valve
 *  II. 1000 msec pulse
 * 
 * 11/08/2024
 * Changed solenoid
 *   800 msec OK
 *   400      OK
 *   200      OK
 *   100      OK
 * ---> now using 100 msec
*/
#define VALVE_PULSE_MSEC 100
#define VALVE_PHASE_OPEN 1
#define VALVE_PHASE_CLOSE 0

#define MAX_WATER_ON_TRY 5

/**
* Controle DRV8874 Single Brushed DC Motor Driver Carrier
* Which is used as a driver for the single coil 9V latching relay
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
*/
struct valve_t
{
  /**
   * @param WATER_ON_MIN_TRIG min number of flow trig to consider water as ON
   * @param WATER_ON_MIN_TRIG_DELAY_MSEC delay to wait for flow triggers/pulses
  */
  valve_t(int WATER_ON_MIN_TRIG, int WATER_ON_MIN_TRIG_DELAY_MSEC) :
    WATER_ON_MIN_TRIG_(WATER_ON_MIN_TRIG),
    WATER_ON_MIN_TRIG_DELAY_MSEC_(WATER_ON_MIN_TRIG_DELAY_MSEC)
  {}

  /**
   * @param flow_cnt_ptr pointer to flow pulse counter, to check that water flows or not
  */
  void init(uint32_t* flow_cnt_ptr)
  {
    flow_cnt_ptr_ = flow_cnt_ptr;
    pinMode(VALVE_ENABLE, OUTPUT);
    pinMode(VALVE_PHASE, OUTPUT);
    delay(250);
    int pulse_applied_cnt = 0;
    water_off(&pulse_applied_cnt);
  }

  /**
   * @param num_of_pulse that was applied
   * @return true if valve opened and water started flowing, else false
  */
  bool water_on(int* num_of_pulse)
  {
    // Loop until water is on or max try occured
    int32_t flow_cnt_tmp = -1;
    unsigned int cnt = 0;
    while (flow_cnt_tmp == -1 || (*flow_cnt_ptr_ <= (flow_cnt_tmp + WATER_ON_MIN_TRIG_) && cnt < MAX_WATER_ON_TRY))
    {
      water_on_();
      flow_cnt_tmp = *flow_cnt_ptr_;
      // wait a bit ot be sure water does flow
      delay(WATER_ON_MIN_TRIG_DELAY_MSEC_);
      cnt ++;
    }
    // also return number of pulses used to caller
    *num_of_pulse = cnt;
    if (*flow_cnt_ptr_ <= (flow_cnt_tmp + WATER_ON_MIN_TRIG_))
    {
      // failed to start water
      return false;
    }
    is_on_ = true;
    return true;
  }

  /**
   * @param num_of_pulse that was applied
   * @return true if valve closed and water stopped flowing, else false
  */
  bool water_off(int* num_of_pulse)
  {
    int32_t flow_cnt_tmp = -1;
    unsigned int cnt = 0;
    while (flow_cnt_tmp == -1 || (*flow_cnt_ptr_ >= (flow_cnt_tmp + WATER_ON_MIN_TRIG_) && cnt < 10))
    {
      water_off_();
      // wait for valve to close
      delay(2000);
      flow_cnt_tmp = *flow_cnt_ptr_;
      // wait a bit ot be sure water does not flow
      delay(WATER_ON_MIN_TRIG_DELAY_MSEC_);
      cnt ++;
    }
    // also return number of pulses used to caller
    *num_of_pulse = cnt;
    if (*flow_cnt_ptr_ >= (flow_cnt_tmp + WATER_ON_MIN_TRIG_))
    {
      // failed to stop water
      return false;
    }
    is_on_ = false;
    return true;
  }

  bool is_on() const
  {
    return is_on_;
  }

private:
  bool is_on_{false};

  /** Pointer to variable updated by trigger from water flow counter */
  uint32_t* flow_cnt_ptr_;

  /** Min number of pulses received to consider water to be ON */
  const uint32_t WATER_ON_MIN_TRIG_;
  /** Delay to wait for when checking if water is ON/OFF */
  const uint32_t WATER_ON_MIN_TRIG_DELAY_MSEC_;

  void water_on_()
  {
    digitalWrite(VALVE_PHASE, VALVE_PHASE_OPEN);
    digitalWrite(VALVE_ENABLE, 1);
    delay(VALVE_PULSE_MSEC);
    digitalWrite(VALVE_ENABLE, 0);
  }

  void water_off_()
  {
    digitalWrite(VALVE_PHASE, VALVE_PHASE_CLOSE);
    digitalWrite(VALVE_ENABLE, 1);
    delay(VALVE_PULSE_MSEC);
    digitalWrite(VALVE_ENABLE, 0);
  }
};