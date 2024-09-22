#ifndef ETH_KECCAK256_H_
#define ETH_KECCAK256_H_

#include <cstdint>
#include <vector>

namespace eth {

class Keccak256 {
 public:
  static std::vector<uint8_t> Hash(const std::vector<uint8_t>& data);

 private:
  Keccak256() = delete;
};

}  // namespace eth

#endif