#pragma once

#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>

//#define HTTP_SERVER_IP "192.168.1.66"
#define HTTP_SERVER_IP "192.168.1.176"
#define HTTP_SERVER_PORT 8000
#define HTTP_REPORT "/report"

struct http_client_t
{
  void init()
  {
  }

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
    }
    else
    {
      Serial.println("HTTP Request Failed");
    }

    client.stop();
  }
};