#include  "../libraries/RpcRobonomics/Data.h"
#include  "../libraries/RpcRobonomics/Call.h"
#include  "../libraries/RpcRobonomics/Extrinsic.h"
#include  "../libraries/RpcRobonomics/Defines.h"

// git clone https://github.com/weidai11/cryptopp
// cd cryptopp
// make libcryptopp.a libcryptopp.so
// sudo make install PREFIX=/usr/local
// sudo ldconfig
// 
// g++ test_call.c -o test_call -DUNIT_TEST -L/usr/local/lib -lcryptopp
// g++ test_call.c -o test_call -DUNIT_TEST -L/usr/local/lib -l:libcryptopp.a

#include "cryptopp/xed25519.h"
#include "cryptopp/osrng.h"

#include <iostream>
#include <cassert>

#define PRIVKEY      "da3cf5b1e9144931a0f0db65664aab662673b099415a7f8121b7245fb0be4143"

void printBytes (Data data) {
   for (auto val : data) 
      printf("%.2x", val);
   printf("\n");   
}

void printCharArray (uint8_t data[], uint8_t size) {
  for (int i = 0; i < size; i++) 
      printf(" %.2x", data [i]);
  printf("\n");
}

void compareBytes (Data data, Data pattern) {
  assert(std::equal(std::begin(data), std::end(data), std::begin(pattern)) && "Bytes vector is not equal to expected pattern");
}

void derivePubKey (uint8_t  privateKey[KEYS_SIZE], uint8_t publicKey[KEYS_SIZE]) {
   using namespace CryptoPP;

   ed25519::Signer signer = ed25519Signer (privateKey);
   ed25519::Verifier verifier(signer);

   const ed25519PublicKey& pubKey = dynamic_cast<const ed25519PublicKey&>(verifier.GetPublicKey());
   auto pt = pubKey.GetPublicKeyBytePtr();

   std::memcpy(publicKey, pt, KEYS_SIZE);
}

void test_callDatalogRecord() {
  auto record = "42";
  Data head = Data{0x33,0};
  Data call = callDatalogRecord(head, record);

  Data callPattern = Data{0x33,0,8,0x34,0x32};
  assert(std::equal(std::begin(call), std::end(call), std::begin(callPattern)) && "Bytes vector is not equal to expected pattern");
}

void test_doPayload() {
    // doPayload (Data, uint32_t, uint64_t, uint64_t, uint32_t, uint32_t, std::string, std::string) 
   auto record = "42";
   Data head = Data {0x33,0};
   Data call = callDatalogRecord(head,record);
   uint32_t era =  0;
   uint64_t nonce = 0;
   uint64_t tip = 0;
   uint32_t specVersion =  0x17;
   uint32_t txVersion = 1;
   std::string ghash = "631ccc82a078481584041656af292834e1ae6daab61d2875b4dd0c14bb9b17bc";
   std::string bhash = "631ccc82a078481584041656af292834e1ae6daab61d2875b4dd0c14bb9b17bc";

   Data data = doPayload (call, era, nonce, tip, specVersion, txVersion, ghash, bhash);

   Data payloadPattern = Data {
        0x33, 0x00, 0x08, 0x34, 0x32, 0x00, 0x00, 0x00, 
        0x17, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x63, 0x1c, 0xcc, 0x82, 0xa0, 0x78, 0x48, 0x15, 
        0x84, 0x04, 0x16, 0x56, 0xaf, 0x29, 0x28, 0x34,
        0xe1, 0xae, 0x6d, 0xaa, 0xb6, 0x1d, 0x28, 0x75, 
        0xb4, 0xdd, 0x0c, 0x14, 0xbb, 0x9b, 0x17, 0xbc,
        0x63, 0x1c, 0xcc, 0x82, 0xa0, 0x78, 0x48, 0x15, 
        0x84, 0x04, 0x16, 0x56, 0xaf, 0x29, 0x28, 0x34,
        0xe1, 0xae, 0x6d, 0xaa, 0xb6, 0x1d, 0x28, 0x75, 
        0xb4, 0xdd, 0x0c, 0x14, 0xbb, 0x9b, 0x17, 0xbc
      };

  assert(std::equal(std::begin(data), std::end(data), std::begin(payloadPattern)) && "Bytes vector is not equal to expected pattern");
}

