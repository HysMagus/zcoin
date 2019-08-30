//
// Created by Top1s on 8/22/2019.
//

#if defined(HAVE_CONFIG_H)
#include "config/bitcoin-config.h"
#endif

#include "bip47.h"
#include "wallet/wallet.h"
#include "wallet/walletexcept.h"
#include "wallet/sigmaspendbuilder.h"
#include "amount.h"
#include "base58.h"
#include "checkpoints.h"
#include "chain.h"
#include "coincontrol.h"
#include "consensus/consensus.h"
#include "consensus/validation.h"
#include "key.h"
#include "keystore.h"
#include "main.h"
#include "zerocoin.h"
#include "sigma.h"
#include "../sigma/coinspend.h"
#include "../sigma/spend_metadata.h"
#include "../sigma/coin.h"
#include "../sigma/remint.h"
#include "../libzerocoin/SpendMetaData.h"
#include "net.h"
#include "policy/policy.h"
#include "primitives/block.h"
#include "primitives/transaction.h"
#include "script/script.h"
#include "script/sign.h"
#include "timedata.h"
#include "txmempool.h"
#include "util.h"
#include "ui_interface.h"
#include "utilmoneystr.h"
#include "validation.h"
#include "darksend.h"
#include "instantx.h"
#include "znode.h"
#include "znode-sync.h"
#include "random.h"
#include "init.h"
#include "hdmint/wallet.h"
#include "rpc/protocol.h"

#include "hdmint/tracker.h"

#include <assert.h>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>

using namespace std;
#include <stdexcept>

extern void noui_connect();

#include <string>
#include <vector>
#include <string.h>
#include <iostream>


namespace bip47 {

    using namespace std;

    PaymentCode::PaymentCode() {
    }

    PaymentCode::PaymentCode(byte pkey[PUBLIC_KEY_LEN], byte ch[CHAIN_LEN]) {
        memcpy(pubkey, pkey, PUBLIC_KEY_LEN);
        memcpy(chain, ch, CHAIN_LEN);
        strPaymentCode = makeV1();
    }

    PaymentCode::PaymentCode(string payment_code) {
        strPaymentCode = payment_code;
        parse_payment_code();
    }

    string PaymentCode::makeV1() {
        string ret = "";
        byte payload[PAYLOAD_LEN];
        byte payment_code[PAYLOAD_LEN + 1];

        memset(payload, 0, PAYLOAD_LEN);
        memset(payment_code, 0, PAYLOAD_LEN + 1);

        // byte 0: type. required value: 0x01
        payload[0] = (byte)0x01;
        // byte 1: features bit field. All bits must be zero except where specified elsewhere in this specification
        //      bit 0: Bitmessage notification
        //      bits 1-7: reserved
        payload[1] = 0;
        // replace sign & x code (33 bytes)
        memcpy(payload + 2, pubkey, PUBLIC_KEY_LEN);
        // replace chain code (32 bytes)
        memcpy(payload + 35, chain, CHAIN_LEN);

        payment_code[0] = (byte)0x47;
        memcpy(payment_code + 1, payload, PAYLOAD_LEN);

        vector<byte> payment_code_checksum(std::begin(payment_code), std::end(payment_code));
        ret = EncodeBase58Check(payment_code_checksum);

        printf("encode ret = %s\n", ret.c_str());

        return ret;
    };

    bool PaymentCode::parse_payment_code() {
        vector<byte> payment_code;
        DecodeBase58Check(strPaymentCode, payment_code);
        if(payment_code[0] != 0x47) {
            LogPrint("Payment code parsing", "Failed");
            return false;
        }

        byte pcodes[PAYLOAD_LEN] = {};
        std::copy(payment_code.begin(), payment_code.end(), pcodes);
        memcpy(pubkey, pcodes + 3, PUBLIC_KEY_LEN);

//        byte c_codes[CHAIN_LEN] = {};
        memcpy(chain, pcodes + PUBLIC_KEY_LEN + 3, CHAIN_LEN);

        return true;
    }

    vector<byte> PaymentCode::getPubkey() {
        printf("\n Get Pubkey %d\n", sizeof(pubkey));
        vector<byte> vpubkey(pubkey, pubkey + PUBLIC_KEY_LEN);

        return vpubkey;
    }

    CPubKey PaymentCode::getMasterPubkey() {

        vector<byte> vpubkey = getPubkey();

        printf("\n Vector size %d\n", vpubkey.size());
        CPubKey masterPubkey(vpubkey);
//        extPubKey.pubkey = masterPubkey;

        return masterPubkey;
    }

    bool PaymentCode::valid() {
        vector<byte> payment_code;
        DecodeBase58Check(strPaymentCode, payment_code);
        if(payment_code[0] != 0x47) {
            LogPrint("Address Format Error", "Failed");
            return false;
        }

        byte pcodes[PAYLOAD_LEN] = {};
        std::copy(payment_code.begin(), payment_code.end(), pcodes);

        printf("\nfirst_byte %s\n", (*(pcodes + 3) == 2 || *(pcodes + 3) == 3) ? "True" : "False" );


        return true;
    }
}
