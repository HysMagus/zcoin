#ifndef PAYMENTADDRESS_H
#define PAYMENTADDRESS_H

#include "wallet/wallet.h"
#include "PaymentCode.h"

class PaymentAddress
{
private:
    /* data */
    int index ;
    vector<unsigned char> privKey;
    PaymentCode paymentCode;
    
public:
    PaymentAddress(/* args */);
    PaymentAddress(PaymentCode paymentCode_t) ;
    PaymentAddress(PaymentCode paymentCode_t, int index_t, vector<unsigned char> privKey_t): paymentCode(paymentCode_t), index(index_t), privKey(privKey_t) {};
    ~PaymentAddress();
    PaymentCode getPaymentCode() ;
    void setPaymentCode(PaymentCode paymentCode_t) ;
    int getIndex() ;
    void setIndex(int inedx_t);
    vector<unsigned char> getPrivKey();
    void setIndexAndPrivKey(int index, vector<unsigned char> privKey);
    void setPrivKey(vector<unsigned char> privKey);
};

#endif
