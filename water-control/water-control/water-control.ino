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

uint32_t start_water_cnt = 0;
uint32_t start_water_cnt_last = 0;

// Wifi report period
const unsigned int sec_report_period = (60 * 1); // TODO 15 min
// value returned by millis() when reported last
local_clock_t last_report_;

// Hard limit watering duration, 1hour
const int64_t MAX_WATER_DURATION_SEC = 3600;

// delay between loop
const unsigned int millisec_loop_step = (3000);//(100); // TODO revert to 100

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
  
  // Why ?
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println("--");
  Serial.println("-- setup END");

#if 0
  // debug
  delay(30000);
  valve.water_on();
  delay(60000);
  valve.water_off();
#endif
}

#define LOG(X) ({ \
  Serial.print(epoch_time_sync.now()); \
  Serial.print(" - "); \
  Serial.print(X); \
})

void loop()
{
  //// debug
  Serial.print("epoch time: ");
  Serial.println(epoch_time_sync.now());

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
    // TODO get flow trig, last scheduled, battery
    if (wifi.connect())
    {
      http_reporter_t::command_t cmd = reporter.report(
        flow_cnt,
        battery.read_volt()
      );
      if (cmd.is_valid)
      {
        Serial.println("=== CMD FROM SERVER ===");
        epoch_time_sync.set_now(cmd.sec_since_epoch);
        if (server_cmd == cmd)
        {
          Serial.println("--> Ignore server command, already known");
        }
        else
        {
          server_cmd = cmd;
          Serial.println("--> Apply server command");
          // A config has been received from server
          // TODO
          // check current epoch time vs next scheduled
          // add duration to current and set deadline
          next_water_schedule = server_cmd.start_time_sec_since_epoch;
          while(next_water_schedule < epoch_time_sync.now() - water_schedule_margin_sec)
          {
            next_water_schedule += server_cmd.period_day * 24 * 60 * 60;
          }
          Serial.print("Next watering at ");
          Serial.println(next_water_schedule);
          Serial.print("happening in sec: ");
          Serial.println(next_water_schedule - epoch_time_sync.now());
        }
      }
    }
    // Regardless success or not, turn off Wifi
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
      if (next_water_schedule > epoch_time_sync.now())
      {
        uint32_t duration_sec = server_cmd.duration_minute * 60;
        if (duration_sec > MAX_WATER_DURATION_SEC)
        {
          duration_sec = MAX_WATER_DURATION_SEC;
        }
        epoch_time_t deadline = epoch_time_sync.now() + duration_sec;
        Serial.println("Water NOW ...................");
        Serial.println("^^^^^ ^^^ ...................");
        //digitalWrite(LED_BUILTIN, HIGH);
        //valve.water_on();

        // TODO check wifi, stay connected, to see if it gets disabled !

        //digitalWrite(LED_BUILTIN, LOW);
        //valve.water_off();
        Serial.println("STOP Water ...................");

        // re-schedule
        last_water_schedule = next_water_schedule;
        next_water_schedule += server_cmd.period_day * 24 * 60 * 60;
        Serial.print("Next watering at ");
        Serial.println(next_water_schedule);
        Serial.print("happening in sec: ");
        Serial.println(next_water_schedule - epoch_time_sync.now());
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
    }
    button.reset_start_water_push();
  }

  //Serial.println("---------------");
  delay(millisec_loop_step);
}
