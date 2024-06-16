//#include <ArduinoLowPower.h>

#include "battery.h"
#include "button.h"
#include "epoch_time_t.h"
#include "led.h"
#include "http_reporter.h"
#include "valve.h"
#include "wifi.h"

uint32_t flow_cnt = 0;
uint32_t flow_cnt_last = 0;
const float liter_per_edge = 0.00245;

uint32_t start_water_cnt = 0;
uint32_t start_water_cnt_last = 0;

// Wifi report period
const unsigned int sec_report_period = (60 * 1); // TODO 15 min
// value returned by millis() when reported last
local_clock_t last_report_;

// Hard limit watering duration, 1.5 hour
const int64_t MAX_WATER_DURATION_SEC = 5400;

// delay between loop
const unsigned int millisec_loop_step = (5000);//(100); // TODO revert to 100

const epoch_time_t water_schedule_margin_sec = (10*60); // TODO 15 min ?
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
  Serial.begin(9600);
  Serial.println("--");
  Serial.println("-- setup");

  pinMode(FLOW_TRIG, INPUT_PULLUP);
  attachInterrupt(
    digitalPinToInterrupt(FLOW_TRIG), flow_trig_isr, FALLING
  );
  
  button.init();
  epoch_time_sync.init();
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

#define LOG(X) ({ \
  Serial.print(epoch_time_sync.now()); \
  Serial.print(" - "); \
  Serial.print(X); \
})

/**
 * @warning Expects Wifi to be connected !
*/
report_status report_via_wifi()
{
  report_status rs = report_status::FAILURE;
  http_reporter_t::command_t cmd = reporter.report(
    flow_cnt,
    battery.read_volt(),
    next_water_schedule,
    last_water_schedule,
    valve.is_on(),
    epoch_time_sync.uptime_sec()
  );
  if (cmd.is_valid)
  {
    Serial.println("=== CMD FROM SERVER ===");
    epoch_time_sync.set_now(cmd.sec_since_epoch);
    if (server_cmd == cmd)
    {
      Serial.println("--> Ignore server command, already known");
      rs = report_status::CMD_ALREADY_KOWN;
    }
    else
    {
      server_cmd = cmd;
      Serial.println("--> Apply server command");
      // A config has been received from server
      // ensure next scheduled water is in the future
      next_water_schedule = server_cmd.start_time_sec_since_epoch;
      while(next_water_schedule < epoch_time_sync.now() - water_schedule_margin_sec)
      {
        next_water_schedule += server_cmd.period_day * 24 * 60 * 60;
      }
      Serial.print("Next watering at ");
      Serial.println(next_water_schedule);
      Serial.print("happening in sec: ");
      Serial.println(next_water_schedule - epoch_time_sync.now());
      rs = report_status::CMD_APPLIED;
    }
  }
  return rs;
}


void loop()
{
  //// debug
  Serial.print("epoch time: ");
  Serial.println(epoch_time_sync.now());

  Serial.print("uptime sec: ");
  Serial.println(epoch_time_sync.uptime_sec());

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
#if 1
    // Report via wifi
    led.on();
    if (wifi.connect())
    {
      if (CMD_APPLIED == report_via_wifi())
      {
        // received a new config, re-report to update status
        report_via_wifi();
      }
    }
    // Regardless success or not, need to turn off Wifi
    wifi.end();
    led.off();
    last_report_ = local_clock_t::now();
#endif
  }

  if (server_cmd.is_valid)
  {
    // Perfom scheduled watering
    // Keep in this scope until water should be off <------ NOPE !
    // ... coz that would mean that water can't be canceled
    if (button.allow_water() && battery.can_use_water() && server_cmd.enabled)
    {
      if (epoch_time_sync.now() > next_water_schedule)
      {
        uint32_t duration_sec = server_cmd.duration_minute * 60;
        if (duration_sec > MAX_WATER_DURATION_SEC)
        {
          duration_sec = MAX_WATER_DURATION_SEC;
        }
        epoch_time_t deadline = epoch_time_sync.now() + duration_sec;
        Serial.println("");
        Serial.println(" >>>> Water NOW <<<<");
        Serial.println("");

        led.on();
        //digitalWrite(LED_BUILTIN, HIGH);
        valve.water_on();
        bool wifi_is_connected = false;
        if (wifi.connect())
        {
          wifi_is_connected = true;
        }
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
                Serial.println("Cancel scheduled watering, by order of server");
                break;
              }
            }
          }
          if (button.allow_water() == false)
          {
            Serial.println("Cancel scheduled watering, by order of button");
            break;
          }
          // fade for 5 sec
          led.fade(5, 1000);
        }
        
        //digitalWrite(LED_BUILTIN, LOW);
        valve.water_off();
        led.off();
        Serial.println("");
        Serial.println(" >>>> STOP Water <<<<");
        Serial.println("");

        // re-schedule
        last_water_schedule = next_water_schedule;
        next_water_schedule += server_cmd.period_day * 24 * 60 * 60;
        Serial.print("Next watering at ");
        Serial.println(next_water_schedule);
        Serial.print("happening in sec: ");
        Serial.println(next_water_schedule - epoch_time_sync.now());

        // report to update status
        report_via_wifi();

        // Regardless success or not, need to turn off Wifi
        wifi.end();
      }
    }
  }

  if (button.start_water_was_pushed())
  {
    // check button on/off and battery
    if (button.allow_water() && battery.can_use_water())
    {
      valve.water_on();
      uint32_t duration_sec = button.bit_value() * 5;
      LOG("Manual start second : ");
      Serial.println(duration_sec);
      led.fade(duration_sec, 1000);
      valve.water_off();
      led.off();
    }
    button.reset_start_water_push();
  }

  //Serial.println("---------------");
  delay(millisec_loop_step);
}
