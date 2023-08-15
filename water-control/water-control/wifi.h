#pragma once

#include <WiFiNINA.h>

#include "time_lib.h"

// Expects arduino_secrets.h to be like :
// #define SECRET_SSID "your-ssid"
// #define SECRET_PASS "your-pass"
#include "arduino_secrets.h"

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

#define GET_TIME_MAX_TRY   20
#define GET_TIME_WAIT_MSEC 1000

struct wifi_t
{
  void init()
  {
    //WiFi.setTimeout(10000);
    // Connect on init, gives time to NTP lock
    bool connected = connect();
    //// wait for NTP sync
    if (connected)
    {
      ntp_time_sync();
    }
  }

  bool connect()
  {
    int status = WiFi.status();
    if (status == WL_CONNECTED)
    {
      // already connected
      return true;
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
      return true;
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
    return false;
  }

  void end()
  {
    WiFi.end();
  }

  void ntp_time_sync()
  {
    int t = WiFi.getTime();
    int cnt = 0;
    while(t == 0 && cnt < GET_TIME_MAX_TRY)
    {
      delay(GET_TIME_WAIT_MSEC);
      t = WiFi.getTime();
      cnt ++;
    }
    if (t == 0)
    {
      Serial.println("Failed to get NTP time, using already set time");
      time_element_t dt = now_element();
      Serial.println(dt.to_string());
    }
    else
    {
      Serial.println("Got NTP time");
      set_time(t);
      // print
      time_element_t dt = now_element();
      Serial.println(dt.to_string());
    }
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
    }
  }
};