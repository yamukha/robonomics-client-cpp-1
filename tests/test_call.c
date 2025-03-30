#ifdef  UNIT_TEST
#define USE_LINUX
#define NO_DEBUG_LOG_PRINT
#endif

#include  "../libraries/RpcRobonomics/Data.h"
#include  "../libraries/RpcRobonomics/Call.h"
#include  "../libraries/RpcRobonomics/Extrinsic.h"
#include  "../libraries/RpcRobonomics/Defines.h"
#include  "../libraries/RpcRobonomics/JsonUtils.h"

// #define DUMMY_KEYS
#ifdef DUMMY_KEYS
#include  "../libraries/RpcRobonomics/Private_dummy.h"
#else
#include  "../libraries/RpcRobonomics/Private.h"
#endif

// git clone https://github.com/weidai11/cryptopp
// cd cryptopp
// make libcryptopp.a libcryptopp.so
// sudo make install PREFIX=/usr/local
// sudo ldconfig
// 
// g++ test_call.c -o test_call -DUNIT_TEST -L/usr/local/lib -l:libcryptopp.a
// g++ test_call.c -o test_call -DUNIT_TEST -L/usr/local/lib -lcryptopp

// with standalone Blake2
// g++ test_call.c ../libraries/blake/Blake2b.cpp -o test_call -DUNIT_TEST -I../libraries/blake  -L/usr/local/lib -lcryptopp

#include "cryptopp/xed25519.h"
#include "cryptopp/osrng.h"

#include <iostream>
#include <cassert>

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

