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

#include "rlp_serializer.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------------- */
/*                             Internal Constants                             */
/* -------------------------------------------------------------------------- */

#define ENABLE_RLP_DEBUG 0
#if defined(ENABLE_RLP_DEBUG) && ENABLE_RLP_DEBUG == 1
#include <stdio.h>
#define DEBUG_PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif

enum rlpProtocolConstants {
  RLP_EXTENDED_LENGTH_THRESHOLD = 55,
  RLP_OFFSET_LIST_SHORT         = 0xC0,
  RLP_OFFSET_LIST_LONG          = 0xF7,
  RLP_OFFSET_ITEM_SHORT         = 0x80,
  RLP_OFFSET_ITEM_LONG          = 0xB7,
};

/* -------------------------------------------------------------------------- */
/*                                  Utilities                                 */
/* -------------------------------------------------------------------------- */

// Quick way to check if buffers are overlapping
static bool rlp_memoverlap( const void *const a, size_t sza, const void *const b, size_t szb) {
    // Cast to uint8_t* type for ability to index bytewise
    const uint8_t *const aAddr = a;
    const uint8_t *const bAddr = b;

    if( (aAddr == bAddr) ||
        ((aAddr < bAddr) && ((aAddr + sza) >= bAddr)) ||
        ((bAddr < aAddr) && ((bAddr + szb) >= aAddr)) ) {
        // Memory regions overlap
        return true;
    } else {
        // Non-overlapping regions
        return false;
    }
}

static inline int rlp_int_size_from_type(RlpType_t t) {
  switch(t) {
    case RLP_TYPE_INT8:
      return 1;
    case RLP_TYPE_INT16:
      return 2;
    case RLP_TYPE_INT32:
      return 4;
    case RLP_TYPE_INT64:
      return 8;
    case RLP_TYPE_INT128:
      return 16;
    case RLP_TYPE_INT256:
      return 32;
    case RLP_TYPE_INT512:
      return 64;
    case RLP_TYPE_INT1024:
      return 128;
    default:
      return ERR_RLP_EBADARG;
  }
}

static inline bool rlp_type_mem_check(size_t buffSz, RlpType_t type) {
  if (RLP_TYPE_IS_INTEGER_TYPE(type))
      return buffSz == rlp_int_size_from_type(type);
    else if (type == RLP_TYPE_BYTE_ARRAY)
      return true;
  // Likely RLP_TYPE_INVALID
  return false;
}

/* -------------------------------------------------------------------------- */
/*                             API Implementation                             */
/* -------------------------------------------------------------------------- */

RlpType_t rlp_int_type_from_size(int s) {
  for(int i = RLP_TYPE_INT8; i <= RLP_TYPE_INT1024; i++) {
    if(s == rlp_int_size_from_type(i))
      return i;
  }
  return ERR_RLP_EBADARG;
}

// Returns length of output in bytes, or a negative error value
int rlp_encode_element(void *rlpEncodedOutput, size_t rlpEncodedOutputLen, const RlpElement_t *const rlpElement)
{
  if(rlpEncodedOutput == NULL || rlpElement == NULL || rlpEncodedOutputLen == 0 || 
     rlpElement->type == RLP_TYPE_INVALID || !rlp_type_mem_check(rlpElement->len, rlpElement->type))
    return ERR_RLP_EBADARG;
  if(rlpEncodedOutputLen < (rlpElement->len + 1)) // extra byte for rlp encoding tag
    return ERR_RLP_ENOMEM;
  if(rlp_memoverlap(rlpEncodedOutput, rlpEncodedOutputLen, rlpElement->buff, rlpElement->len)) // No overlapping memory regions
    return ERR_RLP_EILLEGALMEM;

  uint8_t *rlpOut = (uint8_t *)rlpEncodedOutput;
  uint8_t const *rlpElementBuff = (uint8_t *)rlpElement->buff;
  size_t rlpElementLen = rlpElement->len;
  size_t rlpEncodedLen = 0;

  if(RLP_TYPE_IS_INTEGER_TYPE(rlpElement->type)) {
    const uint8_t *buffBase = rlpElement->buff;
    for(size_t scanZero = 0; scanZero < rlpElement->len; scanZero++) { 
      if(buffBase[scanZero]) {
        rlpElementBuff = (buffBase + scanZero);
        rlpElementLen -= scanZero;
        break;
      } else if (scanZero == rlpElement->len - 1) {
        // If we reach the end and have not yet found a non-zero byte,
        // this field is zero length
        rlpElementLen = 0;
      }
    }
  }
  // Element Header Generation
  if(rlpElementLen == 0) {
    rlpEncodedLen = 1;
    rlpOut[0] = (uint8_t) RLP_OFFSET_ITEM_SHORT;
  }
  else if(rlpElementLen == 1 && (rlpElementBuff[0] == 0x00 || 
          rlpElementBuff[0] < RLP_OFFSET_ITEM_SHORT)) {
    rlpEncodedLen = 1;
    rlpOut[0] = rlpElementBuff[0];
  } 
  else if (rlpElementLen <= RLP_EXTENDED_LENGTH_THRESHOLD) {
    uint8_t length = (uint8_t) (RLP_OFFSET_ITEM_SHORT + rlpElementLen);
    rlpOut[0] = length;
    // Payload
    memcpy(rlpOut + 1, rlpElementBuff, rlpElementLen);
    rlpEncodedLen = rlpElementLen + 1;
  } 
  // Complicated case of needing an extended length byte
  else {
    int tmpLength = rlpElementLen;
    size_t lengthOfLength = (size_t) 0;
    while (tmpLength != 0) {
        ++lengthOfLength;
        tmpLength = tmpLength >> 8;
    }
    if(rlpEncodedOutputLen < (rlpElementLen + lengthOfLength + 1))
      return ERR_RLP_ENOMEM; // buffer is unable to accomodate the encoded payload... 
                             // this should not happen, since we checked above.
    rlpOut[0] = (uint8_t) (RLP_OFFSET_ITEM_LONG + lengthOfLength);
    tmpLength = rlpElementLen;
    for(int i = lengthOfLength; i > 0; --i) {
      rlpOut[i] = (uint8_t) tmpLength;
      tmpLength = tmpLength >> 8;
    }
    // Payload
    memcpy(rlpOut + 1 + lengthOfLength, rlpElementBuff, (rlpElementLen + lengthOfLength + 1));
    rlpEncodedLen = (rlpElementLen + lengthOfLength + 1);
  }
  return rlpEncodedLen; // all was successful, return encoded length.
}

