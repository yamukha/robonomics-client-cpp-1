#pragma once

#include <vector>
#include <string>
#include "Encoder.h"
#include "Utils.h"
#include "Defines.h"
#include  "TxData.h"

#ifdef USE_LINUX
#include "cryptopp/donna.h"
#include "cryptopp/blake2.h"
#include "cryptopp/xed25519.h"
#else
#include <Ed25519.h>
#include <BLAKE2b.h>
#endif

std::vector<uint8_t> doPayload (Data call, uint32_t era, uint64_t nonce, uint64_t tip, uint32_t sv, uint32_t tv, std::string gen, std::string block) {
    Data data;
    append(data, call);
    append(data, encodeCompact(era)); // era; note: it simplified to encode, maybe need to rewrite

    append(data, encodeCompact(nonce));
    append(data, encodeCompact(tip));
              
    encode32LE(sv, data);     // specversion
    encode32LE(tv, data);     // version
            
    std::vector<uint8_t> gh = hex2bytes(gen.c_str());
    append(data, gh);
    std::vector<uint8_t> bh = hex2bytes(block.c_str()); // block hash
    append(data, bh);     
    return data;
}

std::vector<uint8_t> doSign(Data data, uint8_t privateKey[32], uint8_t publicKey[32]) {

    uint8_t payload[data.size()];             
    uint8_t sig[SIGNATURE_SIZE];
     
    std::copy(data.begin(), data.end(), payload);
#ifdef USE_LINUX
    //do like Arduino Ed25519::sign() for unit test, i.e. by crypto++ library
    CryptoPP::Donna::ed25519_sign(payload, data.size(), privateKey, publicKey, sig);
#else
   Ed25519::sign(sig, privateKey, publicKey, payload, data.size());
#endif
    std::vector<byte> signature (sig,sig + SIGNATURE_SIZE);   // signed data as bytes vector
    return signature;
}

std::vector<uint8_t> doEncode (Data signature, Data pubKey, uint32_t era, uint64_t nonce, uint64_t tip, Data call) {
    Data edata;
    append(edata, Data{extrinsicFormat | signedBit});  // version header
    append(edata,0);

    //std::vector<std::byte> pubKey( reinterpret_cast<std::byte*>(std::begin(publicKey)), reinterpret_cast<std::byte*>(std::end(publicKey)));
    append(edata,pubKey);  // signer public key
    append(edata, sigTypeEd25519); // signature type
    append(edata, signature);      // signatured payload
              
    // era / nonce / tip // append(edata, encodeEraNonceTip());
    append(edata, encodeCompact(era)); // era; note: it simplified to encode, maybe need to rewrite
    append(edata, encodeCompact(nonce));
    append(edata, encodeCompact(tip));                            
  
    append(edata, call);
    encodeLengthPrefix(edata); // append length
              
    return edata;
}

#ifdef USE_LINUX
std::vector<uint8_t> doBlake2 (Data data, uint32_t len) {
    CryptoPP::BLAKE2b blake2b(len);
    uint8_t hash2b[len] = {0};
    uint8_t arr[data.size()];
    std::copy(data.begin(), data.end(), arr);
    blake2b.Restart();
    blake2b.Update(arr, data.size());
    blake2b.TruncatedFinal(hash2b, len);
    std::vector<uint8_t> hash(hash2b, hash2b + len);
    return hash;
}
#else
std::vector<uint8_t> doBlake2(Data data, uint32_t len) {
    unsigned char hash2b[len] = {0};
    uint8_t arr[data.size()];
    std::copy(data.begin(), data.end(), arr);

    BLAKE2b blake2b;
    uint8_t hash2b_[len];
    blake2b.reset(len);
    blake2b.update(arr, data.size());
    blake2b.finalize(hash2b, sizeof(hash2b));

    std::vector<uint8_t> hash(hash2b, hash2b + len);
    return hash;
}
#endif

