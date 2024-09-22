#include "rlp.h"

#include <algorithm>

using namespace eth;

uint16_t ReadInteger(const RLP::ByteSpan& data) {
  if (data.length == 0) {
    return 0;
  }
  if (data.length == 1) {
    return data.data[data.offset];
  }
  if (data.length == 2) {
    return (static_cast<uint16_t>(data.data[data.offset]) << 8) |
           data.data[data.offset + 1];
  }
  return 0;
}

bool DecodeChildren(const RLP::ByteSpan& data, std::vector<RLP::Node>& list) {
  std::size_t current_offset = 0;
  while (current_offset < data.length) {
    RLP::Node node;
    std::size_t taken = 0;
    if (!RLP::Decode({.data = data.data,
                      .offset = data.offset + current_offset,
                      .length = data.length - current_offset},
                     node, taken)) {
      return false;
    }
    list.push_back(node);
    current_offset += taken;
  }
  return true;
}

bool RLP::Decode(const ByteSpan& data, RLP::Node& node, std::size_t& taken) {
  taken = 0;
  if (data.length == 0) {
    return false;
  }
  uint8_t type = data.data[data.offset];
  taken++;
  if (type >= 0xf8) {
    uint8_t lengthLength = type - 0xf7;
    if (lengthLength > 2) {
      return false;
    }

    if (taken + lengthLength > data.length) {
      return false;
    }
    uint16_t length = ReadInteger(
        {.data = data.data, .offset = data.offset + 1, .length = lengthLength});
    taken += lengthLength;

    if (taken + length > data.length) {
      return false;
    }
    node.type = NodeType::kList;
    if (!DecodeChildren({.data = data.data,
                         .offset = data.offset + 1 + lengthLength,
                         .length = length},
                        node.list)) {
      return false;
    }
    taken += length;
  } else if (type >= 0xc0) {
    uint8_t length = type - 0xc0;

    if (taken + length > data.length) {
      return false;
    }
    node.type = NodeType::kList;
    if (!DecodeChildren(
            {.data = data.data, .offset = data.offset + 1, .length = length},
            node.list)) {
      return false;
    }
    taken += length;
  } else if (type >= 0xb8) {
    uint8_t lengthLength = type - 0xb7;
    if (lengthLength > 2) {
      return false;
    }

    if (taken + lengthLength > data.length) {
      return false;
    }
    uint16_t length = ReadInteger(
        {.data = data.data, .offset = data.offset + 1, .length = lengthLength});
    taken += lengthLength;

    if (taken + length > data.length) {
      return false;
    }
    node.type = NodeType::kByteArray;
    node.byte_array =
        std::vector<uint8_t>(data.data.begin() + data.offset + taken,
                             data.data.begin() + data.offset + taken + length);
    taken += length;
  } else if (type >= 0x80) {
    uint8_t length = type - 0x80;
    
    if (taken + length > data.length) {
      return false;
    }
    node.type = NodeType::kByteArray;
    node.byte_array =
        std::vector<uint8_t>(data.data.begin() + data.offset + taken,
                             data.data.begin() + data.offset + taken + length);
    taken += length;
  } else {
    node.type = NodeType::kByteArray;
    node.byte_array = {type};
  }
  return true;
}

static std::vector<uint8_t> EncodeList(const std::vector<RLP::Node>& list) {
  std::vector<uint8_t> data;
  for (const auto node : list) {
    const auto node_data = RLP::Encode(node);
    data.insert(data.end(), node_data.begin(), node_data.end());
  }
  return data;
}

static std::vector<uint8_t> EncodeLength(std::size_t length, uint8_t offset) {
  if (length < 56) {
    std::vector<uint8_t> data;
    data.push_back(length + offset);
    return data;
  }

  std::vector<uint8_t> data;
  while (length) {
    data.push_back(length & 0xFF);
    length >>= 8;
  }
  std::reverse(data.begin(), data.end());

  data.insert(data.begin(), data.size() + offset + 55);
  return data;
}

std::vector<uint8_t> RLP::Encode(const RLP::Node& node) {
  std::vector<uint8_t> data;
  switch (node.type) {
    case NodeType::kList: {
      std::vector<uint8_t> encoded_list = EncodeList(node.list);
      std::vector<uint8_t> encoded_list_integer =
          EncodeLength(encoded_list.size(), 0xc0);
      data.insert(data.end(), encoded_list_integer.begin(),
                  encoded_list_integer.end());
      data.insert(data.end(), encoded_list.begin(), encoded_list.end());
      break;
    }
    case NodeType::kByteArray: {
      if (node.byte_array.size() == 1 && node.byte_array[0] < 0x80) {
        data.push_back(node.byte_array[0]);
        break;
      }
      std::vector<uint8_t> encoded_byte_array_integer =
          EncodeLength(node.byte_array.size(), 0x80);
      data.insert(data.end(), encoded_byte_array_integer.begin(),
                  encoded_byte_array_integer.end());
      data.insert(data.end(), node.byte_array.begin(), node.byte_array.end());
      break;
    }
  }
  return data;
}