#pragma once 

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include <Extrinsic.h>
#include <Call.h>
#include <JsonUtils.h>
#include <Defines.h>

typedef struct {
   std::string body;      // responce body
   uint32_t code;         // http responce code 200, 404, 500 etc.
} RpcResult;

class RobonomicsRpc { 
  public:     
    RobonomicsRpc (WiFiClient client, std::string url, std::string key, std::string ss58adr, uint64_t id)
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

          ghash_ = getBlockHash(is_remote_url_);
          bhash_ = getBlockHash(is_remote_url_);

          Serial.printf("Node mode: %s, genesis block hash: 0x%s\n", is_remote_url_?"remote":"local", ghash_.c_str());

        };

    RpcResult DatalogRecord (std::string record) {

    Data edata_;
    
    for (int a = 0 ; a < 2;  a++) {

      HTTPClient http;    
      http.begin(wifi_, url_.c_str());
      http.addHeader("Content-Type", "application/json");
      Serial.print("[HTTP]+POST:\n"); 
      JSONVar params; 
      String jsonString;
      if (isGetParameters_) {
        jsonString = getPayloadJs (ss58adr_,id_counter_);
      } else {
        jsonString = fillParamsJs (edata_,id_counter_);
        edata_.clear();
      }
      Serial.println("sent:");
      Serial.println(jsonString);
      id_counter_++;
    
      int httpCode = http.POST(jsonString);

      if (httpCode > 0) {
          Serial.printf("[HTTP]+POST code: %d\n", httpCode);
            if (httpCode == HTTP_CODE_OK) {
              const String& payload = http.getString();
              Serial.println("received:");
              Serial.println(payload);
         
              JSONVar myObject = JSON.parse(payload);
              if (JSON.typeof(myObject) == "undefined") {
                  Serial.println("");
                  RpcResult r {"Parsing input failed!", -100};
                  return r;               
              } else {
                // RPC FSM                 
                JSONVar keys = myObject.keys();
                bool res_ = false;
                JSONVar val;
                FromJson fj;
                  
                //"result" or "error" 
                for (int i = 0; i < keys.length(); i++) { 
                  JSONVar value = myObject[keys[i]];
                  String str  = JSON.stringify (keys[i]);
                 
                  if(strstr(str.c_str(),"result")) {
                    res_ = true;
                    val = value;
                  }
       
                  if(strstr(str.c_str(),"error"))  {
                    val = value;
                    isGetParameters_ = true;
                  }
                }
                
                // -- 2nd stage: create and send extrinsic
                if (res_) {
                  Serial.println("Try 2nd stage with datalog extrinsic"); 
                  if (isGetParameters_) {
                    fj = parseJson (val);
                    fj.ghash = ghash_;
                    fj.bhash = bhash_;
                    fj.nonce = (uint64_t) decodeU32(fj.nonce, false);
                    Data call = callDatalogRecord(head_dr_, record); // call header for Datalog record + some payload
#ifdef DEBUG_PRINT
                    Serial.println("Nonce decoded: " + String(fj.nonce));
                    Serial.printf("Сall size %d : \n", call.size());
                    for (int k = 0; k < call.size(); k++) 
                       Serial.printf("%02x", call[k]);
#endif
                    Data data_ = doPayload (call, fj.era, fj.nonce, fj.tip, fj.specVersion, fj.tx_version, fj.ghash, fj.bhash);
#ifdef DEBUG_PRINT
                    Serial.printf("\nPayload size %d : \n", data_.size());
                    for (int k = 0; k < data_.size(); k++) 
                       Serial.printf("%02x", data_[k]);
#endif
                    Data signature_ = doSign (data_, privateKey_, publicKey_);
#ifdef DEBUG_PRINT
                    Serial.printf("\nSignatured size %d : \n", signature_.size());
                    for (int k = 0; k < signature_.size(); k++) 
                       Serial.printf("%02x", signature_[k]);
                    Serial.println("");
#endif
                    std::vector<std::uint8_t> pubKey( reinterpret_cast<std::uint8_t*>(std::begin(publicKey_)), reinterpret_cast<std::uint8_t*>(std::end(publicKey_)));
                    edata_ = doEncode (signature_, pubKey, fj.era, fj.nonce, fj.tip, call);
                    Serial.printf("Extrinsic %s: size %d\n", "DatalogRecord", edata_.size());
                    isGetParameters_ = false;
                  } else {
                    isGetParameters_ = true;
                    RpcResult r {"O.K", httpCode};
                    return r;
                  }
                } else {
                  isGetParameters_ = true;
                  RpcResult r {"htpp O.K. but RPC error ", httpCode};
                  return r;
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
      Serial.print("[HTTP]+POST:\n"); 
      JSONVar params; 
      String jsonString;
      if (isGetParameters_) {
        jsonString = getPayloadJs (ss58adr_,id_counter_);
      } else {
        jsonString = fillParamsJs (edata_,id_counter_);
        edata_.clear();
      }
      Serial.println("sent:");
      Serial.println(jsonString);
      id_counter_++;
    
      int httpCode = http.POST(jsonString);

      if (httpCode > 0) {
          Serial.printf("[HTTP]+POST code: %d\n", httpCode);
            if (httpCode == HTTP_CODE_OK) {
              const String& payload = http.getString();
              Serial.println("received:");
              Serial.println(payload);
         
              JSONVar myObject = JSON.parse(payload);
              if (JSON.typeof(myObject) == "undefined") {
                  Serial.println("");
                  RpcResult r {"Parsing input failed!", -100};
                  return r;               
              } else {
                // RPC FSM                 
                JSONVar keys = myObject.keys();
                bool res_ = false;
                JSONVar val;
                FromJson fj;
                  
                //"result" or "error" 
                for (int i = 0; i < keys.length(); i++) { 
                  JSONVar value = myObject[keys[i]];
                  String str  = JSON.stringify (keys[i]);
                 
                  if(strstr(str.c_str(),"result")) {
                    res_ = true;
                    val = value;
                  }
       
                  if(strstr(str.c_str(),"error"))  {
                    val = value;
                    isGetParameters_ = true;
                  }
                }
                
                // -- 2nd stage: create and send transfer balance extrinsic
                if (res_) {
                  Serial.println("Try 2nd stage with extrinsic"); 
                  if (isGetParameters_) {
                    fj = parseJson (val);
                    fj.ghash = ghash_;
                    fj.bhash = bhash_;
                    fj.nonce = (uint64_t) decodeU32(fj.nonce, false);
                    Serial.println("Nonce decoded: " + String(fj.nonce));
                    Data call = callTransferBalance(head_bt_, dst, fee); // call header for Traansfer Balance + some payload
                    Data data_ = doPayload (call, fj.era, fj.nonce, fj.tip, fj.specVersion, fj.tx_version, fj.ghash, fj.bhash);
                    Data signature_ = doSign (data_, privateKey_, publicKey_);
                    std::vector<std::uint8_t> pubKey( reinterpret_cast<std::uint8_t*>(std::begin(publicKey_)), reinterpret_cast<std::uint8_t*>(std::end(publicKey_)));               
                    edata_ = doEncode (signature_, pubKey, fj.era, fj.nonce, fj.tip, call);
                    Serial.printf("extrinsic %s: size %d\n", "TransferBalance", edata_.size());
                    isGetParameters_ = false;
                  } else {
                    isGetParameters_ = true;
                    RpcResult r {"O.K", httpCode};
                    return r;
                  }
                } else {
                  isGetParameters_ = true;
                  RpcResult r {"htpp O.K. but RPC error ", httpCode};
                  return r;
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
      Serial.print("[HTTP]+POST:\n"); 
      JSONVar params; 
      String jsonString;
      if (isGetParameters_) {
        jsonString = getPayloadJs (ss58adr_,id_counter_);
      } else {
        jsonString = fillParamsJs (edata_,id_counter_);
        edata_.clear();
      }
      Serial.println("sent:");
      Serial.println(jsonString);
      id_counter_++;
    
      int httpCode = http.POST(jsonString);

      if (httpCode > 0) {
          Serial.printf("[HTTP]+POST code: %d\n", httpCode);
            if (httpCode == HTTP_CODE_OK) {
              const String& payload = http.getString();
              Serial.println("received:");
              Serial.println(payload);
         
              JSONVar myObject = JSON.parse(payload);
              if (JSON.typeof(myObject) == "undefined") {
                  Serial.println("");
                  RpcResult r {"Parsing input failed!", -100};
                  return r;               
              } else {
                // RPC FSM                 
                JSONVar keys = myObject.keys();
                bool res_ = false;
                JSONVar val;
                FromJson fj;
                  
                //"result" or "error" 
                for (int i = 0; i < keys.length(); i++) { 
                  JSONVar value = myObject[keys[i]];
                  String str  = JSON.stringify (keys[i]);
                 
                  if(strstr(str.c_str(),"result")) {
                    res_ = true;
                    val = value;
                  }
       
                  if(strstr(str.c_str(),"error"))  {
                    val = value;
                    isGetParameters_ = true;
                  }
                }
                
                // -- 2nd stage: create and send extrinsic
                if (res_) {
                  Serial.println("Try 2nd stage with RWS extrinsic"); 
                  if (isGetParameters_) {
                    fj = parseJson (val);
                    fj.ghash = ghash_;
                    fj.bhash = bhash_;
                    fj.nonce = (uint64_t) decodeU32(fj.nonce, false);
                    // Prepare datalog record for nesting:  call header for Datalog record + some payload
                    Data call_nested =  callDatalogRecord(head_dr_, dataSubmit);
                    Data call = callRws(head_rws_, ownerKey, call_nested); // inject nested call into RWS call
#ifdef DEBUG_PRINT
                    Serial.println("Nonce decoded: " + String(fj.nonce));
                    Serial.printf("Сall size %d : \n", call.size());
                    for (int k = 0; k < call.size(); k++) 
                       Serial.printf("%02x", call[k]);
#endif
                    Data data_ = doPayload (call, fj.era, fj.nonce, fj.tip, fj.specVersion, fj.tx_version, fj.ghash, fj.bhash);
#ifdef DEBUG_PRINT
                    Serial.printf("\nPayload size %d : \n", data_.size());
                    for (int k = 0; k < data_.size(); k++) 
                       Serial.printf("%02x", data_[k]);
#endif
                    Data signature_ = doSign (data_, privateKey_, publicKey_);
#ifdef DEBUG_PRINT
                    Serial.printf("\nSignatured size %d : \n", signature_.size());
                    for (int k = 0; k < signature_.size(); k++) 
                       Serial.printf("%02x", signature_[k]);
                    Serial.println("");
#endif
                    std::vector<std::uint8_t> pubKey( reinterpret_cast<std::uint8_t*>(std::begin(publicKey_)), reinterpret_cast<std::uint8_t*>(std::end(publicKey_)));
                    edata_ = doEncode (signature_, pubKey, fj.era, fj.nonce, fj.tip, call);
                    Serial.printf("Extrinsic %s: size %d\n", "RWS+DatalogRecord", edata_.size());
                    isGetParameters_ = false;
                  } else {
                    isGetParameters_ = true;
                    RpcResult r {"O.K", httpCode};
                    return r;
                  }
                } else {
                  isGetParameters_ = true;
                  RpcResult r {"htpp O.K. but RPC error ", httpCode};
                  return r;
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
};
