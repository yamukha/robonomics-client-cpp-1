# Solution to submit extrinsic from microcontroller like ESP8266 to Robonomics blockchain


# There are some predefined values to use
- URLRPC:  blockchain server address to connect
- PRIV_KEY: node private key
- SS58_ADR: linked ss58 address to private key of node
- SS58_DST_KEY: linked ss58 address to private key of destination node for TransferBalance extrinsic method

Note: to explore/create existing key/address can be used sub.py script from this repository or subkey utility from substrate

# Inplemented extrinsic methods
- DatalogRecord
- TransferBalance

# CI
There are in .github/workflows/ files:
- datalog-tiny-test.yml   ->  act -j libtests ->  to test parts of RpcRobonomics library on linux 
- datalog-tiny.yml        ->  act -j esp8622  ->  to build by arduino-cli  binary for target device

# Deploy 
Note: need to update in rpcjson.ino file STASSID  and STAPSK  macro definitions

# Setup or update robonomics URL
To set up the new robonomics URL can be used Arduino SDK Serial Monitor or any other SW able to communicate over serial port (115200N8).
After ESP8266 reset you have 3 seconds window to send URL as string, i.e.:  http://kusama.rpc.robonomics.network/rpc/ or http://192.168.0.102:9933

# EEPROM layout
- 1st byte: URL string size
- 2nd byte: simple checksum for URL string
- 3rd byte: 1st character of URL