void test_doSign () {
   //doSign(Data, uint8_t [32], uint8_t [32])

Data data = Data {
        0x33, 0x00, 0x08, 0x34, 0x32, 0x00, 0x00, 0x00, 
        0x17, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x63, 0x1c, 0xcc, 0x82, 0xa0, 0x78, 0x48, 0x15, 
        0x84, 0x04, 0x16, 0x56, 0xaf, 0x29, 0x28, 0x34,
        0xe1, 0xae, 0x6d, 0xaa, 0xb6, 0x1d, 0x28, 0x75, 
        0xb4, 0xdd, 0x0c, 0x14, 0xbb, 0x9b, 0x17, 0xbc,
        0x63, 0x1c, 0xcc, 0x82, 0xa0, 0x78, 0x48, 0x15, 
        0x84, 0x04, 0x16, 0x56, 0xaf, 0x29, 0x28, 0x34,
        0xe1, 0xae, 0x6d, 0xaa, 0xb6, 0x1d, 0x28, 0x75, 
        0xb4, 0xdd, 0x0c, 0x14, 0xbb, 0x9b, 0x17, 0xbc
      };

   uint8_t publicKey[KEYS_SIZE];
   uint8_t privateKey[KEYS_SIZE];
   
   std::vector<uint8_t> vk = hex2bytes(PRIVKEY);
   std::copy(vk.begin(), vk.end(), privateKey);

   derivePubKey(privateKey, publicKey);
    
   Data signature = doSign (data, privateKey, publicKey);

   Data signaturePattern = Data {
    0x68, 0xd4, 0xd0, 0x1a, 0x5d, 0xd9, 0x8e, 0xbc,
    0xa8, 0xa7, 0x93, 0x15, 0x06, 0x93, 0x8b, 0x6f,
    0x7c, 0x79, 0xab, 0x1b, 0x6b, 0x27, 0x03, 0x60,
    0xfb, 0x28, 0x6c, 0xd4, 0x9d, 0x54, 0xce, 0x69, 
    0x1c, 0xeb, 0xf6, 0x07, 0x0f, 0x02, 0x6c, 0xcf,
    0x78, 0xd8, 0x9d, 0xfd, 0xf6, 0x01, 0xef, 0xc8,
    0xf4, 0x90, 0xce, 0xc4, 0x56, 0x0a, 0xfb, 0x9b,
    0xcf, 0x04, 0x35, 0x11, 0xa0, 0x93, 0xa6, 0x0d
  };

  assert(std::equal(std::begin(signature), std::end(signature), std::begin(signaturePattern)) && "Bytes vector is not equal to expected pattern");
}

