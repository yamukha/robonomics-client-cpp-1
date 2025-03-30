#include <vector>
#include <string>

#include <Arduino.h>
#include <EEPROM.h>
#include <Defines.h>
#include <RpcRobonomics.h>

#ifdef ESP32_MODEL
#include <WiFi.h>
#include <HTTPClient.h>
#else
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#endif

#include <WebRpc.h>

// WiFi credentials and private keys are hidden in Private.h file
// pub keys are derived on ctor of RobonomicsRpc
// need to have derived or generated PUB_OWNER_KEY, SS58_ADR, SS58_DEVICE_ADR
// i.e. by subkey inspect "some mnemonics ..." --network robonomics --scheme ed25519

//#define DUMMY_KEYS
#ifdef DUMMY_KEYS
#include <Private_dummy.h>
#else 
#include <Private.h>
#endif

#define MIN_URL_SIZE  7
#define READ_UART_TIMEOUT 3

uint64_t id_counter = 0; 
uint64_t coins_count = 100000;
RunTimeData rtd;
std::string adr58 = ""; //SS58_DEVICE_ADR;

size_t GetCRC (std::string url, size_t len) {
  uint64_t sum = 0;
  for (size_t i = 0; i < len; i++) {
    sum += url.c_str()[i];
  }
  return (size_t) sum % 256;
}

uint8_t urlSize = (uint8_t)strlen(URLRPC);
uint8_t urlCRC = (uint8_t)GetCRC(URLRPC,urlSize);
uint8_t newUrlCRC = 0;
uint8_t url_eeprom_offset = sizeof(urlCRC) + sizeof(urlSize); // start of URL string

bool new_eeprom_args = false;
bool use_eeprom_args = false;
std::string robonomics_url = URLRPC;
char epprom_data[512];

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\nAwaiting new URL for " + String(READ_UART_TIMEOUT) + " s");

  delay(READ_UART_TIMEOUT * 1000); // Wait input from serial port for new robonomics URL
  String readUrl = Serial.readString();
  readUrl.trim();
  std::string newUrl = readUrl.c_str();
  uint8_t newUrlSize = (uint8_t)strlen(newUrl.c_str());
    
  if (newUrlSize > MIN_URL_SIZE) {
      newUrlCRC = (uint8_t)GetCRC(newUrl.c_str(),newUrlSize);
      new_eeprom_args = true;
  } else {
      Serial.println("Fail to read new URL. It's size is shorter then " + String(MIN_URL_SIZE) + ". Will be used old one!");
  }

  EEPROM.begin(512);

  if (!new_eeprom_args) {
      EEPROM.get(0, epprom_data);
      uint8_t url_eeprom_size = epprom_data[0];
      uint8_t url_eeprom_crc  = epprom_data[1];
  
      Serial.println("EPPROM URL size: " + String(url_eeprom_size) + " CRC: " + String(url_eeprom_crc));
  
      char tmp_data[url_eeprom_size];
      uint8_t j = 0;
      for (int i = url_eeprom_offset; i <= url_eeprom_size + url_eeprom_offset; i++, j++) {
          tmp_data[j]= epprom_data[i];
      }
      std::string url_eeprom_str = tmp_data;
      uint8_t calculated_crc = (uint8_t)GetCRC(url_eeprom_str,url_eeprom_size);
      Serial.println("Calculated size: " + String(strlen(url_eeprom_str.c_str())) + " CRC: " + String(calculated_crc) );
  
      if (calculated_crc == url_eeprom_crc && (uint8_t)strlen(url_eeprom_str.c_str()) == url_eeprom_size){
          Serial.println("EEPROM data OK!");
          Serial.println("robonomics url to connect: " + String(url_eeprom_str.c_str()));
          robonomics_url = url_eeprom_str;
          use_eeprom_args = true;
      } else {
          Serial.println("Wrong CRC for url!");
      }
  }

  if (!use_eeprom_args || new_eeprom_args) {
      std::string urlRPC = "";
      if (!new_eeprom_args){
          Serial.println("No EEPROM data! Set up default.");
          urlRPC = URLRPC;
      } else {
          Serial.println("Write new URL to EEPROM!");
          urlRPC = newUrl;
          urlSize = newUrlSize;
          urlCRC = newUrlCRC;
      }
      Serial.println("New URL: " + String(urlRPC.c_str()));
      Serial.println("New URL size: " + String(urlSize));
      Serial.println("New URL CRC : " + String(urlCRC));

      EEPROM.put(0, urlSize);
      EEPROM.commit();
      EEPROM.put(1, urlCRC);
      EEPROM.commit();

      char urlBytes[urlSize + 1];
      strncpy(urlBytes, urlRPC.c_str(), urlSize);
      urlBytes[urlSize] = '\0';
      for (int i = 0; i <= urlSize; i++) {
           EEPROM.write(url_eeprom_offset + i, urlBytes[i]);
      }

      EEPROM.commit();
      use_eeprom_args = true;
      robonomics_url = urlRPC.c_str();
  }
 
  WiFi.begin(STASSID, STAPSK);
  Serial.printf ("Trying to connected to SSID: %s \n", STASSID);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nConnected and get IP address: ");
  Serial.println(WiFi.localIP());
  delay(5000);

}

