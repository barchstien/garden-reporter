#pragma once

#include <WiFiNINA.h>
#include <ArduinoJson.h>

#include "epoch_time_t.h"
#include "web_log.hpp"

#define HTTP_SERVER_IP "192.168.1.66"
//#define HTTP_SERVER_IP "192.168.1.176"
#define HTTP_SERVER_PORT 8000
#define HTTP_REPORT "/report"
#define HTTP_DEBUG "/debug"

#define MAX_REQUEST_TRY 2
#define MAX_WAIT_RESPONSE_SEC 10

#define READ_BUFF_SIZE 1024

enum report_status
{
  FAILURE,
  CMD_ALREADY_KOWN,
  CMD_APPLIED
};

struct http_reporter_t
{
  WiFiClient client;

  /** Response from server */
  struct command_t
  {
    // Time sync
    uint64_t sec_since_epoch{0};
    // Water schedule
    bool enabled{false};
    uint64_t start_time_sec_since_epoch{0};
    uint32_t period_day{0};
    uint32_t duration_minute{0};
    // status
    bool is_valid{false};

    /** @warning Compare water schedule params only */
    friend bool operator==(const command_t &left, const command_t &right)
    {
      return left.enabled == right.enabled
        && left.start_time_sec_since_epoch == right.start_time_sec_since_epoch
        && left.period_day == right.period_day
        && left.duration_minute == right.duration_minute;
    };
  };

  command_t report(
    float water_liter, 
    float battery_voltage,
    epoch_time_t next_water_schedule,
    bool water_schedule_enabled,
    epoch_time_t last_water_schedule,
    bool water_on,
    uint32_t uptime_sec,
    int rssi_dbm,
    float temperature_celsius,
    web_log_t* web_log)
  {
    //Serial.print("-- http report to ");
    //Serial.print(HTTP_SERVER_IP);
    //Serial.print(":");
    //Serial.println(HTTP_SERVER_PORT);

    command_t cmd;

    unsigned int cnt = 0;
    while (
      client.connect(HTTP_SERVER_IP, HTTP_SERVER_PORT) == false
      && cnt < MAX_REQUEST_TRY)
    {
      cnt ++;
      delay(1000);
    }
    if (cnt < MAX_REQUEST_TRY)
    {
      // Make a HTTP request:
      client.print("GET /report?water_liter=");
      client.print(int(water_liter * 1000.0 + 0.5));
      client.print("&battery_milliv=");
      client.print(int(battery_voltage * 1000.0 + 0.5));
      client.print("&next_water_epoch_t=");
      client.print(next_water_schedule);
      client.print("&water_sch_enabled=");
      client.print(water_schedule_enabled);
      client.print("&last_water_epoch_t=");
      client.print(last_water_schedule);
      client.print("&water_on=");
      client.print(water_on);
      client.print("&uptime_sec=");
      client.print(uptime_sec);
      client.print("&rssi_dbm=");
      client.print(rssi_dbm);
      client.print("&temperature_celsius=");
      client.print(temperature_celsius);
      web_log->write_to_client(&client);
      client.println(" HTTP/1.1");
      client.print("Host: ");
      client.print(HTTP_SERVER_IP);
      client.print(":");
      client.print(HTTP_SERVER_PORT);
      client.println();
      client.println("Connection: close");
      client.println();

      // Give server time to respond
      bool timed_out = true;
      local_clock_t start = local_clock_t::now();
      while (local_clock_t::now() - start < local_clock_t::seconds(MAX_WAIT_RESPONSE_SEC))
      {
        if (client.available())
        {
          timed_out = false;
          break;
        }
        delay(10);
      }

      if (timed_out)
      {
        Serial.println("** Didn't receive response from report !");
      }

      // Read and parse the response
      char response[READ_BUFF_SIZE];
      unsigned int i = 0;
      while (client.available())
      {
        char c = client.read();
        response[i] = c;
        i++;
        if (i >= READ_BUFF_SIZE - 1)
        {
          Serial.println("** READ_BUFF_SIZE too small, message truncated");
          break;
        }
        // May be reading too fast, missing bytes, give it a chance
        if (client.available() == false)
        {
          delay(100);
        }
      }
      response[i] = '\0';
      //Serial.println("");
      //Serial.print("server response: ");
      //Serial.println(response);
      //Serial.println("");
      // Point to JSON data start
      char* json_data_ptr = strchr(response, '{');
      // Parse JSON
      DynamicJsonDocument jsonDoc(READ_BUFF_SIZE);
      DeserializationError jsonError = deserializeJson(jsonDoc, json_data_ptr);

      if (jsonError)
      {
        Serial.print("** JSON Parsing Error: ");
        Serial.println(jsonError.c_str());
      }
      else
      {
        // Access JSON data
        JsonObject root = jsonDoc.as<JsonObject>();
        cmd.sec_since_epoch = root["sec_since_1970"].as<uint64_t>();
        cmd.enabled = root["enabled"].as<bool>();
        cmd.start_time_sec_since_epoch = root["start_time"].as<uint64_t>();
        cmd.period_day = root["period_day"].as<unsigned int>();
        cmd.duration_minute = root["duration_minute"].as<unsigned int>();
        cmd.is_valid = true;
      }
    }
    else
    {
      Serial.println("Client failed to connect. Is server up ?");
    }
    client.stop();
    return cmd;
  }

};
