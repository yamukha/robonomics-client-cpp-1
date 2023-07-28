#include <vector>
#include <string>

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <EEPROM.h>

#include <RpcRobonomics.h>

// ss58 address and related to it ED25519 keypair, can be examined by subkey application or python script
#define PRIV_KEY      "da3cf5b1e9144931a0f0db65664aab662673b099415a7f8121b7245fb0be4143"
#define SS58_ADR       "5HhFH9GvwCST4kRVoFREE7qDJcjYteR5unhQCrBGhhGuRgNb"
#define SS58_DST       "5FHneW46xGXgs5mUiveU4sbTyGBzmstUspZC92UhjJM694ty"
#define SS58_DST_KEY   "8eaf04151687736326c9fea17e25fc5287613693c912909cb226aa4794f26a48"

#ifndef STASSID
#define STASSID "FLY-TL-WR741ND"
#define STAPSK  "xxxxxxxxxx"
#endif

uint64_t id_counter = 0; 
uint64_t coins_count = 0; 

size_t GetCRC (std::string url, size_t len) {
  uint64_t sum = 0;
  for (size_t i = 0; i < len; i++) {
    sum += url.c_str()[i];
  }
  return (size_t) sum % 256;
}

uint8_t urlSize = (uint8_t)strlen(URLRPC);
uint8_t urlCRC = (uint8_t)GetCRC(URLRPC,urlSize);
uint8_t url_eeprom_offset = sizeof(urlCRC) + sizeof(urlSize); // start of URL string


bool use_eeprom_args = false;
std::string robonomics_url = URLRPC;
char epprom_data[512];

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println();
  
  EEPROM.begin(512);
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

  if (!use_eeprom_args) {
      Serial.println("No EEPROM data! Set up default.");
      Serial.println("New URL: " + String(URLRPC));
      Serial.println("New URL size: " + String(urlSize));
      Serial.println("New URL CRC : " + String(urlCRC));
      EEPROM.put(0, urlSize);
      EEPROM.commit();
      EEPROM.put(1, urlCRC);
      EEPROM.commit();
      EEPROM.put(url_eeprom_offset, URLRPC);
      EEPROM.commit();
      use_eeprom_args = true;
      robonomics_url = URLRPC;
  }
 
  WiFi.begin(STASSID, STAPSK);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.printf ("Connected to SSID %s own IP address \n", STASSID);
  Serial.println(WiFi.localIP());

}

void loop () {
    if ((WiFi.status() == WL_CONNECTED)) { 
        WiFiClient client;
        Serial.println("RPC task run");

        RobonomicsRpc rpcProvider(client, robonomics_url, PRIV_KEY, SS58_ADR, id_counter);
        // RpcResult r = rpcProvider.DatalogRecord(std::to_string(42)); // id_counter as payload just for example
        RpcResult r = rpcProvider.TransferBalance(SS58_DST_KEY, coins_count); // coins_count as fee just for example
        coins_count++;
        id_counter = id_counter + 2;
        Serial.printf("[RPC] %ld %s\n", r.code, r.body.c_str());  
        delay(1000);
    }
}
