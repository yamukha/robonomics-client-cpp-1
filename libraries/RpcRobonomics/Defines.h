#pragma once 

#define USE_ARDUINO

#ifdef USE_ARDUINO
// ESP32 or ES8266 in other case
// #define ESP32_MODEL
#endif

#define KEYS_SIZE 32
#define SIGNATURE_SIZE 64
#define URLRPC "http://kusama.rpc.robonomics.network/rpc/"
//#define URLRPC "http://192.168.0.100:9933"
//#define DEBUG_PRINT
