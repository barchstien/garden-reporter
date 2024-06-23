//#include <ArduinoLowPower.h>

#include "battery.h"
#include "button.h"
#include "const.h"
#include "epoch_time_t.h"
#include "led.h"
#include "http_reporter.h"
#include "valve.h"
#include "wifi.h"

uint32_t flow_cnt = 0;
uint32_t flow_cnt_last = 0;
const float flow_cnt_liter_per_edge = 0.00245;

uint32_t start_water_cnt = 0;
uint32_t start_water_cnt_last = 0;

// Wifi report period
const unsigned int sec_report_period = (1 * 60); // TODO 15 min
// local clock of last report, or invalid if previously failed
local_clock_t last_report_;
const unsigned int sec_report_period_when_watering = 60;

// Hard limit watering duration, 1.5 hour
const int64_t MAX_WATER_DURATION_SEC = 5400;

// delay between loop
const unsigned int millisec_loop_step = (100);

// Twice report period, to avoid missing first waterin
// 20 min minimum !
const epoch_time_t water_schedule_margin_sec = max(sec_report_period * 2, 20 * 60);

epoch_time_t next_water_schedule;
epoch_time_t last_water_schedule;

void flow_trig_isr()
{
  flow_cnt ++;
}

battery_t battery;
button_t button;
epoch_time_sync_t epoch_time_sync;
led_t led;
valve_t valve;
wifi_t wifi;
http_reporter_t reporter;

// Command received by server
http_reporter_t::command_t server_cmd;


void setup()
{
  // debug
  Serial.begin(115200);
  Serial.println("--");
  Serial.println("-- setup");

  pinMode(FLOW_TRIG, INPUT_PULLUP);
  attachInterrupt(
    digitalPinToInterrupt(FLOW_TRIG), flow_trig_isr, FALLING
  );
  
  epoch_time_sync.init();
  button.init();
  led.init();
  valve.init();

  // Suspecting low power sleep to make USB serial unstable !!!
  // |--> confirm later, use normal interrupt for now

  // blink
  led.blink(3, 1000, 50);
  
  // Debug
  pinMode(LED_BUILTIN, OUTPUT);

  // Avoid false positive on startup
  // (only happens when plug/unplug, not when using arduino reset button)
  button.reset_start_water_push();

  Serial.println("--");
  Serial.println("-- setup END");
}


/**
 * @warning Expects Wifi to be connected !
*/
report_status report_via_wifi()
{
  report_status rs = report_status::FAILURE;
  
  // Compute water since last successful report
  int32_t flow_cnt_tmp = flow_cnt;
  float water_liter = (float)(flow_cnt_tmp - flow_cnt_last) * flow_cnt_liter_per_edge;

  http_reporter_t::command_t cmd = reporter.report(
    water_liter,
    battery.read_volt(),
    next_water_schedule,
    last_water_schedule,
    valve.is_on(),
    local_clock_t::uptime_sec()
  );
  
  if (cmd.is_valid)
  {
    // report was succesful, reset flow cnt last
    flow_cnt_last = flow_cnt_tmp;

    //Serial.println("=== CMD FROM SERVER ===");
    epoch_time_sync.set_now(cmd.sec_since_epoch);

    if (server_cmd == cmd)
    {
      //Serial.println("--> Ignore server command, already known");
      rs = report_status::CMD_ALREADY_KOWN;
    }
    else
    {
      server_cmd = cmd;
      LOG("--> Apply server command");
      Serial.println("");
      // A config has been received from server
      // ensure next scheduled water is in the future
      next_water_schedule = server_cmd.start_time_sec_since_epoch;
      while(next_water_schedule < epoch_time_sync.now() - water_schedule_margin_sec)
      {
        next_water_schedule += server_cmd.period_day * 24 * 60 * 60;
      }
      Serial.print("    Next watering at ");
      Serial.println(next_water_schedule);
      Serial.print("    happening in sec: ");
      Serial.println(next_water_schedule - epoch_time_sync.now());
      rs = report_status::CMD_APPLIED;
    }
  }
  return rs;
}


