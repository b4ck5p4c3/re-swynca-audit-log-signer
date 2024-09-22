#ifndef SIGNER_ERROR_H_
#define SIGNER_ERROR_H_

#include <cstdint>

enum class Error : uint8_t {
  kSuccess = 0,

  kSerialReadFailed = 1,
  kTransactionIsTooBig = 2,

  kVerifierTransactionIsNotEIP2930 = 0x11,
  kVerifierRLPDecodeFailed = 0x12,
  kVerifierRLPInvalid = 0x13,
  kVerifierChainIdInvalid = 0x14,
  kVerifierContractAddressInvalid = 0x15,
  kVerifierValueIsNotZero = 0x16,
  kVerifierAccessListIsNotEmpty = 0x17,
  kVerifierDataInvalid = 0x18,

  kPong = 0xFE
};

#endif