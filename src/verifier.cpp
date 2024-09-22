#include "verifier.h"
#include "eth/rlp.h"

Error Verifier::Verify(const std::vector<uint8_t>& transaction) const {
  if (transaction[0] != 0x01) {
    return Error::kVerifierTransactionIsNotEIP2930;
  }

  std::vector<uint8_t> rlp_data(transaction.begin() + 1, transaction.end());

  eth::RLP::Node decoded_transacion;
  std::size_t taken;
  if (!eth::RLP::Decode(
          {.data = transaction, .offset = 1, .length = transaction.size() - 1},
          decoded_transacion, taken)) {
    return Error::kVerifierRLPDecodeFailed;
  }
  if (taken != transaction.size() - 1) {
    return Error::kVerifierRLPDecodeFailed;
  }

  if (decoded_transacion.type != eth::RLP::NodeType::kList ||
      decoded_transacion.list.size() != 8) {
    return Error::kVerifierRLPInvalid;
  }

  // check chain_id
  const auto chain_id = decoded_transacion.list[0];
  if (chain_id.type != eth::RLP::NodeType::kByteArray) {
    return Error::kVerifierRLPInvalid;
  }

  if (chain_id.byte_array != this->chain_id_bytes_) {
    return Error::kVerifierChainIdInvalid;
  }

  // check to
  const auto to = decoded_transacion.list[4];
  if (to.type != eth::RLP::NodeType::kByteArray) {
    return Error::kVerifierRLPInvalid;
  }

  if (to.byte_array != this->contract_address_) {
    return Error::kVerifierContractAddressInvalid;
  }

  // check value
  const auto value = decoded_transacion.list[5];
  if (value.type != eth::RLP::NodeType::kByteArray) {
    return Error::kVerifierRLPInvalid;
  }

  if (value.byte_array.size() != 0) {
    return Error::kVerifierValueIsNotZero;
  }

  // check access list
  const auto access_list = decoded_transacion.list[7];
  if (access_list.type != eth::RLP::NodeType::kList) {
    return Error::kVerifierRLPInvalid;
  }

  if (access_list.list.size() != 0) {
    return Error::kVerifierAccessListIsNotEmpty;
  }

  return Error::kSuccess;
}