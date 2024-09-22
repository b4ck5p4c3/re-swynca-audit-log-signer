#ifndef ETH_RLP_H_
#define ETH_RLP_H_

#include <cstdint>
#include <vector>

namespace eth {

class RLP {
 public:
  enum class NodeType { kByteArray = 0, kList = 1 };

  struct Node {
    NodeType type;
    std::vector<uint8_t> byte_array;
    std::vector<Node> list;
  };

  struct ByteSpan {
    std::vector<uint8_t> data;
    std::size_t offset;
    std::size_t length;
  };

  static bool Decode(const ByteSpan& data, Node& result, std::size_t& taken);
  static std::vector<uint8_t> Encode(const Node& result);

 private:
  RLP() = delete;
};

}  // namespace eth

#endif