void test_doEncode() {
    //doEncode (Data, Data, uint32_t, uint64_t, uint64_t, Data)
    Data signature = Data {
    0x68, 0xd4, 0xd0, 0x1a, 0x5d, 0xd9, 0x8e, 0xbc,
    0xa8, 0xa7, 0x93, 0x15, 0x06, 0x93, 0x8b, 0x6f,
    0x7c, 0x79, 0xab, 0x1b, 0x6b, 0x27, 0x03, 0x60,
    0xfb, 0x28, 0x6c, 0xd4, 0x9d, 0x54, 0xce, 0x69, 
    0x1c, 0xeb, 0xf6, 0x07, 0x0f, 0x02, 0x6c, 0xcf,
    0x78, 0xd8, 0x9d, 0xfd, 0xf6, 0x01, 0xef, 0xc8,
    0xf4, 0x90, 0xce, 0xc4, 0x56, 0x0a, 0xfb, 0x9b,
    0xcf, 0x04, 0x35, 0x11, 0xa0, 0x93, 0xa6, 0x0d
  };
     
    uint32_t era =  0;
    uint64_t nonce = 0;
    uint64_t tip = 0;
    auto record = "42";

    uint8_t publicKey[KEYS_SIZE];
    uint8_t privateKey[KEYS_SIZE];
   
    std::vector<uint8_t> vk = hex2bytes(PRIVKEY);
    std::copy(vk.begin(), vk.end(), privateKey);
    derivePubKey(privateKey, publicKey);
    std::vector<std::uint8_t> pubKeyVector( reinterpret_cast<std::uint8_t*>(std::begin(publicKey)), reinterpret_cast<std::uint8_t*>(std::end(publicKey)));

    Data head = Data{0x33,0};
    Data call = callDatalogRecord(head, record);
    
    Data edata = doEncode (signature, pubKeyVector, era, nonce, tip, call);

    Data edataPattern = Data {
      0xad, 0x01, 0x84, 0x00, 0xf9, 0x0b, 0xc7, 0x12,
      0xb5, 0xf2, 0x86, 0x40, 0x51, 0x35, 0x31, 0x77,
      0xa9, 0xd6, 0x27, 0x60, 0x5d, 0x4b, 0xf7, 0xec,
      0x36, 0xc7, 0xdf, 0x56, 0x8c, 0xfd, 0xce, 0xa9,
      0xf2, 0x37, 0xc1, 0x85, 0x00, 0x68, 0xd4, 0xd0,
      0x1a, 0x5d, 0xd9, 0x8e, 0xbc, 0xa8, 0xa7, 0x93,
      0x15, 0x06, 0x93, 0x8b, 0x6f, 0x7c, 0x79, 0xab,
      0x1b, 0x6b, 0x27, 0x03, 0x60, 0xfb, 0x28, 0x6c,
      0xd4, 0x9d, 0x54, 0xce, 0x69, 0x1c, 0xeb, 0xf6,
      0x07, 0x0f, 0x02, 0x6c, 0xcf, 0x78, 0xd8, 0x9d,
      0xfd, 0xf6, 0x01, 0xef, 0xc8, 0xf4, 0x90, 0xce,
      0xc4, 0x56, 0x0a, 0xfb, 0x9b, 0xcf, 0x04, 0x35,
      0x11, 0xa0, 0x93, 0xa6, 0x0d, 0x00, 0x00, 0x00,
      0x33, 0x00, 0x08, 0x34, 0x32
    };
    
    assert(std::equal(std::begin(edata), std::end(edata), std::begin(edataPattern)) && "Bytes vector is not equal to expected pattern");
}

