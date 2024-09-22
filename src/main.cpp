#include <Arduino.h>

#include "error.h"
#include "signer.h"
#include "verifier.h"

#include <string>

#if __has_include("secrets.h")
#include "secrets.h"
#endif

#ifndef SIGNER_KEY
#error No signer key present
#endif

#ifndef CONTRACT_ADDRESS
#error No contract address present
#endif 

#ifndef CHAIN_ID
#error No chain ID present
#endif

std::vector<uint8_t> ParseHexString(const std::string& hex_string) {
  std::vector<uint8_t> data;
  for (std::size_t i = 2; i < hex_string.size(); i += 2) {
    data.push_back(std::stoi(std::string() + hex_string[i] + hex_string[i + 1], nullptr, 16));
  }
  return data;
}

Signer signer(ParseHexString(SIGNER_KEY));
Verifier verifier(ParseHexString(CONTRACT_ADDRESS), ParseHexString(CHAIN_ID));

void setup() {
  Serial.begin(115200);
}

std::vector<uint8_t> ReadBytesFully(Stream& serial, std::size_t size) {
  std::vector<uint8_t> data(size);

  std::size_t read = 0;
  while (read < size) {
    read += serial.readBytes(data.data() + read, size - read);
  }

  return data;
}

uint16_t VectorToUInt16(const std::vector<uint8_t>& data) {
  return (static_cast<uint16_t>(data[0])) |
         (static_cast<uint16_t>(data[1]) << 8);
}

uint32_t VectorToUInt32(const std::vector<uint8_t>& data) {
  return (static_cast<uint32_t>(data[0])) |
         (static_cast<uint32_t>(data[1]) << 8) |
         (static_cast<uint32_t>(data[2]) << 16) |
         (static_cast<uint32_t>(data[3]) << 24);
}

std::vector<uint8_t> UInt32ToVector(uint32_t value) {
  std::vector<uint8_t> data(4);
  data[0] = value & 0xFF;
  data[1] = (value >> 8) & 0xFF;
  data[2] = (value >> 16) & 0xFF;
  data[3] = (value >> 24) & 0xFF;
  return data;
}

std::vector<uint8_t> UInt16ToVector(uint16_t value) {
  std::vector<uint8_t> data(2);
  data[0] = value & 0xFF;
  data[1] = (value >> 8) & 0xFF;
  return data;
}

void WriteError(Stream& stream, Error error) {
  stream.write(0x5A);
  stream.write(0xA5);
  stream.write(static_cast<uint8_t>(error));
}

uint32_t CalculateChecksum(const std::vector<uint8_t>& data) {
  uint32_t crc = 0xFFFFFFFF;
  for (const auto byte : data) {
    crc = crc ^ byte;
    for (uint32_t i = 0; i < 8; i++) {
      crc = (crc >> 1) ^ (0xEDB88320 & (-(crc & 1)));
    }
  }
  return ~crc;
}

void WriteResponse(Stream& stream, const std::vector<uint8_t>& data) {
  stream.write(0x5A);
  stream.write(0xA5);
  stream.write(static_cast<uint8_t>(Error::kSuccess));
  const auto checksum = UInt32ToVector(CalculateChecksum(data));
  const auto data_length = UInt16ToVector(data.size());
  stream.write(data_length.data(), data_length.size());
  stream.write(data.data(), data.size());
  stream.write(checksum.data(), checksum.size());
}

bool WaitForPreamble(Stream& stream) {
  while (ReadBytesFully(stream, 1)[0] != 0x5A) {}
  if (ReadBytesFully(stream, 1)[0] != 0xA5) {
    return false;
  }
  return true;
}

bool VerifyChecksum(const std::vector<uint8_t>& data, uint32_t checksum) {
  return CalculateChecksum(data) == checksum;
}

bool ReadTransaction(Stream& stream, std::vector<uint8_t>& result) {
  if (!WaitForPreamble(stream)) {
    return false;
  }
  const auto transaction_size =
      VectorToUInt16(ReadBytesFully(stream, sizeof(uint16_t)));
  const auto transaction = ReadBytesFully(stream, transaction_size);
  const auto checksum =
      VectorToUInt32(ReadBytesFully(stream, sizeof(uint32_t)));

  if (!VerifyChecksum(transaction, checksum)) {
    return false;
  }

  result = transaction;
  return true;
}

void loop() {
  std::vector<uint8_t> transaction;
  const auto success = ReadTransaction(Serial, transaction);
  if (!success) {
    WriteError(Serial, Error::kSerialReadFailed);
    return;
  }

  if (transaction.size() == 0) {
    WriteError(Serial, Error::kPong);
    return;
  }

  if (transaction.size() > 32768) {
    WriteError(Serial, Error::kTransactionIsTooBig);
    return;
  }

  const auto verifyResult = verifier.Verify(transaction);

  if (verifyResult != Error::kSuccess) {
    WriteError(Serial, verifyResult);
    return;
  }

  WriteResponse(Serial, signer.Sign(transaction));
}