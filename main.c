/**
 * RLP Serializer
 * https://github.com/afkamalipour/simple-rlp
 *
 * This library implements the RLP Encoding scheme used by Ethereum.
 * The specifications referred to are available at the URL below. 
 * Spec: https://github.com/ethereum/wiki/wiki/%5BEnglish%5D-RLP
 * 
 */

/**
 * MIT License
 *
 * Copyright (c) 2020 Aurash Kamalipour <afkamalipour@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "rlp_serializer.h"
#include <stdio.h>

/*
00000000
47868c0000000000
0000c350
e242e54155b1abc71fc118065270cecaaf8b77683b9aca
*/
// Example of Ethereum txn using library
uint8_t _nonce[] = {0x00, 0x00, 0x00, 0x00};
RlpElement_t nonce = {
  .buff = _nonce,
  .len = sizeof(_nonce),
  .type = RLP_TYPE_INT32
};
uint8_t _gasPrice[] = {0x0f, 0x42, 0x40};
RlpElement_t gasPrice = {
  .buff = _gasPrice,
  .len = 4,
  .type = RLP_TYPE_INT32
};
uint8_t _gasLimit[] = {0x3b, 0x9a, 0xca, 0x00};
RlpElement_t gasLimit = {
  .buff = _gasLimit,
  .len = 4,
  .type = RLP_TYPE_INT32
};
uint8_t _addressTo[] = {0xe0, 0xde, 0xfb, 0x92, 0x14, 0x5f, 0xef, 0x3c, 0x3a, 0x94, 0x56, 0x37, 0x70, 0x5f, 0xaf, 0xd3, 0xaa, 0x74, 0xa2, 0x41};
RlpElement_t addressTo = {
  .buff = _addressTo,
  .len = sizeof(_addressTo),
  .type = RLP_TYPE_BYTE_ARRAY
};
uint8_t _value[] = {0xde, 0x0b, 0x6b, 0x3a, 0x76, 0x40, 0x00, 0x00};
RlpElement_t value = {
  .buff = _value,
  .len = sizeof(_value),
  .type = RLP_TYPE_BYTE_ARRAY
};
uint8_t _data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
RlpElement_t data = {
  .buff = _data,
  .len = sizeof(_data),
  .type = RLP_TYPE_BYTE_ARRAY
};
uint8_t _v[] = {0x01};
RlpElement_t v = {
  .buff = NULL,
  .len = 0,
  .type = RLP_TYPE_BYTE_ARRAY
};
RlpElement_t r = {
  .buff = NULL,
  .len = 0,
  .type = RLP_TYPE_BYTE_ARRAY
};
RlpElement_t s = {
  .buff = NULL,
  .len = 0,
  .type = RLP_TYPE_BYTE_ARRAY
};

// Gather
RlpElement_t const *const ethTxn[] = {&nonce, &gasPrice, &gasLimit, &addressTo, &value, &data, &v, &r, &s};


static void debug_hexstring(const void *const address, size_t nBytes)
{
     uint8_t *a = ((uint8_t*)(address));
     unsigned long int b = ((volatile unsigned long int)(nBytes));
     if ((NULL == a) || (b <= 0)) return;
     for (unsigned long int i = 1; i <= b; i++) {
       if(a[i-1] < 0x10)
          printf("0%x",a[i-1]);
        else
          printf("%x",a[i-1]);
     }
}

int main() {
  uint8_t rlpTx[2048] = {0};
  int outputLen = 0;
  outputLen = rlp_encode_list(rlpTx, sizeof(rlpTx)/sizeof(rlpTx[0]), ethTxn, sizeof(ethTxn)/sizeof(ethTxn[0]));
  if(outputLen < 0)
    printf("error, return code: %d\r\n", outputLen);
  else {
    printf("RLP encoded eth txn [%d B]:\r\n", outputLen);
    debug_hexstring(rlpTx, outputLen);
  }
  return 0;
}