#pragma once 

#include <Defines.h>
#include <Extrinsic.h>
#include <Call.h>
#include <JsonUtils.h>

#ifdef ESP32_MODEL
#include <WiFi.h>
#include <HTTPClient.h>
#else
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#endif

#ifdef USE_ARDUINO
#include <BLAKE2b.h>
#else
#include <Blake2.h>
#endif

typedef struct {
   std::string body;      // responce body
   uint32_t code;         // http responce code 200, 404, 500 etc.
} RpcResult;

class RobonomicsRpc { 
  public:     
    RobonomicsRpc (WiFiClient client, std::string url, std::string key, std::string ss58adr, uint64_t id, RunTimeData rtd)
        : wifi_(client), url_(url), ss58adr_(ss58adr), isGetParameters_ (true), id_counter_(id)
        {  
          std::vector<uint8_t> vk = hex2bytes(key);
          std::copy(vk.begin(), vk.end(), privateKey_);
  
          Ed25519::derivePublicKey(publicKey_, privateKey_);
          is_remote_url_ = getTypeUrl(url);

          // call headers initialization
          if (is_remote_url_) {
             head_bt_  = Data{0x1f,0};
             head_dr_  = Data{0x33,0};
             head_rws_ = Data{0x37,0};
          } else {
             head_bt_ = Data{7,0};
             head_dr_ = Data{0x11,0};
             head_rws_ = Data{0x13,0};
          }

          // Get from ctor TxData struct
          if (rtd.hasHash ) {
            // use runtime data
            bhash_ = rtd.bhash.erase(0,2); // remove '0x'
            ghash_ = rtd.ghash.erase(0,2);
          } else {
            // use defaults
            ghash_ = getBlockHash(is_remote_url_);
            bhash_ = getBlockHash(is_remote_url_);
          }

          if (rtd.hasRunTimeData) {
            // use runtime data
            specVersion_ = rtd.specVersion;
            tx_version_  = rtd.tx_version;
          } else {
            // use defaults
            tx_version_ = 1;
            if (is_remote_url_) // local 1 remote 33
              specVersion_ = 33;
            else
              specVersion_ = 1;
          }
          log_printf("Node mode: %s, genesis block hash: 0x%s\n", is_remote_url_?"remote":"local", ghash_.c_str());
        };

    RpcResult DatalogRecord (std::string record) {

    Data edata_;
    
    for (int a = 0 ; a < 2;  a++) {

      HTTPClient http;    
      http.begin(wifi_, url_.c_str());
      http.addHeader("Content-Type", "application/json");
      log_printf("[HTTP]+POST:\n");
      JSONVar params; 
      String jsonString;
      if (isGetParameters_) {
        jsonString = getPayloadJs (ss58adr_,id_counter_);
      } else {
        jsonString = fillParamsJs (edata_,id_counter_);
        edata_.clear();
      }
      log_println("sent:");
      log_println(jsonString);
      id_counter_++;
    
      int httpCode = http.POST(jsonString);

      if (httpCode > 0) {
          log_printf("[HTTP]+POST code: %d\n", httpCode);
            if (httpCode == HTTP_CODE_OK) {
              const String& payload = http.getString();
              log_println("received:");
              log_println(payload);
         
              JSONVar myObject = JSON.parse(payload);
              if (JSON.typeof(myObject) == "undefined") {
                  log_println("");
                  RpcResult r {"Parsing input failed!", -100};
                  return r;               
              } else {
                // RPC FSM                 
                bool res_ = false;
                TxData txData = {.ghash = ghash_, .bhash = bhash_, .specVersion = specVersion_, .tx_version = tx_version_};
                std::optional <int> parsedRes;
                if (isGetParameters_ && a ==0)
                  parsedRes = parseJsonResultInt(payload);
                if (parsedRes.has_value()) {
                  txData.nonce = parsedRes.value();
                  log_printf("Nonce: %d\n", txData.nonce);
                  res_ = true;
                } else {
                  isGetParameters_ = true;
                }
                // -- 2nd stage: create and send extrinsic
                if (res_) {
                  if (isGetParameters_) {
                    log_println("Try 2nd stage with extrinsic");
                    Data call = callDatalogRecord(head_dr_, record); // call header for Datalog record + some payload
                    printBytes("Сall size ", call);
                    edata_ = doExtrinsic (call, privateKey_, publicKey_, txData);
                    log_printf("Extrinsic %s: size %d\n", "DatalogRecord", edata_.size());
                    isGetParameters_ = false;
                  } else {
                    isGetParameters_ = true;
                    RpcResult r {"O.K", httpCode};
                    return r;
                  }
                } else {
                  isGetParameters_ = true;
                  if (a ==1) {
                    std::string rs = parseJsonResult(payload);
                    RpcResult r {"htpp O.K. " + rs, httpCode};
                    return r;
                  }
               }// res_
           } // json parse
        } else {
            isGetParameters_ = true;
            RpcResult r {"http not 200 error: ", httpCode};
            return r;
         } // httpCode == HTTP_CODE_OK
      } else {
        isGetParameters_ = true;
        RpcResult r {"http > 0 error: ", httpCode};
        return r;
      } // httpCode > 0
    } // for
    isGetParameters_ = true;
    RpcResult r {"http: ", HTTP_CODE_OK};
    return r; 
  };
  
