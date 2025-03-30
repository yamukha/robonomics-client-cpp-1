#pragma once

#include <cstdlib>
#include <optional>
#include <vector>
#include <string>
#include "Defines.h"
#include "TxData.h"
#include "Utils.h"

#ifdef USE_LINUX
typedef std::string String;
#include "../json/json.hpp"
#else
#include <Arduino_JSON.h>
#endif

TxData parseJson (String val) {
   log_printf("parse: %s\n", val.c_str());
   TxData txData;
   txData.nonce = (long int) std::atoll(val.c_str());
   log_printf("Nonce %lu, tip %lu, era %u \n", txData.nonce, txData.tip, txData.era);
   return txData;
}

String getPayloadJs (std::string account, uint64_t id_cnt) {
#ifdef USE_LINUX
    nlohmann::json params;
    nlohmann::json get_payload;
#else
    JSONVar params;
    JSONVar get_payload;
#endif

    String jsonString;
    params [0] = account.c_str();

    get_payload["jsonrpc"] = "2.0";
    get_payload["id"] = (double) id_cnt; // to increment
    get_payload["method"] = "account_nextIndex"; //  system_accountNextIndex or account_nextIndex instead of obsolete get_paylod
    get_payload["params"] = params;
 #ifdef USE_LINUX
    jsonString = get_payload.dump();
#else
    jsonString = JSON.stringify(get_payload);
#endif
    return jsonString;
}

String fillParamsJs (std::vector<uint8_t> data, uint64_t id_cnt) {
#ifdef USE_LINUX
    nlohmann::json params;
    nlohmann::json extrinsic;
#else
    JSONVar params;
    JSONVar extrinsic;
#endif

    String jsonString;
    std::string param0 = "0x" + toHexString(data);
    params [0] = param0.c_str();

    extrinsic["jsonrpc"] = "2.0";
    extrinsic["id"] = (double) id_cnt;
    extrinsic["method"] = "author_submitExtrinsic";
    extrinsic["params"] = params;

#ifdef USE_LINUX
    jsonString = extrinsic.dump();
#else
    jsonString = JSON.stringify(extrinsic);
#endif
    return jsonString;
}

#ifdef USE_LINUX
// TODO not Arduino json parsers
RunTimeData parseJsonRT (String payload) {
  RunTimeData rtd;
  return rtd;
}
std::string parseJsonResult (String payload) {
  return "";
}

String rpcJson(std::string method, std::string param, uint64_t id_cnt) {
    String js;
    nlohmann::json payload;

    payload["jsonrpc"] = "2.0";
    payload["id"] = id_cnt;
    payload["method"] = method;
    if ("" != param)
    {
      payload["params"] [0] = param;
    }

    js = payload.dump().c_str();
    return js;
}

#else
String rpcJson(std::string method, std::string param, uint64_t id_cnt)
{
   String js;
   JSONVar params;
   if ("" != param)
   {
      params[0] = param.c_str();
   }

   JSONVar payload;
   payload["jsonrpc"] = "2.0";
   payload["id"] = (double)id_cnt;
   payload["method"] = method.c_str();
   payload["params"] = params;

   js = JSON.stringify(payload);
   return js;
}

std::string parseJsonResult (String payload) {
    std::string result = "";
    JSONVar myObject = JSON.parse(payload);
    if (myObject.hasOwnProperty("result")) {
      result = std::string((const char *)myObject["result"]);
    }
    if (myObject.hasOwnProperty("error")) {
      String as = JSON.stringify(myObject["error"]);
      std::string strRes (as.c_str());
      result = strRes;
    }
    log_printf("result: %s\n", result.c_str());
    return result;
}

std::optional <int> parseJsonResultInt (String payload) {
    std::optional <int> result;
    JSONVar myObject = JSON.parse(payload);
    if (myObject.hasOwnProperty("result")) {
      result = myObject["result"];
      //log_printf("parsed result: %d\n", result.value());
    } else 
      log_printf("Cannot parse result as int\n");
    return result;
}

RunTimeData parseJsonRT (String payload) {
    RunTimeData rtData;
    bool hasSpecVer = false;
    bool hasTxVer = false;
    JSONVar myObject = JSON.parse(payload);
    if (myObject.hasOwnProperty("result")) {
      JSONVar jResult = myObject["result"];
      if (jResult.hasOwnProperty("specVersion")) {
        long int specVersion = (long int) jResult["specVersion"];
        rtData.specVersion = (uint32_t) specVersion;
        log_printf("specVersion: %u\n", rtData.specVersion);
        hasSpecVer = true;
      };
      if (jResult.hasOwnProperty("transactionVersion")) {
        long int tx_version =  (long int) jResult["transactionVersion"];
        rtData.tx_version = (uint32_t) tx_version;
        log_printf("tx_version: %u\n", rtData.tx_version);
        hasTxVer = true;
      };
    };
    if (hasSpecVer && hasTxVer)
      rtData.hasRunTimeData = true;
    return rtData;
}
#endif
