/**
 * RLP Serializer
 * https://github.com/afkamalipour/simple-rlp
 *
 * This library implements the RLP Encoding scheme used by Ethereum.
 * The specifications referred to are available at the URL below. 
 * Spec: https://github.com/ethereum/wiki/wiki/%5BEnglish%5D-RLP
 * 
 * Note: This library does not validate ethereum transactions, 
 * this RLP serializer will serialize anything into the RLP encoding standard.
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

#ifndef __RLP_SERIALIZER_H_
#define __RLP_SERIALIZER_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define RLP_SERIALIZER_VER_MAJOR 1
#define RLP_SERIALIZER_VER_MINOR 0
#define RLP_SERIALIZER_VER_PATCH 0

typedef enum {
  RLP_TYPE_INVALID,
  RLP_TYPE_BYTE_ARRAY,
  // Note as per the spec "positive integers must be represented in big endian binary from with no leading zeroes"
  RLP_TYPE_INT8, // these were not aligned with true byte values to prevent end-user misbehavior
  RLP_TYPE_INT16,
  RLP_TYPE_INT32,
  RLP_TYPE_INT64,
  RLP_TYPE_INT128,
  RLP_TYPE_INT256,
  RLP_TYPE_INT512,
  RLP_TYPE_INT1024,
} RlpType_t;
#define RLP_TYPE_IS_INTEGER_TYPE(x) ((x) >= RLP_TYPE_INT8) && ((x) <= RLP_TYPE_INT1024)


// This is a scatter type
// Use this to set your individual payload/fields
// Create an array of these elements for a list.
typedef struct rlpElement {
  RlpType_t    type;   // type of data pointed to by *buff; note: integers get zeroes trimmed out
  size_t       len;    // length of data field (zeroes included); used by the library to access buffer properly without overrunning it
  const void   *buff;  // the pointer to the data; this can be anything from a byte array to an integer type. (NOTE: big endian format only)
} RlpElement_t;
#define RLP_ELEMENT_INTEGER(i) ((RlpElement_t){ .buff = &(i), .len = sizeof(i), .type = rlp_type_from_size(sizeof(i))})
#define RLP_ELEMENT_BYTEARRAY(d, l) ((RlpElement_t){ .buff = (d), .len = (l), .type = RLP_TYPE_BYTE_ARRAY})

typedef enum {
  ERR_RLP_EUNKNOWN   =  INT8_MIN,   // Unknown failure
  ERR_RLP_EBADARG,                  // Bad argument
  ERR_RLP_EILLEGALMEM,              // Memory access violation (overlapping buffers)
  ERR_RLP_ENOMEM,                   // Not enough memory
  // The commented section of error codes below is reserved for decoding RLP payloads
  // ERR_RLP_EINVAL,                   // Invalid RLP data
  // ERR_RLP_EMSGSIZE,                 // RLP data exceeds size provided (insufficient buffer space)
  // ERR_RLP_ENODATA,                  // Not enough data was provided
  // ERR_RLP_ENOENT,                   // No entry was found
  ERR_RLP_OK         =  0,          // No error
} ERLPError_e;


// Determine the correct RLP integer type based on size
RlpType_t rlp_int_type_from_size(int s);

// Returns length of output in bytes, or a negative error value
int rlp_encode_element(void *rlpEncodedOutput, size_t rlpEncodedOutputLen, const RlpElement_t *const rlpElement);

// Returns length of output in bytes, or a negative error value
int rlp_encode_list(void *rlpEncodedOutput, size_t rlpEncodedOutputLen, const RlpElement_t *const *rlpElementsArr, size_t rplElementsLen);


#endif