#ifndef VERIFIER_H_
#define VERIFIER_H_

#include <cstdint>
#include <vector>
#include "error.h"

class Verifier {
 public:
  Verifier(const std::vector<uint8_t>& contract_address,
           const std::vector<uint8_t>& chain_id_bytes)
      : contract_address_(contract_address), chain_id_bytes_(chain_id_bytes) {}

  Error Verify(const std::vector<uint8_t>& transaction) const;

 private:
  std::vector<uint8_t> contract_address_;
  std::vector<uint8_t> chain_id_bytes_;
};

#endif