  RpcResult TransferBalance (std::string dst, uint64_t fee) {

    Data edata_;

    for (int a = 0 ; a < 2;  a++) {

      HTTPClient http;    
      http.begin(wifi_, url_.c_str());
      http.addHeader("Content-Type", "application/json");
      log_printf("[HTTP]+POST:\n");
      JSONVar params; 
      String jsonString;
      if (isGetParameters_) {
        jsonString = getPayloadJs (ss58adr_,id_counter_);
      } else {
        jsonString = fillParamsJs (edata_,id_counter_);
        edata_.clear();
      }
      log_println("sent:");
      log_println(jsonString);
      id_counter_++;
    
      int httpCode = http.POST(jsonString);

      if (httpCode > 0) {
          log_printf("[HTTP]+POST code: %d\n", httpCode);
            if (httpCode == HTTP_CODE_OK) {
              const String& payload = http.getString();
              log_println("received:");
              log_println(payload);
         
              JSONVar myObject = JSON.parse(payload);
              if (JSON.typeof(myObject) == "undefined") {
                  log_println("");
                  RpcResult r {"Parsing input failed!", -100};
                  return r;
              } else {
                // RPC FSM
                bool res_ = false;
                TxData txData = {.ghash = ghash_, .bhash = bhash_, .specVersion = specVersion_, .tx_version = tx_version_};
                std::optional <int> parsedRes;
                if (isGetParameters_ && a == 0)
                  parsedRes = parseJsonResultInt(payload);
                if (parsedRes.has_value()) {
                  txData.nonce = parsedRes.value();
                  log_printf("Nonce: %d\n", txData.nonce);
                  res_ = true;
                } else {
                  isGetParameters_ = true;
                }
                // -- 2nd stage: create and send transfer balance extrinsic
                if (res_) {
                  if (isGetParameters_) {
                    log_println("Try 2nd stage with extrinsic");
                    Data call = callTransferBalance(head_bt_, dst, fee); // call header for Traansfer Balance + some payload
                    printBytes("Сall size ", call);
                    edata_ = doExtrinsic (call, privateKey_, publicKey_, txData);
                    log_printf("extrinsic %s: size %d\n", "TransferBalance", edata_.size());
                    isGetParameters_ = false;
                  } else {
                    isGetParameters_ = true;
                    RpcResult r {"O.K", httpCode};
                    return r;
                  }
                } else {
                  isGetParameters_ = true;
                  if (a ==1) {
                    std::string rs = parseJsonResult(payload);
                    RpcResult r {"htpp O.K. " + rs, httpCode};
                    return r;
                  }
               }// res_
           } // json parse
        } else {
            isGetParameters_ = true;
            RpcResult r {"http not 200 error: ", httpCode};
            return r;
         } // httpCode == HTTP_CODE_OK
      } else {
        isGetParameters_ = true;
        RpcResult r {"http > 0 error: ", httpCode};
        return r;
      } // httpCode > 0
    } // for
    isGetParameters_ = true;
    RpcResult r {"http: ", HTTP_CODE_OK};
    return r; 
  };

