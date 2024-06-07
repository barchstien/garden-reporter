#if 1
#pragma once

#include <WiFiNINA.h>
//#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>

#include "time_lib.h"

#define HTTP_SERVER_IP "192.168.1.175"//66"
//#define HTTP_SERVER_IP "192.168.1.176"
#define HTTP_SERVER_PORT 8000
#define HTTP_REPORT "/report"
#define HTTP_DEBUG "/debug"

struct http_reporter_t
{
  void report()
  {
    Serial.print("-- http report to ");
    Serial.print(HTTP_SERVER_IP);
    Serial.print(":");
    Serial.println(HTTP_SERVER_PORT);

    WiFiClient client;


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

      delay(1000);

      // Read and parse the response
      String response;
      while (client.available())
      {
        char c = client.read();
        //Serial.print("==>");
        //Serial.print(int(c));
        //Serial.print(" - ");
        //Serial.println(c);
        //Serial.write(c);
        response += c;
      }
      Serial.println(response);
      String json_data = response.substring(response.indexOf('{'));
      // Parse JSON
      DynamicJsonDocument jsonDoc(1024); // Adjust size as needed
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
        Serial.println(root["sec_since_1970"].as<String>());
        //String value = root["key_name"].as<String>(); // Replace key_name with the actual key in your JSON
        //Serial.print("Value from JSON: ");
        //Serial.println(value);
        auto t = break_to_element(root["sec_since_1970"].as<int>());
        Serial.print("GMT: ");
        Serial.println(t.to_string());
      }
    }
    else
    {
      Serial.println("Client NOT connected");
    }
    Serial.println("Client stop................");
    client.stop();


    
#if 0
    Serial.print("HTTP Response Code: ");
    Serial.println(statusCode);

    if (statusCode == 200)
    {
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
 #endif
  }

  // debug
  void debug(String msg)
  {
#if 0
    WiFiClient client;
    HttpClient httpClient = HttpClient(client, HTTP_SERVER_IP, HTTP_SERVER_PORT);
    String content_type = "application/x-www-form-urlencoded";
    String put_data = "debug="+msg;
    httpClient.put(HTTP_DEBUG, content_type, put_data);

    // Read and parse the response
    int statusCode = httpClient.responseStatusCode();
    String responseBody = httpClient.responseBody();

    Serial.print("HTTP Debug Response Code: ");
    Serial.println(statusCode);

    if (statusCode != 200)
    {
      Serial.println("HTTP Debug Request Failed");
    }

    client.stop();
#endif
    Serial.println("-- http debug");
    WiFiClient client;
    if (client.connect(HTTP_SERVER_IP, HTTP_SERVER_PORT))
    {
      Serial.println("Client connected");
    }
    else
    {
      Serial.println("Client NOT connected");
    }
    client.stop();
  }
};
#endif