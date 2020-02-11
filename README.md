# Simple RLP Serializer
A lightweight and easy to use RLP encoding library.

### What is RLP?
>The purpose of RLP is to encode arbitrarily nested arrays of binary data, and RLP is the main encoding method used to serialize objects in Ethereum. The only purpose of RLP is to encode structure; encoding specific atomic data types (eg. strings, ints, floats) is left up to higher-order protocols; in Ethereum integers must be represented in big endian binary form with no leading zeroes (thus making the integer value zero be equivalent to the empty byte array).

RLP spec: https://github.com/ethereum/wiki/wiki/%5BEnglish%5D-RLP

# How to use the Simple RLP Serializer

### Example:
The below example demonstrates how to define a few elements you wish to RLP encode.
##### [Important Note]: This library does not validate ethereum transactions, this RLP serializer will serialize anything into the RLP encoding standard.
```
// Example arbitrary data
uint8_t myToAddress[] = {
  0xe0, 0xde, 0xfb, 0x92, 0x14,
  0x5f, 0xef, 0x3c, 0x3a, 0x94,
  0x56, 0x37, 0x70, 0x5f, 0xaf,
  0xd3, 0xaa, 0x74, 0xa2, 0x41 };

uint8_t myValue[] = {
  0xde, 0x0b, 0x6b, 0x3a, 0x76, 
  0x40, 0x00, 0x00 };


// Use the provided type to reference the buffer and indicates its size
RlpElement_t addressTo = {
  .buff = myToAddress,
  .len = sizeof(myToAddress) // length is size in bytes
};
RlpElement_t value = {
  .buff = myValue,
  .len = sizeof(myValue)
};

// Create a list to encapsulate multiple elements
RlpElement const *const rlpThisList[] = {&addressTo, &value};

// Encode the list into RLP
uint8_t rlpBuffer[sizeof(myAddress) + sizeof(myValue) + 4]; // extra bytes for headers

int err = rlp_encode_list(rlpBuffer, sizof(rlpBuffer),
                          rlpThisList, sizeof(rlpThisList)/sizeof(rlpThisList), 
                          false); // false to not trim leading zeros.

if(err < 0)
  return err;

```

### If you wish to encode individual elements:
You may use `rlp_encode_element()` which will support character arrays and integers of arbitrary length.
```
// Integer to be serialized
uint32_t num0 = 42;

// Tokenize the data
RlpElement_t t[] = {
  .buff = &num0, .len = sizeof(num0)
};

// Serialize the data
uint8_t rlpRaw[255];
int bufSz = sizeof(rlpRaw);
int err = rlp_encode_element(rlpRaw, bufSz, &t, true);
assert(err >= 0);
```

