#include "keccak256.h"

using namespace eth;

const std::size_t kHashLength = 32;
const std::size_t kBlockLength = 200 - kHashLength * 2;
const std::size_t kNumRounds = 24;
const uint8_t kRotation[5][5] = {
    {0, 36, 3, 41, 18},   {1, 44, 10, 45, 2},  {62, 6, 43, 15, 61},
    {28, 55, 25, 21, 56}, {27, 20, 39, 8, 14},
};

uint64_t RotL64(uint64_t x, int i) {
  return ((0U + x) << i) | (x >> ((64 - i) & 63));
}

static void Absorb(uint64_t state[5][5]) {
  uint64_t(*a)[5] = state;
  uint8_t r = 1;  // LFSR
  for (uint32_t i = 0; i < kNumRounds; i++) {
    // Theta step
    uint64_t c[5] = {};
    for (uint32_t x = 0; x < 5; x++) {
      for (uint32_t y = 0; y < 5; y++) {
        c[x] ^= a[x][y];
      }
    }
    for (uint32_t x = 0; x < 5; x++) {
      uint64_t d = c[(x + 4) % 5] ^ RotL64(c[(x + 1) % 5], 1);
      for (uint32_t y = 0; y < 5; y++) {
        a[x][y] ^= d;
      }
    }

    // Rho and pi steps
    uint64_t b[5][5];
    for (uint32_t x = 0; x < 5; x++) {
      for (uint32_t y = 0; y < 5; y++) {
        b[y][(x * 2 + y * 3) % 5] = RotL64(a[x][y], kRotation[x][y]);
      }
    }

    // Chi step
    for (uint32_t x = 0; x < 5; x++) {
      for (uint32_t y = 0; y < 5; y++) {
        a[x][y] = b[x][y] ^ (~b[(x + 1) % 5][y] & b[(x + 2) % 5][y]);
      }
    }

    // Iota step
    for (uint32_t j = 0; j < 7; j++) {
      a[0][0] ^= static_cast<uint64_t>(r & 1) << ((1 << j) - 1);
      r = static_cast<uint8_t>((r << 1) ^ ((r >> 7) * 0x171));
    }
  }
}

std::vector<uint8_t> Keccak256::Hash(const std::vector<uint8_t>& data) {
  uint64_t state[5][5] = {};

  // XOR each message byte into the state, and absorb full blocks
  uint32_t blockOff = 0;
  for (std::size_t i = 0; i < data.size(); i++) {
    uint32_t j = blockOff >> 3;
    state[j % 5][j / 5] ^= static_cast<uint64_t>(data[i])
                           << ((blockOff & 7) << 3);
    blockOff++;
    if (blockOff == kBlockLength) {
      Absorb(state);
      blockOff = 0;
    }
  }

  // Final block and padding
  {
    uint32_t i = blockOff >> 3;
    state[i % 5][i / 5] ^= 0x01ull << ((blockOff & 7) << 3);
    blockOff = kBlockLength - 1;
    uint32_t j = blockOff >> 3;
    state[j % 5][j / 5] ^= 0x80ull << ((blockOff & 7) << 3);
    Absorb(state);
  }

  // Uint64 array to bytes in little endian
  std::vector<uint8_t> hash(kHashLength);

  for (std::size_t i = 0; i < kHashLength; i++) {
    uint32_t j = i >> 3;
    hash[i] = static_cast<uint8_t>(state[j % 5][j / 5] >> ((i & 7) << 3));
  }

  return hash;
}