void loop () {
    if ((WiFi.status() == WL_CONNECTED)) {
        WiFiClient client;

        //  Get runtime parameters by RPC methods:
        // - state_getRuntimeVersion to get specVersion and transactionVersion
        // - chain_getBlockHash and/or chain_getHead to get genesis_hash
        // - pass to changed ctor in RunTimeData struct
        // - derive address from PRIV_KEY (over bub key)

        if (adr58 == "")
           adr58 = doAddress(PRIV_DEVICE_KEY);
        if (!rtd.hasHash) { // && rtd.hasRunTimeData) {
          rtd = rpcGetRT(client, robonomics_url, "state_getRuntimeVersion", "", id_counter);
          id_counter++;
          // std::string bhash = rpcGet(client, robonomics_url, "chain_getBlockHash", "", id_counter); // "" for the last block
          // rtd.bhash = bhash;
          // id_counter++;
          std::string ghash = rpcGet(client, robonomics_url, "chain_getBlockHash", "0", id_counter);;
          rtd.ghash = ghash;
          rtd.bhash = ghash;
          id_counter++;
          rtd.hasHash = true;
        }

#define RWS_EXTRINSIC
#define DATALOG_EXTRINSIC

#ifndef RWS_EXTRINSIC
#ifdef DATALOG_EXTRINSIC
        Serial.printf("\n[RPC]: Datalog task run\n");
        RobonomicsRpc rpcProvider(client, robonomics_url, PRIV_KEY, SS58_ADR, id_counter, rtd);
        RpcResult r = rpcProvider.DatalogRecord(std::to_string(id_counter)); // id_counter as payload just for example
#else
        Serial.printf("\n[RPC]: TX balance task run\n");
        RobonomicsRpc rpcProvider(client, robonomics_url, PRIV_KEY, SS58_ADR, id_counter, rtd);
        RpcResult r = rpcProvider.TransferBalance(PUB_OWNER_KEY, coins_count); // coins_count as fee just for example
#endif
#else
        Serial.printf("\n[RPC]: RWS task run\n");
        RobonomicsRpc rpcProvider(client, robonomics_url, PRIV_KEY, adr58, id_counter, rtd);
        //RobonomicsRpc rpcProvider(client, robonomics_url, PRIV_KEY, SS58_DEVICE_ADR, id_counter, rtd);
        std::string payload = R"({"SDS_P1":11.45,"SDS_P2":7.50,"noiseMax":48.0,"noiseAvg":47.55,"temperature":20.95,"press":687.97,"humidity":62.5,"lat":0.000000,"lon":0.00000})"; // max len < 144 
        std::string payloads = R"({temperature:20.9,pressure:687.97,humidity:62.5,p1:11.45,p2:7.50,nm:48.00,na:47.55})";
        std::string payloadx = payload + payload + payload + payloads; //  512 bytes: max size of record
        RpcResult r = rpcProvider.RwsDatalogRecord(PUB_OWNER_KEY, payloadx);
#endif
        coins_count += 10000;
        id_counter = id_counter + 2;
        Serial.printf("[RPC] %ld %s\n", r.code, r.body.c_str());  
        delay(12000);
    }
}
