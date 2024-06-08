#if 1
#pragma once

#include <WiFiNINA.h>
#include <ArduinoJson.h>

//#include "time_lib.h"

#define HTTP_SERVER_IP "192.168.1.175"//66"
//#define HTTP_SERVER_IP "192.168.1.176"
#define HTTP_SERVER_PORT 8000
#define HTTP_REPORT "/report"
#define HTTP_DEBUG "/debug"

struct http_reporter_t
{
  WiFiClient client;

  /** Response from server */
  struct command_t
  {
    uint64_t sec_since_epoch{0};
    bool enabled{false};
    uint64_t start_time_sec_since_epoch{0};
    unsigned int period_day{0};
    unsigned int duration_minute{0};
    bool is_valid{false};
  };

  command_t report()
  {
    Serial.print("-- http report to ");
    Serial.print(HTTP_SERVER_IP);
    Serial.print(":");
    Serial.println(HTTP_SERVER_PORT);

    command_t cmd;

    if (client.connect(HTTP_SERVER_IP, HTTP_SERVER_PORT))
    {
      Serial.println("Client connected");
      // Make a HTTP request:
      client.println("GET /report HTTP/1.1");
      client.print("Host: ");
      client.print(HTTP_SERVER_IP);
      client.print(":");
      client.print(HTTP_SERVER_PORT);
      client.println();
      client.println("Connection: close");
      client.println();

      // give server time to respond
      delay(1000);

      // Read and parse the response
      String response;
      while (client.available())
      {
        char c = client.read();
        response += c;
      }
      Serial.println("");
      Serial.print("server response: ");
      Serial.println(response);
      String json_data = response.substring(response.indexOf('{'));
      // Parse JSON
      DynamicJsonDocument jsonDoc(1024);
      DeserializationError jsonError = deserializeJson(jsonDoc, json_data.c_str());

      if (jsonError)
      {
        Serial.print("JSON Parsing Error: ");
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
      Serial.println("Client NOT connected");
    }
    Serial.println("Client stop................");
    client.stop();
    return cmd;
  }

};
#endif