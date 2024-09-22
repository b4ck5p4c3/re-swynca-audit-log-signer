#include "signer.h"
#include <algorithm>
#include "eth/keccak256.h"
#include "eth/rlp.h"
#include "include/secp256k1_recovery.h"

const std::size_t kSignatureLength = 32;

Signer::~Signer() {
  secp256k1_context_destroy(this->secp256k1_context_);
}

std::vector<uint8_t> Signer::Sign(const std::vector<uint8_t>& data) const {
  const auto hash = eth::Keccak256::Hash(data);

  secp256k1_ecdsa_recoverable_signature signature;
  secp256k1_ecdsa_sign_recoverable(this->secp256k1_context_, &signature,
                                   hash.data(), this->key_.data(), nullptr,
                                   nullptr);

  std::vector<uint8_t> r(signature.data, signature.data + 32);
  std::reverse(r.begin(), r.end());
  std::vector<uint8_t> s(signature.data + 32, signature.data + 64);
  std::reverse(s.begin(), s.end());
  uint8_t recovery = signature.data[64];

  eth::RLP::Node transaction;
  std::size_t taken;
  eth::RLP::Decode({.data = data, .offset = 1, .length = data.size() - 1},
                   transaction, taken);

  eth::RLP::Node recovery_param;
  recovery_param.type = eth::RLP::NodeType::kByteArray;
  if (recovery != 0) {
    recovery_param.byte_array.push_back(1);
  }
  transaction.list.push_back(recovery_param);

  eth::RLP::Node r_value;
  r_value.type = eth::RLP::NodeType::kByteArray;
  for (std::size_t i = 0; i < kSignatureLength - r.size(); i++) {
    r_value.byte_array.push_back(0);
  }
  r_value.byte_array.insert(r_value.byte_array.end(), r.begin(), r.end());
  transaction.list.push_back(r_value);

  eth::RLP::Node s_value;
  s_value.type = eth::RLP::NodeType::kByteArray;
  for (std::size_t i = 0; i < kSignatureLength - s.size(); i++) {
    s_value.byte_array.push_back(0);
  }
  s_value.byte_array.insert(s_value.byte_array.end(), s.begin(), s.end());
  transaction.list.push_back(s_value);

  std::vector<uint8_t> signed_transaction_data = {data[0]};
  const auto signed_transaction = eth::RLP::Encode(transaction);
  signed_transaction_data.insert(signed_transaction_data.end(),
                                 signed_transaction.begin(),
                                 signed_transaction.end());
  return signed_transaction_data;
}
