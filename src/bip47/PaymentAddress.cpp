#include "PaymentAddress.h"
#include "PaymentCode.h"


PaymentAddress::PaymentAddress()
{
    // paymentCode = nullptr;
    // privKey = nullptr;
    index = 0;

}
PaymentAddress::PaymentAddress(PaymentCode paymentCode_t)
{
    paymentCode = paymentCode_t;
    index = 0;
    // privKey = nullptr;
    
}

PaymentCode PaymentAddress::getPaymentCode() {
    return paymentCode;
}

void PaymentAddress::setPaymentCode(PaymentCode paymentCode_t) {
    paymentCode = paymentCode_t;
}
int PaymentAddress::getIndex() {
    return index;
}

void PaymentAddress::setIndex(int index_t) {
    index = index_t;
}

vector<unsigned char> PaymentAddress::getPrivKey() {
    return privKey;
}

void PaymentAddress::setIndexAndPrivKey(int index_t, vector<unsigned char> privKey_t) {
    index = index_t;
    privKey = privKey_t;
}

void PaymentAddress::setPrivKey(vector<unsigned char> privKey_t) {
    privKey = privKey_t;
}

CPubKey PaymentAddress::getSendECKey()
{
    return getSendECKey(getSecretPoint());
}

CKey PaymentAddress::getReceiveECKey()
{
    return getReceiveECKey(getSecretPoint());
}

GroupElement PaymentAddress::get_sG()
{
    return get_sG(getSecretPoint());
}

SecretPoint PaymentAddress::getSharedSecret() {
    return sharedSecret();
}

Scalar PaymentAddress::getSecretPoint() {
    return secretPoint();
}

GroupElement PaymentAddress::getECPoint() {
    CPubKey pubkey;
    vector<unsigned char> pubkeybytes = paymentCode.addressAt(index).getPubKey();
    pubkey.Set(pubkeybytes.begin(), pubkeybytes.end());
    
    GroupElement ge;
    ge.generate(pubkeybytes.data());
    return ge;
}

std::vector<unsigned char> PaymentAddress::hashSharedSecret() {

    std::vector<unsigned char> shardbytes = getSharedSecret().ECDHSecretAsBytes();
    Scalar scal(shardbytes.data());
    sigma::Params* _ec_params = sigma::Params::get_default();
    GroupElement sg = _ec_params->get_g() * scal;
    
    return shardbytes;
}

GroupElement PaymentAddress::get_sG(Scalar s) {
    sigma::Params* _ec_params = sigma::Params::get_default();
    return _ec_params->get_g() * s;
}

CPubKey PaymentAddress::getSendECKey(Scalar s)
{
    LogPrintf("getSendECKey:SecretPoint = %s\n", s.GetHex());
    
    GroupElement ecPoint = getECPoint();
    LogPrintf("getSendECKey:ecPoint = %s\n", ecPoint.GetHex());
    
    GroupElement sG = get_sG(s);
    GroupElement ecG = ecPoint + sG;
    LogPrintf("getSendECKey:ecG= %s\n", ecG.GetHex());
    
    unsigned char buffer[33] = {0};
    unsigned char* bufferflow = ecG.serialize(buffer);
    LogPrintf("getSendECKey:buffer = %s\n", HexStr(&buffer[0], bufferflow));
    
    CPubKey pkey;
    pkey.Set(&buffer[0], bufferflow);

    return pkey;
}

CKey PaymentAddress::getReceiveECKey(Scalar s)
{
    Scalar privKeyValue(privKey.data());
    Scalar newKeyS = privKeyValue + s;
    CKey pkey;
    vector<unsigned char> ppkeybytes = ParseHex(newKeyS.GetHex());
    pkey.Set(ppkeybytes.begin(), ppkeybytes.end(), true);
    return pkey;
}

SecretPoint PaymentAddress::sharedSecret()
{
    SecretPoint secP(privKey, paymentCode.addressAt(index).getPubKey());
    return secP;
}

secp_primitives::Scalar PaymentAddress::secretPoint()
{
    return secp_primitives::Scalar(hashSharedSecret().data());

}













