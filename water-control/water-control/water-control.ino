//#include <ArduinoLowPower.h>

#include "battery.h"
#include "button.h"
#include "epoch_time_t.h"
#include "led.h"
#include "http_reporter.h"
#include "valve.h"
#include "web_log.hpp"
#include "wifi.h"

uint32_t flow_cnt = 0;
uint32_t flow_cnt_last = 0;
const float flow_cnt_liter_per_edge = 0.00245;
//const float flow_cnt_liter_per_edge = 0.00237;

uint32_t start_water_cnt = 0;
uint32_t start_water_cnt_last = 0;

// Wifi report period
const unsigned int sec_report_period = (15 * 60); // TODO 15 min
// local clock of last report, or invalid if previously failed
epoch_time_t last_report_ = -1;
const unsigned int sec_report_period_when_watering = 60;
const unsigned int water_loop_period_sec = 5;

// Hard limit watering duration, 1.5 hour
const int64_t MAX_WATER_DURATION_SEC = 5400;

// delay between loop
const unsigned int millisec_loop_step = (100);

// Twice report period, to avoid missing first watering
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
led_t led(&epoch_time_sync);
web_log_t web_log;
valve_t valve;
wifi_t wifi;
http_reporter_t reporter;

// Command received by server
http_reporter_t::command_t server_cmd;

// Log tp serial/usb
#define LOG(X) ({ \
  Serial.print(epoch_time_sync.now_as_string()); \
  Serial.print(" - "); \
  Serial.print(X); \
})

// Log with a timestamp to web server. Send unix time as hex string
#define WEB_LOG(STR) ({ \
  epoch_time_t now = epoch_time_sync.now(); \
  web_log.append(String((uint32_t)(now >> 32), HEX)); \
  web_log.append(String((uint32_t)(now & 0xffffffff), HEX) + " "); \
  web_log.append(STR); \
  web_log.commit(); \
})

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

  WEB_LOG("Setup END");
}


/**
 * @warning Expects Wifi to be connected !
*/
report_status sync_with_server(bool allow_clock_adjust)
{
  report_status rs = report_status::FAILURE;
  
  // Compute water since last successful report
  int32_t flow_cnt_tmp = flow_cnt;
  float water_liter = (float)(flow_cnt_tmp - flow_cnt_last) * flow_cnt_liter_per_edge;

  http_reporter_t::command_t cmd = reporter.report(
    water_liter,
    battery.read_volt(),
    next_water_schedule,
    server_cmd.enabled,
    last_water_schedule,
    valve.is_on(),
    epoch_time_sync.uptime_sec(),
    wifi.rssi_dbm(),
    epoch_time_sync.rtc_temp_celsius(),
    &web_log
  );
  
  if (cmd.is_valid)
  {
    // report was succesful, reset flow cnt last
    flow_cnt_last = flow_cnt_tmp;

    if (allow_clock_adjust)
    {
      int offset_applied = epoch_time_sync.set_now(cmd.sec_since_epoch);
      if (offset_applied != 0)
      {
        WEB_LOG(String("Adjusted RTC clock by ") + offset_applied + String(" seconds"));
      }
    }

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
      WEB_LOG("Apply new schedule from server");
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

void water_for_duration(int32_t duration_sec, bool is_manual_triggered)
{
  if (duration_sec > MAX_WATER_DURATION_SEC)
  {
    duration_sec = MAX_WATER_DURATION_SEC;
  }

  epoch_time_t deadline = epoch_time_sync.now() + duration_sec;

  LOG(" >>>> Water NOW <<<< duration minutes: ");
  Serial.println((float)duration_sec / 60.0);
  WEB_LOG(String("Water for ") + (int)(((float)duration_sec / 60.0) + 0.5) + String(" minutes"));
  //digitalWrite(LED_BUILTIN, HIGH);
  valve.water_on();

  // Report on first iteration
  epoch_time_t http_report_deadline = epoch_time_sync.now();

  // Report as long as it is watering
  // ... and check if config get changed
  while(epoch_time_sync.now() < deadline)
  {
    // Cancel if button is off or battery low
    if (button.allow_water() == false)
    {
      LOG("Cancel watering, by order of button");
      Serial.println("");
      WEB_LOG("Cancel watering, by order of button");
      break;
    }
    if (battery.can_use_water() == false)
    {
      LOG("Cancel watering, by order of battery");
      Serial.println("");
      WEB_LOG("Cancel watering, by order of battery");
      break;
    }
    if (wifi.is_connected() && epoch_time_sync.now() >= http_report_deadline)
    {
      // HTTP report
      led.on();
      report_status rs = sync_with_server(false);
      // manual triger ignores water schedule enable/disable
      if ( !is_manual_triggered && rs == report_status::CMD_APPLIED)
      {
        // Check enabled only to interrupt the loop
        // other config were applied in sync_with_server()
        if (server_cmd.enabled == false)
        {
          LOG("Cancel scheduled watering, by order of server");
          Serial.println("");
          WEB_LOG("Cancel scheduled watering, by order of server");
          break;
        }
      }
      http_report_deadline = epoch_time_sync.now() + sec_report_period_when_watering;
      led.off();
    }
    // fade also means delay
    led.fade(water_loop_period_sec, 1000);
  }
  
  //digitalWrite(LED_BUILTIN, LOW);
  valve.water_off();
  LOG(" >>>> STOP Water <<<<");
  WEB_LOG(" >>>> STOP Water <<<<");
  Serial.println("");
}


void loop()
{
  //// HTTP report ////
  // Send status and get config including epoch time sync and watering schedule
  if (last_report_ < 0 || (epoch_time_sync.now() - last_report_ > sec_report_period))
  {
    // Report via wifi
    led.on();
    if (wifi.connect())
    {
      report_status rs = sync_with_server(true);
      if (CMD_APPLIED == rs)
      {
        // Sync with success
        // received a new config, re-report to update status
        sync_with_server(true);
      }
      else if (FAILURE == rs)
      {
        // Also update timestamp, to avoid fast re-try
        // ... which may happen if server or net is down
        LOG("[!] Failed periodic sync with server !!");
      }
      last_report_ = epoch_time_sync.now();
    }
    // Regardless of success or not, turn off Wifi
    wifi.end();
    led.off();
  }

  //// Water schedule ////
  if (server_cmd.is_valid)
  {
    // Perfom scheduled watering
    // If water isn't allowed/enabled still do update next watering coz it has passed
    if (epoch_time_sync.now() > next_water_schedule)
    {
      led.on();
      wifi.connect();
      if (button.allow_water() && battery.can_use_water() && server_cmd.enabled)
      {
        water_for_duration(server_cmd.duration_minute * 60, false);
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
      if (wifi.is_connected())
      {
        sync_with_server(false);
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
      LOG("-- Manual trigger HTTP report");
      last_report_ = -1;
    }
    else
    {
      LOG("-- Manual trigger water");
      // check button on/off and battery
      if (button.allow_water() && battery.can_use_water())
      {
        led.on();
        wifi.connect();
        
        // multiple of 15 minutes
        uint32_t duration_sec = button.bit_value() * 15 * 60;
        water_for_duration(duration_sec, true);
        
        // trigger report to HTTP server on next iteration
        last_report_ = -1;
        
        wifi.end();
        led.off();
      }
    }
    button.reset_start_water_push();
  }

  delay(millisec_loop_step);
}
