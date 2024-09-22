# re-swynca-audit-log-signer

Simple hardware ETH transaction signer based on Bluepill (STM32F103*8 with 128KB of Flash)

Supports:
- Transaction validation (target, value, chain id)
- Transaction signing with specific

Used with [re-swynca](https://github.com/b4ck5p4c3/re-swynca) to sign audit-log transactions in Base blockchain

## Secret defines

In file `src/secrets.h` you need to define:
```c
#define SIGNER_KEY "0x123...456" // ETH wallet private key
#define CONTRACT_ADDRESS "0x789...123" // Taget ETH contract address
#define CHAIN_ID "0x2105" // ETH chain id (0x2105 for Base Mainnet)
```