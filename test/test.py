import serial
import struct
import binascii

s = serial.Serial('COM10', 115200)

def initialize(s):
    s.write(b'\x00' * 16)

def checksum(transaction):
    return binascii.crc32(transaction)

def sign_transaction(s, transaction):
    s.write(b'\x5A\xA5')
    s.write(struct.pack('<H', len(transaction)))
    s.write(transaction)
    s.write(struct.pack('<I', checksum(transaction)))
    if s.read(2) != b'\x5A\xA5':
        raise ValueError('Wrong response')
    error = s.read(1)[0]
    if error == 0xFE:
        return None
    if error != 0:
        raise ValueError(f'Signer error {hex(error)}')
    length = struct.unpack('<H', s.read(2))[0]
    result = s.read(length)
    result_checksum = struct.unpack('<I', s.read(4))[0]
    if result_checksum != checksum(result):
        raise ValueError('Wrong checksum')
    return result

print('opened')
initialize(s)
print('initialized')
print(sign_transaction(s, bytes.fromhex('01f88882210504835c84a0825f8d94a7f1b7b98ee6704afb743a0e38c282ca9b850e8280b864ec01413d000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000000000000000000053132333435000000000000000000000000000000000000000000000000000000c0')).hex())