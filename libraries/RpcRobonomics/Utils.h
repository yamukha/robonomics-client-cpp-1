#pragma once

#include <vector>
#include <string>
#include <cstring>

#include "Defines.h"

std::vector<uint8_t> hex2bytes (std::string hex) {
    std::vector<uint8_t> bytes;
    for (unsigned int i = 0; i < hex.length(); i += 2) {
      std::string byteString = hex.substr(i, 2);
      uint8_t byte = (uint8_t) strtol(byteString.c_str(), NULL, 16);
      bytes.push_back(byte);
    }
    return bytes;
}

std::string swapEndian(std::string str) {
    std::string hex = str.c_str();
    std::string bytes;
    for (unsigned int i = 0; i < hex.length(); i += 2) {
      std::string s = hex.substr(i, 2);
      if ( !(strstr(s.c_str(),"0x")) && !(strstr(s.c_str(),"0X")) ) {
        std::string byteString = hex.substr(i, 2);
        bytes.insert(0,byteString);
      }
    }
    return bytes;
}

bool getTypeUrl(std::string url) {
    if (url.find("http://kusama.rpc.robonomics.network/rpc/") != std::string::npos)
      return true;
    return false;
}

// {"jsonrpc":"2.0","id":1,"method":"chain_getBlockHash","params":[0]}'
std::string getBlockHash (bool is_remote) {
    if (is_remote)
      return "631ccc82a078481584041656af292834e1ae6daab61d2875b4dd0c14bb9b17bc";
    else 
      // return "60ac92592dd51574a8bfa8094deae863f2e2ffe86889bef466d9768c625467ed";
      return "368d43155203b23de832b622b96172a5e5e8143a19342a240974c5c8394662bc";

}

void printBytes(std::string str, Data &bytes) {
  debug_printf("%s %lu: 0x", str.c_str(), bytes.size());
  for (int k = 0; k < bytes.size(); k++)
    debug_printf("%02x", bytes[k]);
  debug_printf("\n");
}

void printBytes(std::string str,  uint8_t bytes[], uint16_t len) {
  debug_printf("%s %u: 0x", str.c_str(), len);
  for (int k = 0; k < len; k++)
    debug_printf("%02x", bytes[k]);
  debug_printf("\n");
}

std::string toHexString (Data &data) {
    std::string str;
    char ch [3];
    for (int i = 0; i < data.size();i++) {
        sprintf(ch,"%02x",data[i]);
        str.append(ch);
    }
    return str;
}

const char *const ALPHABET = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
const int ALPHABET_MAP[128] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0,  1,  2,
    3,  4,  5,  6,  7,  8,  -1, -1, -1, -1, -1, -1, -1, 9,  10, 11, 12, 13, 14, 15, 16, -1, 17, 18, 19, 20,
    21, -1, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, -1, -1, -1, -1, -1, -1, 33, 34, 35, 36, 37, 38, 39,
    40, 41, 42, 43, -1, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, -1, -1, -1, -1, -1};

// result must be declared: char result[len * 137 / 100];
int EncodeBase58(const unsigned char *bytes, int len, unsigned char result[]) {
    unsigned char digits[512] = {0};
    int digitslen = 1;
    for (int i = 0; i < len; i++) {
        unsigned int carry = (unsigned int)bytes[i];
        for (int j = 0; j < digitslen; j++) {
            carry += (unsigned int)(digits[j]) << 8;
            digits[j] = (unsigned char)(carry % 58);
            carry /= 58;
        }
        while (carry > 0) {
            digits[digitslen++] = (unsigned char)(carry % 58);
            carry /= 58;
        }
    }
    int resultlen = 0;
    for (; resultlen < len && bytes[resultlen] == 0;)
        result[resultlen++] = '1';
    for (int i = 0; i < digitslen; i++)
        result[resultlen + i] = ALPHABET[digits[digitslen - 1 - i]];
    result[digitslen + resultlen] = 0;
    return digitslen + resultlen;
}

