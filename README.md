# Solution to submit extrinsics from microcontroller like ESP8266 to Robonomics blockchain

# Inplemented extrinsic methods
DatalogRecord\
RWS with DatalogRecord

# CI
There are in .github/workflows/ files:\
esp-extrinsics.yml        ->  act -j esp8622  ->  to build by arduino-cli binary for target device and run library unit tests

# Deploy 
1. Set up in Private.h file proper values of keys (Note: ED25519 scheme!) and addresses for subscription owner and device.\
    Note: to explore/create existing key/address can be used sub.py script from this repository or subkey utility from substrate\
    i.e. to get keys for known account mnemonic phrase:\
    subkey inspect "some ... mnemonics" --network robonomics --scheme ed25519\
2. Set up in Private.h file proper values for STASSID and STAPSK macro definitions for WiFi access

# For RWS set up subscription  described here:
https://wiki.robonomics.network/docs/get-subscription
https://wiki.robonomics.network/docs/subscription-launch/

# Set up or update robonomics URL
To set up the new robonomics URL can be used Arduino SDK Serial Monitor or any other SW able to communicate over serial port (115200N8).\
After ESP8266 reset you have 3 seconds window to send URL as string, i.e.:  http://kusama.rpc.robonomics.network/rpc/ or http://192.168.0.102:9933

# EEPROM layout
- 1st byte: URL string size
- 2nd byte: simple checksum for URL string
- 3rd byte: 1st character of URL

# Submit of extrinsic be checked in polkadot web UI -> Network -> Explorer
https://polkadot.js.org/apps/?rpc=wss%3A%2F%2Fkusama.rpc.robonomics.network%2F#/explorer
