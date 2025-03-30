# pragma once
#include <string>

typedef struct {
   std::string ghash;      // genesis hash
   std::string bhash;      // block_hash
   uint32_t version = 0;
   uint64_t nonce;
   uint64_t tip = 0;
   uint32_t specVersion;   // Runtime spec version 
   uint32_t tx_version;
   uint32_t era = 0;
} TxData;

typedef struct {
   std::string ghash;      // genesis hash
   std::string bhash;      // block_hash
   uint32_t version = 0;
   uint32_t specVersion;   // Runtime spec version 
   uint32_t tx_version;
   uint32_t era = 0;
   bool hasRunTimeData  = false;
   bool hasHash         = false;
} RunTimeData;