// based on number pairs from https://github.com/qdrvm/scale-codec-cpp/blob/master/test/scale_compact_test.cpp
void test_DecodeU32() {
  // test 1 byte encoded mode: swap bytes flag is false
  uint64_t decode_0 = decodeU32 (0, false);
  assert (decode_0 == 0 && "Scale compact decoder error for 0");

  uint64_t decode_1 = decodeU32 (4, false);
  assert (decode_1 == 1 && "Scale compact decoder error for 1");

  uint64_t decode_63 = decodeU32 (252, false);
  assert (decode_63 == 63 && "Scale compact decoder error for 63");

  // test 2 bytes encoded mode: swap bytes flag is false
  uint64_t decode_64 = decodeU32 (0x0101, false);
  assert (decode_64 == 64 && "Scale compact decoder error for 64");

  uint64_t decode_255 = decodeU32 (0x03fd, false);
  assert (decode_255 == 255 && "Scale compact decoder error for 255");

  uint64_t decode_511 = decodeU32 (0x07fd, false);
  assert (decode_511 == 511 && "Scale compact decoder error for 511");

  uint64_t decode_16383 = decodeU32 (0xfffd, false);
  assert (decode_16383 == 16383 && "Scale compact decoder error for 16383");

  // test 4 bytes encoded mode: swap bytes flag is false
  uint64_t decode_16384 = decodeU32 (0x00010002, false);
  assert (decode_16384 == 16384 && "Scale compact decoder error for 16384");

  uint64_t decode_65535 = decodeU32 (0x0003fffe, false);
  assert (decode_65535 == 65535 && "Scale compact decoder error for 65535");

  uint64_t decode_max4bytes = decodeU32 (0xfffffffe, false);
  assert (decode_max4bytes == 1073741823ul && "Scale compact decoder error for 1073741823");


  // test 1 byte encoded mode with reversed bytes order: swap bytes flag is true
  uint64_t decode_0Rev = decodeU32 (0, true);
  assert (decode_0Rev == 0 && "Scale compact decoder error for swapped 0");

  uint64_t decode_1Rev = decodeU32 (4, true);
  assert (decode_1Rev == 1 && "Scale compact decoder error for swapped 1");

  uint64_t decode_63Rev = decodeU32 (252, true);
  assert (decode_63Rev == 63 && "Scale compact decoder error for swapped 63");

  // test 2 bytes encoded mode with reversed bytes order: swap bytes flag is true
  uint64_t decode_64Rev = decodeU32 (0x0101, true);
  assert (decode_64Rev == 64 && "Scale compact decoder error for swapped 64");

  uint64_t decode_255Rev = decodeU32 (0xfd03, true);
  assert (decode_255Rev == 255 && "Scale compact decoder error for swapped 255");

  uint64_t decode_511Rev = decodeU32 (0xfd07, true);
  assert (decode_511Rev == 511 && "Scale compact decoder error for swapped 511");

  uint64_t decode_16383Rev = decodeU32 (0xfdff, true);
  assert (decode_16383Rev == 16383 && "Scale compact decoder error for swapped 16383");

 // test 4 bytes encoded mode with reversed bytes order: swap bytes flag is true
  uint64_t decode_16384Rev = decodeU32 (0x02000100, true);
  assert (decode_16384Rev == 16384 && "Scale compact decoder error for swapped 16384");

  uint64_t decode_65535Rev = decodeU32 (0xfeff0300, true);
  assert (decode_65535Rev == 65535 && "Scale compact decoder error for swapped 65535");

  uint64_t decode_max4bytesRev = decodeU32 (0xfeffffff, true);
  assert (decode_max4bytesRev == 1073741823ul && "Scale compact decoder error for swapped 1073741823");


  // test BigInt mode: mode byte is equal 3. While it is not implemented 0 is returned.
  uint64_t decode_BigInt1 = decodeU32 (3, true);
  assert (decode_BigInt1 == 0 && "Scale compact decoder BigInt1 error for 1 byte wrong input");

  uint64_t decode_BigInt1Rev = decodeU32 (3, true);
  assert (decode_BigInt1Rev == 0 && "Scale compact decoder error for swapped 1 byte wrong input");

  uint64_t decode_BigInt2 = decodeU32 (0x0703, false);
  assert (decode_BigInt2 == 0 && "Scale compact decoder error for 2 bytes wrong input");

  uint64_t decode_BigInt2Rev = decodeU32 (0x0307, true);
  assert (decode_BigInt2Rev == 0 && "Scale compact decoder error for swapped 2 bytes wrong input");

  uint64_t decode_BigInt4 = decodeU32 (0xfffffff3, false);
  assert (decode_BigInt4 == 0 &&  "Scale compact decoder error for 4 bytes wrong input");

  uint64_t decode_BigInt4Rev = decodeU32 (0xffffffff, true);
  assert (decode_BigInt4Rev == 0 && "Scale compact decoder error for swapped 4 bytes wrong input");
}

int main () {
  test_callDatalogRecord();
  test_doPayload();
  test_doSign();
  test_doEncode();
  test_DecodeU32();
}
