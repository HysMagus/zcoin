#include "Bip47Util.h"
#include "wallet/wallet.h"
#include "PaymentCode.h"
#include "SecretPoint.h"
#include "primitives/transaction.h"
#include "PaymentAddress.h"
#include <vector>
#include "uint256.h"

using namespace std;

bool BIP47Util::getOpCodeOutput(const CTransaction& tx, CTxOut& txout) {
    for(int i = 0; i < tx.vout.size(); i++) {
        if (tx.vout[i].scriptPubKey[0] == OP_RETURN) {
            txout = tx.vout[i];
            return true;
        }
    }
    return false;
}


bool BIP47Util::isValidNotificationTransactionOpReturn(CTxOut txout) {
    vector<unsigned char> op_date;
    return getOpCodeData(txout, op_date);
}

bool BIP47Util::getOpCodeData(CTxOut txout, vector<unsigned char>& op_data) {
    CScript::const_iterator pc = txout.scriptPubKey.begin();
    vector<unsigned char> data;
    
    while (pc < txout.scriptPubKey.end())
    {
        opcodetype opcode;
        if (!txout.scriptPubKey.GetOp(pc, opcode, data))
        {
            LogPrintf("GetOp false in getOpCodeData\n");
            return false;
        }
        LogPrintf("Data.size() = %d,  opcode = 0x%x\n", data.size(), opcode);
        if (data.size() > 0 && opcode < OP_PUSHDATA4  )
        {
            op_data = data;
            return true;
        } 
        
    }
}

bool BIP47Util::getPaymentCodeInNotificationTransaction(vector<unsigned char> privKeyBytes, CTransaction tx, PaymentCode &paymentCode) {
    // tx.vin[0].scriptSig
    CWalletTx wtx(pwalletMain, tx);

    CTxOut txout;
    if(!getOpCodeOutput(tx, txout)) {
        LogPrintf("Cannot Get OpCodeOutput\n");
        return false;
    }

    if(!isValidNotificationTransactionOpReturn(txout))
    {
        LogPrintf("Error isValidNotificationTransactionOpReturn txout\n");
        return false;
    }

    vector<unsigned char> op_data;
    if(!getOpCodeData(txout, op_data)) {
        LogPrintf("Cannot Get OpCodeData\n");
        return false;
    }

    /**
     * @Todo Get PubKeyBytes from tx script Sig
     * */
    vector<unsigned char> pubKeyBytes;

    if (!getScriptSigPubkey(wtx.vin[0], pubKeyBytes))
    {
        LogPrintf("Bip47Utiles PaymentCode ScriptSig GetPubkey error\n");
        return false;
    }


    // vector<unsigned char> outpoint = ParseHex(wtx.vin[0].prevout.ToString());
    vector<unsigned char> outpoint(wtx.vin[0].prevout.hash.begin(), wtx.vin[0].prevout.hash.end());

    SecretPoint secretPoint(privKeyBytes, pubKeyBytes);

    LogPrintf("output: %s\n", wtx.vin[0].prevout.hash.GetHex());
    uint256 secretPBytes(secretPoint.ECDHSecretAsBytes());
    LogPrintf("secretPoint: %s\n", secretPBytes.GetHex());

    vector<unsigned char> mask = PaymentCode::getMask(secretPoint.ECDHSecretAsBytes(), outpoint);
    vector<unsigned char> payload = PaymentCode::blind(op_data, mask);
    PaymentCode pcode(payload.data(), payload.size());
    paymentCode = pcode;
    return true;
}

bool BIP47Util::getScriptSigPubkey(CTxIn txin, vector<unsigned char>& pubkeyBytes)
{

    LogPrintf("ScriptSig size = %d\n", txin.scriptSig.size());
    CScript::const_iterator pc = txin.scriptSig.begin();
    vector<unsigned char> chunk0data;
    vector<unsigned char> chunk1data;
    
    opcodetype opcode0, opcode1;
    if (!txin.scriptSig.GetOp(pc, opcode0, chunk0data))
    {
        LogPrintf("Bip47Utiles ScriptSig Chunk0 error != 2\n");
        return false;
    }
    LogPrintf("opcode0 = %x, chunk0data.size = %d\n", opcode0, chunk0data.size());

    if (!txin.scriptSig.GetOp(pc, opcode1, chunk1data))
    {
        LogPrintf("Bip47Utiles ScriptSig Chunk1 error != 2\n");
        return false;
    } 
    LogPrintf("opcode1 = %x, chunk1data.size = %d\n", opcode1, chunk1data.size());
    
    if(!chunk0data.empty() && chunk0data.size() > 2 && !chunk1data.empty() && chunk1data.size() > 2)
    {
        pubkeyBytes = chunk1data;
        return true;
    }
    else if(opcode0 == OP_CHECKSIG && !chunk0data.empty() && chunk0data.size() > 2)
    {
        pubkeyBytes = chunk0data;
        return true;
    }

    LogPrintf("Script did not match expected form: \n");
    // while (pc < txin.scriptSig.end())
    // {
    //     opcodetype opcode;
    //     if (!txin.scriptSig.GetOp(pc, opcode, chunk0data))
    //         break;
    //     if (data.size() == 80 && opcode == OP_RETURN)
    //     {
    //         op_data = data;
    //         return true;
    //     }
    // }
    return false;
}

PaymentAddress BIP47Util::getPaymentAddress(PaymentCode &pcode, int idx, CExtKey extkey) {
    vector<unsigned char> privKey(extkey.key.GetPrivKey().begin(), extkey.key.GetPrivKey().end());
    return PaymentAddress(0, pcode, idx, privKey);
}