// Returns length of output in bytes, or a negative error value
int rlp_encode_list(void *rlpEncodedOutput, size_t rlpEncodedOutputLen, 
                    const RlpElement_t *const *rlpElementsArr, size_t rplElementsLen)
{
  if( rlpEncodedOutput == NULL || rlpElementsArr == NULL || rlpEncodedOutputLen == 0 )
    return ERR_RLP_EBADARG;
  
  // loop through all elements and determine if sufficient output space is available
  // and there are no memory overlap violations
  size_t spaceRemaining = rlpEncodedOutputLen;
  for(int i = 0; i < rplElementsLen; i++) {
    if(spaceRemaining < (rlpElementsArr[i]->len + 1)) // extra byte for rlp encoding tag
      return ERR_RLP_ENOMEM;
    else
      spaceRemaining -= (rlpElementsArr[i]->len + 1);

    if(rlp_memoverlap(rlpEncodedOutput, rlpEncodedOutputLen, rlpElementsArr[i]->buff, rlpElementsArr[i]->len)) // No overlapping memory regions
      return ERR_RLP_EILLEGALMEM;
  }
  size_t rlpEncodedLen = 0;
  uint8_t *rlpOut = (uint8_t *) rlpEncodedOutput;
  
  // Encode each element
  for(int i = 0; i < rplElementsLen; i++) {
    int ret = rlp_encode_element((rlpOut + rlpEncodedLen), (rlpEncodedOutputLen - rlpEncodedLen), rlpElementsArr[i]);
    DEBUG_PRINTF("retsize == %zu | elementNum == %d\r\n", ret, i);
    if(ret < 0)
      return ret;
    rlpEncodedLen += ret;
  }
  // Calculate list header byte size
  uint8_t listHdrByteCnt = 0;
  for(int byteShifts = rlpEncodedLen; byteShifts > 0; byteShifts = byteShifts >> 8) {
    listHdrByteCnt++;
  }
  if(rlpEncodedLen > RLP_EXTENDED_LENGTH_THRESHOLD)
    listHdrByteCnt++; // additional byte for extended length tag
  // orig
  if((listHdrByteCnt + rlpEncodedLen) > rlpEncodedOutputLen) {
    // ain't enough room for the header
    return ERR_RLP_ENOMEM;
  }
  // first shift everything down to fit the header
  for(int i = (listHdrByteCnt + rlpEncodedLen); i >= listHdrByteCnt ; i--) {
    rlpOut[i] = rlpOut[i - (listHdrByteCnt)];
  }
  if(rlpEncodedLen > RLP_EXTENDED_LENGTH_THRESHOLD) {
    // generate the header
    for(int i = 0; i < listHdrByteCnt; i++) {
      rlpOut[(listHdrByteCnt - 1) - i] = // subtract 1 to turn count into index
        (uint8_t) (rlpEncodedLen >> (8 * i));
    }
    rlpOut[0] = (uint8_t) (RLP_OFFSET_LIST_LONG + (listHdrByteCnt - 1));
  }
  else {
    rlpOut[0] = (uint8_t) (RLP_OFFSET_LIST_SHORT + rlpEncodedLen);
  }
  rlpEncodedLen += listHdrByteCnt;
  return rlpEncodedLen;
}