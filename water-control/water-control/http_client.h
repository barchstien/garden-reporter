#pragma once

#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>

#include "time_lib.h"

//#define HTTP_SERVER_IP "192.168.1.66"
#define HTTP_SERVER_IP "192.168.1.176"
#define HTTP_SERVER_PORT 8000
#define HTTP_REPORT "/report"

struct http_client_t
{
  void report()
  {
    //
    WiFiClient client;
    HttpClient httpClient = HttpClient(client, HTTP_SERVER_IP, HTTP_SERVER_PORT);
    httpClient.get(HTTP_REPORT);

    // Read and parse the response
    int statusCode = httpClient.responseStatusCode();
    String responseBody = httpClient.responseBody();

    Serial.print("HTTP Response Code: ");
    Serial.println(statusCode);

    if (statusCode == 200) {
      Serial.println("Response Body:");
      Serial.println(responseBody);

      // Parse JSON
      DynamicJsonDocument jsonDoc(1024); // Adjust size as needed
      DeserializationError jsonError = deserializeJson(jsonDoc, responseBody);

      if (jsonError)
      {
        Serial.print("JSON Parsing Error: ");
        Serial.println(jsonError.c_str());
      }
      else
      {
        // Access JSON data
        JsonObject root = jsonDoc.as<JsonObject>();
        Serial.println(root["sec_since_1970"].as<String>());
        //String value = root["key_name"].as<String>(); // Replace key_name with the actual key in your JSON
        //Serial.print("Value from JSON: ");
        //Serial.println(value);
        auto t = break_to_element(root["sec_since_1970"].as<int>());
        Serial.println(t.to_string());
      }
    }
    else
    {
      Serial.println("HTTP Request Failed");
    }

    client.stop();
  }
};