    RpcResult RwsDatalogRecord (std::string ownerKey, std::string dataSubmit) { // AccountId32, call -> Launch (AccountID32 robot, H256 param) 

    Data edata_;
    
    for (int a = 0 ; a < 2;  a++) {

      HTTPClient http;    
      http.begin(wifi_, url_.c_str());
      http.addHeader("Content-Type", "application/json");
      log_printf("[HTTP]+POST:\n");
      JSONVar params; 
      String jsonString;
      if (isGetParameters_) {
        jsonString = getPayloadJs (ss58adr_,id_counter_);
      } else {
        jsonString = fillParamsJs (edata_,id_counter_);
        edata_.clear();
      }
      log_printf("sent: %s \n", jsonString.c_str());
      id_counter_++;
      int httpCode = http.POST(jsonString);

      if (httpCode > 0) {
          log_printf("[HTTP]+POST code: %d\n", httpCode);
            if (httpCode == HTTP_CODE_OK) {
              const String& payload = http.getString();
              log_printf("received: %s\n", payload.c_str());
              JSONVar myObject = JSON.parse(payload);
              if (JSON.typeof(myObject) == "undefined") {
                  log_println("");
                  RpcResult r {"Parsing input failed!", -100};
                  return r;               
              } else {
                // RPC FSM
                bool res_ = false;
                TxData txData = {.ghash = ghash_, .bhash = bhash_, .specVersion = specVersion_, .tx_version = tx_version_};
                std::optional <int> parsedRes;
                if (isGetParameters_ && a == 0)
                  parsedRes = parseJsonResultInt(payload);
                if (parsedRes.has_value()) {
                  txData.nonce = parsedRes.value();
                  log_printf("Nonce: %d\n", txData.nonce);
                  res_ = true;
                } else {
                  isGetParameters_ = true;
                }
                // -- 2nd stage: create and send extrinsic
                if (res_) {
                  if (isGetParameters_) {
                    log_println("Try 2nd stage with extrinsic");
                    // Prepare datalog record for nesting:  call header for Datalog record + some payload
                    Data call_nested =  callDatalogRecord(head_dr_, dataSubmit);
                    log_printf("Submitted datalog size %d: %s\n", dataSubmit.size(), dataSubmit.c_str());
                    printBytes("Сall Datalog size ", call_nested);
                    Data call = callRws(head_rws_, ownerKey, call_nested); // inject nested call into RWS call
                    printBytes("Сall size ", call);
                    edata_ = doExtrinsic (call, privateKey_, publicKey_, txData);
                    log_printf("Extrinsic %s: size %d\n", "RWS+DatalogRecord", edata_.size());
                    isGetParameters_ = false;
                  } else {
                    isGetParameters_ = true;
                    RpcResult r {"O.K", httpCode};
                    return r;
                  }
                } else {
                  isGetParameters_ = true;
                  if (a ==1) {
                    std::string rs = parseJsonResult(payload);
                    RpcResult r {"htpp O.K. " + rs, httpCode};
                    return r;
                  }
               }// res_
           } // json parse
        } else {
            isGetParameters_ = true;
            RpcResult r {"http not 200 error: ", httpCode};
            return r;
         } // httpCode == HTTP_CODE_OK
      } else {
        isGetParameters_ = true;
        RpcResult r {"http > 0 error: ", httpCode};
        return r;
      } // httpCode > 0
    } // for
    isGetParameters_ = true;
    RpcResult r {"http: ", HTTP_CODE_OK};
    return r; 
  };
    
  private:
    std::string url_;
    std::string ss58adr_;
    WiFiClient wifi_;
    bool isGetParameters_;
    uint8_t publicKey_[KEYS_SIZE];
    uint8_t privateKey_[KEYS_SIZE];
    uint64_t id_counter_;
    bool is_remote_url_; 
    Data head_rws_;         // call header for RWS
    Data head_dr_;          // call header for Datalog record
    Data head_bt_;          // call header for Balance transfer
    std::string bhash_;
    std::string ghash_;
    uint32_t specVersion_;
    uint32_t tx_version_;
};
