// #pragma once
#ifndef WEBRPC_H
#define WEBRPC_H

#include <string>

#ifdef USE_ARDUINO

#ifdef ESP32_MODEL
#include <WiFi.h>
#include <HTTPClient.h>
#else
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#endif

#include <TxData.h>
#include <Defines.h>

std::string rpcGet(WiFiClient wifi, std::string url, std::string method, std::string param, uint64_t cnt)
{
     HTTPClient http;
     http.begin(wifi, url.c_str());
     http.addHeader("Content-Type", "application/json");

     String jsonString;
     std::string rpc_result = "";

     jsonString = rpcJson(method, param, cnt);
     log_printf("%s \n", jsonString.c_str());

     int httpCode = http.POST(jsonString);

     if (httpCode > 0)
     {
          log_printf("[HTTP]+POST code: %d\n", httpCode);
          if (httpCode == HTTP_CODE_OK)
          {
               const String &payload = http.getString();
               log_printf("%s \n", payload.c_str());
               std::string rpc_result = parseJsonResult(payload);
               return rpc_result;
          }
     }
     return rpc_result;
}

RunTimeData rpcGetRT(WiFiClient wifi, std::string url, std::string method, std::string param, uint64_t cnt)
{
     HTTPClient http;
     http.begin(wifi, url.c_str());
     http.addHeader("Content-Type", "application/json");

     String jsonString;
     RunTimeData rtd;

     jsonString = rpcJson(method, param, cnt);
     log_printf("%s \n", jsonString.c_str());

     int httpCode = http.POST(jsonString);

     if (httpCode > 0)
     {
          log_printf("[HTTP]+POST code: %d\n", httpCode);
          if (httpCode == HTTP_CODE_OK)
          {
               const String &payload = http.getString();
               log_printf("%s \n", payload.c_str());
               rtd = parseJsonRT(payload);
               return rtd;
          }
     }
     return rtd;
}

#else
// TODO use some htpp library i.e. httplib for linux host
#endif

#endif