void loop()
{
  //// debug
  //Serial.print("epoch time: ");
  //Serial.println(epoch_time_sync.now());

  //Serial.print("uptime sec: ");
  //Serial.println(epoch_time_sync.uptime_sec());

  //Serial.print("minutes: ");
  //Serial.print(button.bit_value() * 15);
  //Serial.print(" allowed: ");
  //Serial.println(button.allow_water());

  //Serial.print("Battery: ");
  //Serial.println(battery.read_volt());
  //// debug end

  // HTTP report
  // Send status
  // Get config including epoch time sync and watering schedule
  if (last_report_.is_valid() == false
    || local_clock_t::now() - last_report_ > local_clock_t::seconds(sec_report_period))
  {
    // Report via wifi
    led.on();
    bool failed = false;
    if (wifi.connect())
    {
      report_status rs = report_via_wifi();
      if (CMD_APPLIED == rs)
      {
        // received a new config, re-report to update status
        report_via_wifi();
      }
      else if (FAILURE == rs)
      {
        //
        failed = true;
      }
    }
    // Regardless of success or not, turn off Wifi
    wifi.end();
    led.off();
    if (failed == false)
    {
      last_report_ = local_clock_t::now();
    }
    else
    {
      last_report_ = local_clock_t();
    }
  }

  if (server_cmd.is_valid)
  {
    // Perfom scheduled watering
    // If water isn't allowed/enabled still do update next watering coz it has passed
    if (epoch_time_sync.now() > next_water_schedule)
    {
      led.on();
      bool wifi_is_connected = false;
      if (wifi.connect())
      {
        wifi_is_connected = true;
      }
      if (button.allow_water() && battery.can_use_water() && server_cmd.enabled)
      {
        uint32_t duration_sec = server_cmd.duration_minute * 60;
        if (duration_sec > MAX_WATER_DURATION_SEC)
        {
          duration_sec = MAX_WATER_DURATION_SEC;
        }
        epoch_time_t deadline = epoch_time_sync.now() + duration_sec;
        Serial.println("");
        LOG(" >>>> Water NOW <<<<");
        Serial.println("");
        //digitalWrite(LED_BUILTIN, HIGH);
        valve.water_on();
        
        // TODO : Use millis() instead, coz it's can't be adjusted
        //        Clock that can be adjusted could lead to years of watering
        // Report as long as it is watering
        // ... and check if config get changed
        while(epoch_time_sync.now() < deadline)
        {
          if (wifi_is_connected)
          {
            led.on();
            report_status rs = report_via_wifi();
            if (rs == report_status::CMD_APPLIED)
            {
              // Check enabled only ? 
              // What about new schedule ?
              // start/period/duration
              if (server_cmd.enabled == false)
              {
                LOG("Cancel scheduled watering, by order of server");
                Serial.println("");
                break;
              }
            }
            led.off();
          }
          if (button.allow_water() == false)
          {
            LOG("Cancel scheduled watering, by order of button");
            Serial.println("");
            break;
          }
          // fade also means delay
          led.fade(sec_report_period_when_watering, 1000);
        }
        
        //digitalWrite(LED_BUILTIN, LOW);
        valve.water_off();
        Serial.println("");
        LOG(" >>>> STOP Water <<<<");
        Serial.println("");
      }
      // re-schedule
      // ... but not if it has been re-scheduled by wifi config already
      // ... meaning next schedule is not already set in future
      if (epoch_time_sync.now() > next_water_schedule)
      {
        last_water_schedule = next_water_schedule;
        next_water_schedule += server_cmd.period_day * 24 * 60 * 60;
        LOG("Next watering at ");
        Serial.println(next_water_schedule);
        Serial.print("  happening in sec: ");
        Serial.println(next_water_schedule - epoch_time_sync.now());
      }
      // report to update status
      if (wifi_is_connected)
      {
        report_via_wifi();
      }
      // Regardless success or not, need to turn off Wifi
      wifi.end();
      led.off();
    }
  }

  if (button.start_water_was_pushed())
  {
    if (button.bit_value() == 0)
    {
      // trigger report to HTTP server on next iteration
      // makes last_report_ local clock invalid
      last_report_ = local_clock_t();
    }
    else
    {
      // check button on/off and battery
      if (button.allow_water() && battery.can_use_water())
      {
        valve.water_on();
        // multiple of 15 minutes
        uint32_t duration_sec = button.bit_value() * 15 * 60;
        LOG("Manual start second : ");
        Serial.println(duration_sec);
        for (int i=0; i<duration_sec; i++)
        {
          // fade does delay()
          led.fade(1, 1000);
          if (button.allow_water() == false)
          {
            LOG("Cancel manual start after sec:");
            Serial.println(i);
            break;
          }
        }
        valve.water_off();
        led.off();
        // trigger report to HTTP server on next iteration
        // makes last_report_ local clock invalid
        last_report_ = local_clock_t();
      }
    }
    button.reset_start_water_push();
  }

  delay(millisec_loop_step);
}
