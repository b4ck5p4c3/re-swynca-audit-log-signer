#ifndef SIGNER_H_
#define SIGNER_H_

#include <cstdint>
#include <vector>

#include "include/secp256k1.h"

class Signer {
 public:
  Signer(const std::vector<uint8_t>& key)
      : key_(key),
        secp256k1_context_(secp256k1_context_create(SECP256K1_CONTEXT_NONE)) {}
  ~Signer();

  std::vector<uint8_t> Sign(const std::vector<uint8_t>& data) const;

 private:
  std::vector<uint8_t> key_;
  secp256k1_context* secp256k1_context_;
};

#endif