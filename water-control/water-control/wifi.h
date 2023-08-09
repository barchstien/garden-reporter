#pragma once

#include <WiFiNINA.h>

// debug
#include <TimeLib.h>

// Expects arduino_secrets.h to be like :
// #define SECRET_SSID "your-ssid"
// #define SECRET_PASS "your-pass"
#include "arduino_secrets.h"

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

struct wifi_t
{
  wifi_t()
    : is_connected(false)
  {}

  void init()
  {
    WiFi.setTimeout(30000);
    // Try to connect on startup ?
    connect();
    //// wait for NTP sync
    //delay(10000);
    //sleep();
  }

  void connect()
  {
    int status = WiFi.status();
    if (status == WL_CONNECTED)
    {
      // already connected
      return;
    }
    
    Serial.print("-- Wifi connecting to ssid: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    if (status == WL_CONNECTED)
    {
      Serial.print("   Connected to ");
      Serial.println(WiFi.SSID());
      Serial.print("   rssi: ");
      Serial.println(WiFi.RSSI());
    }
    else if (status == WL_NO_MODULE)
      Serial.println("   Wifi module not found");
    else if (status == WL_IDLE_STATUS)
      Serial.println("   Idle...");
    else if (status == WL_NO_SSID_AVAIL)
      Serial.println("   No ssid availale");
    else if (status == WL_CONNECT_FAILED)
      Serial.println("   Connection failed");
    else if (status == WL_CONNECTION_LOST)
      Serial.println("   Connection lost");
    else
      Serial.println("Failed to connect !");
  }

  void wakeup()
  {
    WiFi.noLowPowerMode();
    //connect();
  }

  void sleep()
  {
    WiFi.lowPowerMode();
    //WiFi.disconnect();
    //WiFi.end();
    //is_connected = false;
  }

  void debug()
  {
    int t = WiFi.getTime();
    Serial.println("-- Wifi debug");
    Serial.print("getTime(): ");
    Serial.println(t);
    tmElements_t dt;
    time_t tt = t;
    breakTime(tt, dt);
    //dt.Year += 1970;
    Serial.print(dt.Year + 1970);
    Serial.print("/");
    Serial.print(dt.Month);
    Serial.print("/");
    Serial.print(dt.Day);
    Serial.print(" ");
    Serial.print(dt.Hour);
    Serial.print(":");
    Serial.print(dt.Minute);
    Serial.print(":");
    Serial.print(dt.Second);
    Serial.println();
  }

  void print_status()
  {
    int status = WiFi.status();
    if (status == WL_CONNECTED)
    {
      Serial.print("Connected to ");
      Serial.println(WiFi.SSID());
    }
    else
    {
      Serial.print("Disconnected, status: ");
      Serial.println(status);
      //std::to_string(status)).c_str()
    }
  }

  // keep ?
  bool is_connected;
};