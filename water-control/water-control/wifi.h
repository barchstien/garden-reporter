#pragma once

#include <WiFiNINA.h>
#include <ArduinoJson.h>

//#include "time_lib.h"

#define HTTP_SERVER_IP "192.168.1.175"//66"
//#define HTTP_SERVER_IP "192.168.1.176"
#define HTTP_SERVER_PORT 8000
#define HTTP_REPORT "/report"
#define HTTP_DEBUG "/debug"

// Expects arduino_secrets.h to be like :
// #define SECRET_SSID "your-ssid"
// #define SECRET_PASS "your-pass"
#include "arduino_secrets.h"

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

#define GET_TIME_MAX_TRY   20
#define GET_TIME_WAIT_MSEC 1000

// debug
void printMacAddress(byte mac[])
{
  for (int i = 5; i >= 0; i--)
  {
    if (mac[i] < 16)
    {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0)
    {
      Serial.print(":");
    }
  }
  Serial.println();
}

struct wifi_t
{

  bool connect()
  {
    // debug list
    if (WiFi.status() == WL_NO_MODULE)
    {
      Serial.println("Communication with WiFi module failed!");
      delay(5000);
    }
    // print your MAC address:
    //byte mac[6];
    //WiFi.macAddress(mac);
    //Serial.print("MAC: ");
    //printMacAddress(mac);

    int status = WL_IDLE_STATUS;
    int cnt = 0;
    while (status != WL_CONNECTED && cnt < 20)
    {
      status = WiFi.begin(ssid, pass);
      delay(500);
      cnt++;
    }
    print_status();
    if (status == WL_CONNECTED)
    {
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

  bool is_connected()
  {
    return WiFi.status() == WL_CONNECTED;
  }

  int32_t rssi_dbm()
  {
    return WiFi.RSSI();
  }

  void print_status()
  {
    int status = WiFi.status();
    if (status == WL_CONNECTED)
    {
      Serial.print("Connected to ");
      Serial.print(WiFi.SSID());
      Serial.print(" : ");
      Serial.print(WiFi.localIP());
      Serial.print(" rssi: ");
      Serial.print(WiFi.RSSI());
      Serial.println(" dBm");
    }
    else
    {
      Serial.print("Disconnected, status: ");
      Serial.println(status);
    }
  }
};