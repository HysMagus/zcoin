#include "Bip47PaymentChannel.h"
#include "wallet/wallet.h"
#include "Bip47Util.h"
#include "Bip47Address.h"
#include "PaymentAddress.h"


string Bip47PaymentChannel::TAG = "Bip47PaymentChannel";

int Bip47PaymentChannel::STATUS_NOT_SENT = -1;
int Bip47PaymentChannel::STATUS_SENT_CFM = 1;
int Bip47PaymentChannel::LOOKAHEAD = 10;
Bip47PaymentChannel::Bip47PaymentChannel()     
: status(STATUS_NOT_SENT),
currentOutgoingIndex(0),
currentIncomingIndex(-1)
{}

Bip47PaymentChannel::Bip47PaymentChannel(string v_paymentCode)
: status(STATUS_NOT_SENT),
currentOutgoingIndex(0),
currentIncomingIndex(-1)
{
    paymentCode = v_paymentCode;
}

Bip47PaymentChannel::Bip47PaymentChannel(string v_paymentCode, string v_label) {
    paymentCode = v_paymentCode ;
    label = v_label;
}

string Bip47PaymentChannel::getPaymentCode() {
    return paymentCode;
}

void Bip47PaymentChannel::setPaymentCode(string pc) {
    paymentCode = pc;
}

std::list<Bip47Address>& Bip47PaymentChannel::getIncomingAddresses() {
    return incomingAddresses;
}

int Bip47PaymentChannel::getCurrentIncomingIndex() {
    return currentIncomingIndex;
}

void Bip47PaymentChannel::generateKeys(CWallet *bip47Wallet) {
    for(int i = 0; i < LOOKAHEAD; i++)
    {
        PaymentCode pcode(paymentCode);
        PaymentAddress paddr = BIP47Util::getReceiveAddress(bip47Wallet, pcode, i);
        CKey newgenKey = paddr.getReceiveECKey();
        bip47Wallet->importKey(newgenKey);
        
        
//         paddr.getReceiveECKey()
    }
//     for (int i = 0; i < LOOKAHEAD; i++) {
//         ECKey key = BIP47Util.getReceiveAddress(bip47Wallet, paymentCode, i).getReceiveECKey();
//         Address address = bip47Wallet->getAddressOfKey(key);

//         log.debug("New address generated");
//         log.debug(address.tostring());
//         bip47Wallet->importKey(key);
// //            incomingAddresses.add(i, new Bip47Address(address.toString(), i));
//         incomingAddresses.push_back(Bip47Address(address.toString(), i));

//     }

//     currentIncomingIndex = LOOKAHEAD - 1;
}

Bip47Address* Bip47PaymentChannel::getIncomingAddress(string address) {
    for (Bip47Address bip47Address: incomingAddresses) {
        if (bip47Address.getAddress().compare(address)==0) {
            return &bip47Address;
        }
    }
    return nullptr;
}

void Bip47PaymentChannel::addNewIncomingAddress(string newAddress, int nextIndex) {
    //incomingAddresses.add(nextIndex, new Bip47Address(newAddress, nextIndex));
    incomingAddresses.push_back(Bip47Address(newAddress, nextIndex));      
    currentIncomingIndex = nextIndex;
}

string Bip47PaymentChannel::getLabel() {
    return label;
}

void Bip47PaymentChannel::setLabel(string l) {
    label = l;
}

std::list<string>& Bip47PaymentChannel::getOutgoingAddresses() {
    return outgoingAddresses;
}

bool Bip47PaymentChannel::isNotificationTransactionSent() {
    return status == STATUS_SENT_CFM;
}

void Bip47PaymentChannel::setStatusSent() {
    status = STATUS_SENT_CFM;
}

int Bip47PaymentChannel::getCurrentOutgoingIndex() {
    return currentOutgoingIndex;
}

void Bip47PaymentChannel::incrementOutgoingIndex() {
    currentOutgoingIndex++;
}

void Bip47PaymentChannel::addAddressToOutgoingAddresses(string address) {
    outgoingAddresses.push_back(address);
}

void Bip47PaymentChannel::setStatusNotSent() {
    status = STATUS_NOT_SENT;
}
