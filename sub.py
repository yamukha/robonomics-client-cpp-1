from substrateinterface import SubstrateInterface, Keypair, KeypairType
from substrateinterface.exceptions import SubstrateRequestException
import time, sys

# script to help debug extrinsics with wireshark tcpdump

# usage for local node:
# python3 sub.py local [extrinsic] [param1] [param2]
# python sub.py local datalog 
# for remote:
# python3 sub.py remote rws
# supported extrisics: datalog, balance, rws

# use subkey to get ed25519 ss58_addresses for mnemonics from files: mnemonics.txt, mnemonicsO.txt, mnemonicsD.txt
# subkey inspect "mnemonics from mnemonics*.txt files" --network robonomics --scheme ed25519  
# then replace inspected ss58_address and subscription_id for RWS call
# Note: in git are placed files with dummy mnemonics
# put in mnemonics directory valid mnemonics files and point path to it:
mnemonics_dir="";
#mnemonics_dir="mnemonics/";

# The account with some balance
with open(mnemonics_dir + 'mnemonics.txt', 'r') as file:
    mnemonics = file.read().rstrip()
    keypair  = Keypair.create_from_uri(mnemonics, crypto_type=KeypairType.ED25519)
    keypair.ss58_address = "4FKq5Vqm5APgsoqJ6ADp9FC6csfdabaCN4CDT4L5Dp6x8RgK" # instead of substrate use robonomics address
# RWS subscription bid Owner
with open(mnemonics_dir + 'mnemonicsO.txt', 'r') as file:
    mnemonicsO = file.read().rstrip()    
    keypairO  = Keypair.create_from_uri(mnemonicsO, crypto_type=KeypairType.ED25519)
    keypairO.ss58_address = "4FhjeTDgmS1nUWf12Xdr7tJCKNa2w23PmNnDFLSDhWA5ccNy" # instead of substrate use robonomics address
# RWS subscribed Device
with open(mnemonics_dir + 'mnemonicsD.txt', 'r') as file:
    mnemonicsD = file.read().rstrip()    
    keypairD  = Keypair.create_from_uri(mnemonicsD, crypto_type=KeypairType.ED25519)
    keypairD.ss58_address = "4CTW757AC8Y5orf6w9JzWDi7sErEgKJZAcHfPyzqFJYGLZVk"

if len(sys.argv) > 1 and sys.argv[1] == "local":
    wait4inclusion = False
    #block_hash = "0x5a73dd6af880604a183bff15a19599fd386f00264ab1dd56813fda7eaffe456e"
    block_hash = "0x368d43155203b23de832b622b96172a5e5e8143a19342a240974c5c8394662bc"
    #node_url =  url="ws://127.0.0.1:9944"
    #node_url =  url="http://127.0.0.1:9944"
    node_url =  url="http://192.168.0.103:9944"
    # node_url =  url="ws://192.168.0.103:9944"
    #keypair  = Keypair.create_from_uri('//Alice')
    #keypairM = Keypair.create_from_uri('//Bob')

else:
    wait4inclusion = False
    block_hash = "0x631ccc82a078481584041656af292834e1ae6daab61d2875b4dd0c14bb9b17bc"
    # node_url =  url="http://kusama.rpc.robonomics.network/rpc/"
    node_url =  url="ws://kusama.rpc.robonomics.network/rpc/"
    # node_url =  url="http://192.168.0.103:9944"
    # block_hash = "0x368d43155203b23de832b622b96172a5e5e8143a19342a240974c5c8394662bc"

print('node_url: "{}", block_hash: "{}" "{}"'.format(node_url, block_hash, len(sys.argv)))

try:
    substrate = SubstrateInterface(
        url=node_url,
        ss58_format=2,
        type_registry_preset='kusama'
    )

except ConnectionRefusedError:
    print("No local Substrate node running, try running 'start_local_substrate_node.sh' first")
    exit()

# Other address for transfer balance
print('priv key ', bytes(keypair.private_key).hex())
print('pub key  ', bytes(keypair.public_key).hex())
print('ss58     ', keypair.ss58_address)
account_info = substrate.query('System', 'Account', params=[keypair.ss58_address])
print('Account info', account_info.value)