void test_doAddress () {
  // to check
  // subkey inspect 0xda3cf5b1e9144931a0f0db65664aab662673b099415a7f8121b7245fb0be4143 --network robonomics --scheme ed25519
  std::string addrStr = doAddress(PRIV_DEVICE_KEY);
  // std::cout <<  "Address size: "<< addrStr.size() << ", " << addrStr << std::endl;
  std::string addrExpected = "4HpehDru6HhBWANrjvczT7AxqmJcLp7t2PA641kG59L2cQpz";
  assert( addrStr == addrExpected && "Error to get SS58 Address from key");
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
   
   std::vector<uint8_t> vk = hex2bytes(PRIV_DEVICE_KEY);
   std::copy(vk.begin(), vk.end(), privateKey);

   derivePubKey(publicKey, privateKey);
    
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
   
    std::vector<uint8_t> vk = hex2bytes(PRIV_DEVICE_KEY);
    std::copy(vk.begin(), vk.end(), privateKey);
    derivePubKey(publicKey, privateKey);
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

  // setup WSL ports
  // New-NetFirewallRule -DisplayName "WSL2 Port Bridge" -Direction Inbound -Action Allow -Protocol TCP -LocalPort 9933,9944
  // netsh interface portproxy add v4tov4 listenport=9933 listenaddress=0.0.0.0 connectport=9933 connectaddress=172.18.145.240
  // ./target/release/robonomics --dev --tmp --unsafe-rpc-external --unsafe-ws-external

void test_json() {
  // depreciated get_payload method to get nonce, era, txVersion
  // curl "http://192.168.0.104:9933" -X POST -H "Content-Type: application/json" --data '{"jsonrpc":"2.0","id":20,"method":"get_payload","params":["5FHneW46xGXgs5mUiveU4sbTyGBzmstUspZC92UhjJM694ty"]}'
  // --> {"jsonrpc":"2.0","result":["0x00","0x01000000","0x00","0x0000000000000000","0x0100000000000000"],"id":20}
  //  nonce, spec_version, tip, era, tx_version
  //  ["0x631ccc82a078481584041656af292834e1ae6daab61d2875b4dd0c14bb9b17bc",0,16,0,"Immortal",1]
  //  genesis_hash, nonce, spec_version, tip, era, tx_version

  // chain_getBlockHash or chain_getHead to get genesis_hash
  // curl "http://kusama.rpc.robonomics.network/rpc/" -X POST -H "Content-Type: application/json" --data '{"jsonrpc":"2.0","id":20,"method":"chain_getHead","params":[]}'
  // {"jsonrpc":"2.0","result":"0x631ccc82a078481584041656af292834e1ae6daab61d2875b4dd0c14bb9b17bc","id":1}
  // curl "http://192.168.0.103:9944" -X POST -H "Content-Type: application/json" --data '{"jsonrpc":"2.0","id":1,"method":"chain_getBlockHash","params":[0]}'
  // {"jsonrpc":"2.0","result":"0x368d43155203b23de832b622b96172a5e5e8143a19342a240974c5c8394662bc","id":1}

  // can be replaced by state_getRuntimeVersion to get specVersion and transactionVersion
  // curl "http://192.168.0.103:9944" -X POST -H "Content-Type: application/json" --data '{"method":"state_getRuntimeVersion","params":[],"id":1,"jsonrpc":"2.0"}'
  // -->   {"jsonrpc":"2.0","result":{"specName":"robonomics-dev","implName":"robonomics-dev-airalab","authoringVersion":1,"specVersion":1,"implVersion":1,"apis":[["0xdf6acb689907609b",4],["0x37e397fc7c91f5e4",2],["0x40fe3ad401f8959a",6],["0xd2bc9897eed08f15",3],["0xf78b278be53f454c",2],["0xdd718d5cc53262d4",1],["0xed99c5acb25eedf5",3],["0xbc9d89904f5b923f",1],["0x37c8bb1350a9a2a8",4],["0xab3c0572291feb8b",1]],"transactionVersion":1,"stateVersion":1},"id":1}

  // account_nextIndex RPC or system_accountNextIndex to get nonce
  // curl "http://kusama.rpc.robonomics.network/rpc/" -X POST -H "Content-Type: application/json" --data '{"jsonrpc":"2.0","id":20,"method":"account_nextIndex","params":["5FHneW46xGXgs5mUiveU4sbTyGBzmstUspZC92UhjJM694ty"]}'
  // curl "http://kusama.rpc.robonomics.network/rpc/" -X POST -H "Content-Type: application/json" --data '{"jsonrpc":"2.0","id":20,"method":"system_accountNextIndex","params":["5FHneW46xGXgs5mUiveU4sbTyGBzmstUspZC92UhjJM694ty"]}'

  // author_submitExtrinsic to send signed data
  // sent: {"jsonrpc":"2.0","id":1,"method":"author_submitExtrinsic","params":["0x31038400f90bc712b5f2864051353177a9d627605d4bf7ec36c7df568cfdcea9f237c1850067e6443a40b695aee390d4909baa4540badc1eb02a6ea888a8b76fadd40a71eeaf568387b0eba338b9a52abee70df2d5dac5d552d2a42ed89f2fa507f172540700000013008eaf04151687736326c9fea17e25fc5287613693c912909cb226aa4794f26a481100010138656166303431353136383737333633323663396665613137653235666335323837363133363933633931323930396362323236616134373934663236613438"]}
  // {"jsonrpc":"2.0","error":{"code":1010,"message":"Invalid Transaction","data":"Transaction has a bad signature"},"id":1}

  uint8_t publicKey[KEYS_SIZE];
  uint8_t privateKey[KEYS_SIZE];

  std::vector<uint8_t> vk = hex2bytes(PRIV_DEVICE_KEY);
  std::copy(vk.begin(), vk.end(), privateKey);
  derivePubKey(publicKey, privateKey);
  std::vector<std::uint8_t> pubKeyVector( reinterpret_cast<std::uint8_t*>(std::begin(publicKey)), reinterpret_cast<std::uint8_t*>(std::end(publicKey)));

  // getPayloadJs() with account_nextIndex
  std::string payload = getPayloadJs(SS58_DEVICE_ADR, 8);
  std::string rpcExpected  = R"({"id":8.0,"jsonrpc":"2.0","method":"account_nextIndex","params":["5FHneW46xGXgs5mUiveU4sbTyGBzmstUspZC92UhjJM694ty"]})"; 
  log_printf("%s\n",payload.c_str());
  assert( payload == rpcExpected && "Filled rpc is not equal to expected pattern");

  // fillParamsJs() with author_submitExtrinsic
  auto record = "42"; // usefull data
  Data head = Data {0x33,0};
  Data call = callDatalogRecord(head,record);
  uint32_t era =  0;
  uint64_t nonce = 0; // from account_nextIndex
  uint64_t tip = 0;
  uint32_t txVersion = 1;      // from state_getRuntimeVersion;
  uint32_t specVersion =  0x1; // from state_getRuntimeVersion; 33 for http://kusama.rpc.robonomics.network/rpc/  1 for local node
  std::string ghash = "631ccc82a078481584041656af292834e1ae6daab61d2875b4dd0c14bb9b17bc";
  std::string bhash = "631ccc82a078481584041656af292834e1ae6daab61d2875b4dd0c14bb9b17bc";

  Data data = doPayload (call, era, nonce, tip, specVersion, txVersion, ghash, bhash);

  Data signature = doSign (data, privateKey, publicKey);
  Data edata = doEncode (signature, pubKeyVector, era, nonce, tip, call);

  payload = fillParamsJs(edata, 9); // with id
  rpcExpected  = R"({"id":9.0,"jsonrpc":"2.0","method":"author_submitExtrinsic","params":["0xad018400f90bc712b5f2864051353177a9d627605d4bf7ec36c7df568cfdcea9f237c185000eef49f2806113f781b42b5742c831e6605630f6e8b07398aea74c270be74bd7b9d8beb6b401cc97bce78bc3282461169d3a50d9191d97f2ba10f7ca4d384a090000003300083432"]})";
  log_printf("%s\n",payload.c_str());

  assert( payload == rpcExpected && "Filled rpc is not equal to expected pattern");

  // doExtrinsic for short payload
  TxData txe;
  txe.nonce = nonce;
  txe.tip = tip;
  txe.era = era;
  txe.tx_version = txVersion;
  txe.specVersion = specVersion;
  txe.ghash = ghash;
  txe.bhash = bhash;

  Data extrinsic = doExtrinsic (call, privateKey, publicKey, txe);
  payload = fillParamsJs(extrinsic, 9); // with id
  log_printf("%s\n",payload.c_str());

  assert( payload == rpcExpected && "Filled rpc is not equal to expected pattern");

  // parseJson()
  txe = parseJson("751");
  assert( txe.nonce == 751 && "Nonce is not equal to expected");
}

void test_doBlake() {
  Data data = Data {0x41,0x42}; // to hash 'AB'

  // Blake from cryptopp library
  auto hash = doBlake2(data, 32);
  assert (toHexString(hash) == "b028154a4095ff893c348083860df68e48cd23eeb2b626984256375b0bd497f2" && "Scale compact decoder error for 0");

  // online to compare
  // https://www.shawntabrizi.com/substrate-js-utilities/
  // https://emn178.github.io/online-tools/blake2b/
}

void test_doExtrinsic(bool isRemote) {

  Data head_rws = Data{0x37,0};
  Data head_dr = Data {0x33,0};
  TxData txe;

  // assign runtime depending data
  if (isRemote) {
    txe.ghash = "631ccc82a078481584041656af292834e1ae6daab61d2875b4dd0c14bb9b17bc";
    txe.bhash = "631ccc82a078481584041656af292834e1ae6daab61d2875b4dd0c14bb9b17bc";
    txe.specVersion = 33; // local 1 remote 33
    txe.tx_version = 1;
  } else {
    txe.ghash = "368d43155203b23de832b622b96172a5e5e8143a19342a240974c5c8394662bc";
    txe.bhash = "368d43155203b23de832b622b96172a5e5e8143a19342a240974c5c8394662bc";
    txe.specVersion = 1;
    txe.tx_version = 1;
    //  head_bt = Data{7,0};
    head_dr = Data{0x11,0};
    head_rws = Data{0x13,0};
  }
  txe.era = 0;
  txe.nonce = 0;
  txe.tip = 0;

  uint8_t publicKey[KEYS_SIZE];
  uint8_t privateKey[KEYS_SIZE];

  std::vector<uint8_t> vk = hex2bytes(PRIV_DEVICE_KEY);
  std::copy(vk.begin(), vk.end(), privateKey);

  std::string ownerKey = PUB_OWNER_KEY;
  derivePubKey(publicKey, privateKey);

  // usefull data with payload 257 bytes
  std::string record =R"({"SDS_P1":11.45,"SDS_P2":7.50,"noiseMax":48.0,"noiseAvg":47.55,"temperature":20.95,"press":687.97,"humidity":62.5,"lat":0.000000,"lon":0.00000})";
  // Prepare datalog record for nesting:  call header for Datalog record + some payload
  Data call_nested = callDatalogRecord(head_dr,record);
  Data call = callRws(head_rws, ownerKey, call_nested); // inject nested call into RWS call
  Data extrinsic = doExtrinsic (call, privateKey, publicKey, txe);
  std::string expected = "6d048400f90bc712b5f2864051353177a9d627605d4bf7ec36c7df568cfdcea9f237c18500720960504f246a40640535ad75181b619c7e8b73d878fc4640c8a13b1d5271abedec2c048422b1f1fa2d67c6b8a56bee8b03fa2b53097b1108fee64ed7773c0800000013008eaf04151687736326c9fea17e25fc5287613693c912909cb226aa4794f26a4811003d027b225344535f5031223a31312e34352c225344535f5032223a372e35302c226e6f6973654d6178223a34382e302c226e6f697365417667223a34372e35352c2274656d7065726174757265223a32302e39352c227072657373223a3638372e39372c2268756d6964697479223a36322e352c226c6174223a302e3030303030302c226c6f6e223a302e30303030307d";
  // printBytes("Signature size", extrinsic);
  assert (toHexString(extrinsic) == expected && "Error in crated 1st extrinsic");

  // usefull data 2X with payload 400 bytes
  std::string record2x = record + record;
  call_nested = callDatalogRecord(head_dr,record2x);
  call = callRws(head_rws, ownerKey, call_nested);
  extrinsic = doExtrinsic (call, privateKey, publicKey, txe);

  std::string expected2 = "a9068400f90bc712b5f2864051353177a9d627605d4bf7ec36c7df568cfdcea9f237c18500502e3b7cbd209e7ba9aea942747bd5f9dd49c6529f6a572ee29be7fc12730cebdd26c8091961b8e3c3310626851161db3d1c88bdbfd47ecfe09a04e4cc3e7b0100000013008eaf04151687736326c9fea17e25fc5287613693c912909cb226aa4794f26a48110079047b225344535f5031223a31312e34352c225344535f5032223a372e35302c226e6f6973654d6178223a34382e302c226e6f697365417667223a34372e35352c2274656d7065726174757265223a32302e39352c227072657373223a3638372e39372c2268756d6964697479223a36322e352c226c6174223a302e3030303030302c226c6f6e223a302e30303030307d7b225344535f5031223a31312e34352c225344535f5032223a372e35302c226e6f6973654d6178223a34382e302c226e6f697365417667223a34372e35352c2274656d7065726174757265223a32302e39352c227072657373223a3638372e39372c2268756d6964697479223a36322e352c226c6174223a302e3030303030302c226c6f6e223a302e30303030307d";
  // printBytes("Signature size", extrinsic);
  assert (toHexString(extrinsic) == expected2 && "Error in crated 2nd extrinsic");

  // usefull data 3X with payload 543 bytes
  std::string record3x = record + record + record;
  call_nested = callDatalogRecord(head_dr,record3x);
  call = callRws(head_rws, ownerKey, call_nested);
  extrinsic = doExtrinsic (call, privateKey, publicKey, txe);

  std::string expected3 = "e5088400f90bc712b5f2864051353177a9d627605d4bf7ec36c7df568cfdcea9f237c18500ecc3c2fdabbc988198b0d5b1c84034cb913b99ef4bf35a2145596f5856ddf300771441585822006d390ca855395b59c5173c79cd4d2c7321d60f91790db3fd0f00000013008eaf04151687736326c9fea17e25fc5287613693c912909cb226aa4794f26a481100b5067b225344535f5031223a31312e34352c225344535f5032223a372e35302c226e6f6973654d6178223a34382e302c226e6f697365417667223a34372e35352c2274656d7065726174757265223a32302e39352c227072657373223a3638372e39372c2268756d6964697479223a36322e352c226c6174223a302e3030303030302c226c6f6e223a302e30303030307d7b225344535f5031223a31312e34352c225344535f5032223a372e35302c226e6f6973654d6178223a34382e302c226e6f697365417667223a34372e35352c2274656d7065726174757265223a32302e39352c227072657373223a3638372e39372c2268756d6964697479223a36322e352c226c6174223a302e3030303030302c226c6f6e223a302e30303030307d7b225344535f5031223a31312e34352c225344535f5032223a372e35302c226e6f6973654d6178223a34382e302c226e6f697365417667223a34372e35352c2274656d7065726174757265223a32302e39352c227072657373223a3638372e39372c2268756d6964697479223a36322e352c226c6174223a302e3030303030302c226c6f6e223a302e30303030307d";
  // printBytes("Signature size", extrinsic);
  assert (toHexString(extrinsic) == expected3 && "Error in crated 3nd extrinsic");

  // usefull data 512 with payload 626 bytes
   std::string record0 = R"({temperature:20.9,pressure:687.97,humidity:62.5,p1:11.45,p2:7.50,nm:48.00,na:47.55})";
   std::string recordm = record + record + record + record0; // max datalog record size 512 bytes
   call_nested = callDatalogRecord(head_dr,recordm);
   call = callRws(head_rws, ownerKey, call_nested);
   extrinsic = doExtrinsic (call, privateKey, publicKey, txe);

   std::string expectedm = "310a8400f90bc712b5f2864051353177a9d627605d4bf7ec36c7df568cfdcea9f237c185000815a89e895337f39b58de60cbe24a4c29bcd60d1e939d886d4c518d49d35858d380548aea539e93e567fa9b9f6a7b1fab80012048b75a1c857416d471e0ba0100000013008eaf04151687736326c9fea17e25fc5287613693c912909cb226aa4794f26a48110001087b225344535f5031223a31312e34352c225344535f5032223a372e35302c226e6f6973654d6178223a34382e302c226e6f697365417667223a34372e35352c2274656d7065726174757265223a32302e39352c227072657373223a3638372e39372c2268756d6964697479223a36322e352c226c6174223a302e3030303030302c226c6f6e223a302e30303030307d7b225344535f5031223a31312e34352c225344535f5032223a372e35302c226e6f6973654d6178223a34382e302c226e6f697365417667223a34372e35352c2274656d7065726174757265223a32302e39352c227072657373223a3638372e39372c2268756d6964697479223a36322e352c226c6174223a302e3030303030302c226c6f6e223a302e30303030307d7b225344535f5031223a31312e34352c225344535f5032223a372e35302c226e6f6973654d6178223a34382e302c226e6f697365417667223a34372e35352c2274656d7065726174757265223a32302e39352c227072657373223a3638372e39372c2268756d6964697479223a36322e352c226c6174223a302e3030303030302c226c6f6e223a302e30303030307d7b74656d70657261747572653a32302e392c70726573737572653a3638372e39372c68756d69646974793a36322e352c70313a31312e34352c70323a372e35302c6e6d3a34382e30302c6e613a34372e35357d";
   // printBytes("Signature size", extrinsic);
   assert (toHexString(extrinsic) == expectedm && "Error in crated 4th extrinsic");
}

int main () {
  test_callDatalogRecord();
  test_doPayload();
  test_doSign();
  test_doEncode();
  test_DecodeU32();
  test_doBlake();
  test_doExtrinsic(false);
  test_json();
  test_doAddress();
}