std::vector<uint8_t> doExtrinsic (Data call, uint8_t privateKey[KEYS_SIZE], uint8_t publicKey[KEYS_SIZE], TxData txe) {
    Data payload = doPayload (call, txe.era, txe.nonce, txe.tip, txe.specVersion, txe.tx_version, txe.ghash, txe.bhash);
    printBytes("Payload size", payload);
    // Blake2 if payload size > 256 byte
    Data hash {};
    if (payload.size() > MAX_PAYLOAD_DATA_LEN) {
       hash = doBlake2(payload,BLAKE2_HASH_LEN);
       printBytes("Blake2b hash ", hash);
    }
    Data signature {};
    if (payload.size() > MAX_PAYLOAD_DATA_LEN) {
      signature = doSign (hash, privateKey, publicKey);
    }
    else {
      signature = doSign (payload, privateKey, publicKey);
    }
    printBytes("Signature size", signature);
    std::vector<std::uint8_t>  pubKey;
    pubKey.assign(publicKey, publicKey + KEYS_SIZE);
    Data encoded = doEncode (signature, pubKey, txe.era, txe.nonce, txe.tip, call);
    return encoded;
}

std::string getAddrFromPublicKey(uint8_t pubKey[KEYS_SIZE], uint16_t prefix) {

    size_t prefix_len = prefix < 64 ? 1 : 2;
    size_t ss58pre_len =  sizeof(SS58_PRE) - 1;
    size_t hash_len = 2;
    Data pkey (pubKey, pubKey + KEYS_SIZE);

    Data prfx;
    if (prefix < 64) {
        prfx.push_back(static_cast<uint8_t>(prefix));
    } else {
        // i.e. prefix = 137;
        uint16_t fullPrefix = 0x4000 | ((prefix >> 8) & 0x3F) | ((prefix & 0xFF) << 6);
        prfx.push_back(static_cast<uint8_t>(fullPrefix >> 8));
        prfx.push_back(static_cast<uint8_t>(fullPrefix & 0xFF));
    }

    Data toHash;
    toHash.insert(toHash.end(), std::begin(SS58_PRE), std::begin(SS58_PRE) + ss58pre_len);
    toHash.insert(toHash.end(), std::begin(prfx), std::end(prfx));
    toHash.insert(toHash.end(), std::begin(pkey), std::end(pkey));
    Data hash = doBlake2 (toHash, ARRAY_LEN_64);

    Data rawAddr;
    rawAddr.insert(rawAddr.end(), std::begin(prfx), std::end(prfx));
    rawAddr.insert(rawAddr.end(), std::begin(pkey), std::end(pkey));
    rawAddr.insert(rawAddr.end(), std::begin(hash), std::begin(hash) + hash_len);

    unsigned char rawAddress[rawAddr.size()] = {0};
    std::copy(rawAddr.begin(), rawAddr.end(), rawAddress);

    unsigned char addr58[ARRAY_LEN_64] = {0};
    int encodedLen = EncodeBase58(rawAddress, rawAddr.size(), addr58);

    std::vector<char> addr (addr58, addr58 + encodedLen + 1);
    std::string address(addr.data());
    return address;
}

#ifdef USE_LINUX
void derivePubKey ( uint8_t publicKey[KEYS_SIZE], uint8_t  privateKey[KEYS_SIZE]){
    using namespace CryptoPP;

    ed25519::Signer signer = ed25519Signer (privateKey);
    ed25519::Verifier verifier(signer);

    const ed25519PublicKey& pubKey = dynamic_cast<const ed25519PublicKey&>(verifier.GetPublicKey());
    auto pt = pubKey.GetPublicKeyBytePtr();
 
    std::memcpy(publicKey, pt, KEYS_SIZE);
}
#else
#endif

std::string doAddress(std::string pkey) {
    uint8_t publicKey[KEYS_SIZE];
    uint8_t privateKey[KEYS_SIZE];

    std::vector<uint8_t> vk = hex2bytes(pkey);
    std::copy(vk.begin(), vk.end(), privateKey);
#ifdef USE_LINUX
    derivePubKey(publicKey, privateKey);
#else
     Ed25519::derivePublicKey(publicKey, privateKey);
#endif
    std::string addrStr = getAddrFromPublicKey (publicKey, KEYS_SIZE);
    log_printf("Size %lu, Address: %s\n", addrStr.size(), addrStr.c_str());
    return addrStr;
}