# Other address for transfer balance
print('priv key subcription Owner: ', bytes(keypairO.private_key).hex())
print('pub  key subcription Owner: ', bytes(keypairO.public_key).hex())
print('ss58     subcription Owner: ', keypairO.ss58_address)
account_infoO = substrate.query('System', 'Account', params=[keypairO.ss58_address])
print('Account info subcription Owner:', account_infoO.value)

print('priv key subcribed Device: ', bytes(keypairD.private_key).hex())
print('pub  key subcribed Device: ', bytes(keypairD.public_key).hex())
print('ss58     subcribed Device: ', keypairD.ss58_address)
#account_infoD = substrate.query('System', 'Account', params=[keypairD.ss58_address])
#print('Account info subcribed Device:', account_infoD.value)

call_bt = substrate.compose_call(
    call_module='Balances',
    call_function='transfer',
    call_params={
        'dest': keypairO.ss58_address,
        'value': 100000
    }
)

call_dr = substrate.compose_call(
    call_module='Datalog',
    call_function='record',
    call_params={
        'record': '{"SDS_P1":11.45,"SDS_P2":7.50,"noiseMax":48.0,"noiseAvg":47.55,"temperature":20.95,"press":687.97,"humidity":62.5,"lat":0.000000,"lon":0.00000}'
        #'record': '{"temperature":20,"press":687.97,"humidity":62.5,"lat":0.000000,"lon":0.00000}'
        # 'record': ' '
    }
)

call_rws = substrate.compose_call(
    call_module='RWS',
    call_function='call',
    call_params={
        'subscription_id': '4FhjeTDgmS1nUWf12Xdr7tJCKNa2w23PmNnDFLSDhWA5ccNy',
        'call': call_dr
    }
)

call=call_dr
if len(sys.argv) > 2 and sys.argv[2] == "balance":
    call=call_bt
if len(sys.argv) > 2 and sys.argv[2] == "rws":
    call=call_rws
    print("RWS:")
    keypair = keypairD    

# Get payment info
print ("call: ",call, "\nas bytes: ", call.encode())

#payment_info = substrate.get_payment_info(call=call, keypair=keypair)
#print("Payment info: ", payment_info)

extrinsic = substrate.create_signed_extrinsic(
    call=call,
    keypair=keypair,
    # era={'period': 0, 'current': 0},
    # nonce = 437, # incremented ?
)
print("signed extrinsic: ", extrinsic)

nonce = 444
# nonce = 72
era= {'period': 0,  'current': 0}
#signature_payload = substrate.generate_signature_payload(call=call, era=era, nonce=nonce)
#signature_payload = substrate.generate_signature_payload(call=call, era=era) # +1 byte to 'record'
signature_payload = substrate.generate_signature_payload(call=call,  nonce=nonce)  # +1 byte to 'record', like for ESP with correct nonce
#signature_payload = substrate.generate_signature_payload(call=call)  # 1+1 byte to 'record'

print("payload: ", signature_payload)

try:
    print ('try submit')
    exit()
    receipt = substrate.submit_extrinsic(extrinsic, wait_for_inclusion=wait4inclusion) # true for ws
    # receipt = substrate.submit_extrinsic(extrinsic, wait_for_inclusion=True) # signature_payload
    print('Extrinsic "{}" included in block "{}"'.format(
         receipt.extrinsic_hash, receipt.block_hash
    ))

    try:
        if receipt.is_success:
            print(" Success, triggered events:")
            # for event in receipt.triggered_events:
            #    print(f"* {event.value}")
        else:
           print('Extrinsic Failed: ', receipt.error_message)
    except:
        print("Failed to send Extrinsic with Exception")
            
    #account_info = substrate.query('System', 'Account', params=[keypair.ss58_address])
    #print('Account info  : ', account_info.value)

    #account_infoO = substrate.query('System', 'Account', params=[keypairO.ss58_address])
    #print('Account info subcription Owner: ', account_infoO.value)

    #account_infoD = substrate.query('System', 'Account', params=[keypairD.ss58_address])
    #print('Account info subcribed Device: ', account_infoD.value)

except SubstrateRequestException as e:
    print("Failed to send: {}".format(e))
