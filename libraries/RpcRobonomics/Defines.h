#pragma once 

#define USE_ARDUINO

#ifdef USE_ARDUINO
// ESP32 or ES8266 in other case
#define ESP32_MODEL
#endif

#define KEYS_SIZE 32
#define SIGNATURE_SIZE 64
//#define URLRPC "http://kusama.rpc.robonomics.network/rpc/"
#define URLRPC "http://192.168.0.104:9944"
#define DEBUG_PRINT
#define LOG_PRINT
#define MAX_PAYLOAD_DATA_LEN 256
#define BLAKE2_HASH_LEN 32
#define ARRAY_LEN_64 64
#define SS58_PRE "SS58PRE"

#ifdef USE_LINUX
#undef USE_ARDUINO
#endif

#ifdef NO_DEBUG_LOG_PRINT
#undef DEBUG_PRINT
#undef LOG_PRINT
inline void no_print(const char *fmt, ...) {}
#endif

#ifdef USE_ARDUINO
#define debug_printf(...) Serial.printf(__VA_ARGS__)
#define debug_println(...) Serial.println(__VA_ARGS__)
#else
#ifdef DEBUG_PRINT
#define debug_printf(...) printf(__VA_ARGS__)
#define debug_println(...) printf(__VA_ARGS__, "\n")
#else
#define debug_printf(...) no_print(__VA_ARGS__)
#define debug_println(...) no_print(__VA_ARGS__)
#endif
#endif

#ifdef USE_ARDUINO
#define log_printf(...) Serial.printf(__VA_ARGS__)
#define log_println(...) Serial.println(__VA_ARGS__)
#else
#ifdef LOG_PRINT
#define log_printf(...) printf(__VA_ARGS__)
#define log_println(...) printf(__VA_ARGS__, "\n")
#else
#define log_printf(...) no_print(__VA_ARGS__)
#define log_println(...) no_print(__VA_ARGS__)
#endif
#endif

