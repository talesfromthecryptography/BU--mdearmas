#include <string.h> // for memset, etc.
#include <stdio.h>  // for printf

#include "bu.h"

// NOTE: In this code "word" always refers to a uint32_t

// Copy dest = src
void bu_cpy(bigunsigned *dest, bigunsigned *src) {
  uint16_t cnt = src->used;
  dest->used = cnt;
  dest->base = 0;

  // reset upper 0s in dest
  memset(dest->digit, 0, sizeof(uint32_t)*BU_DIGITS-cnt);

  uint8_t i_dest = 0;
  uint8_t i_src = src->base;

  while (cnt-- > 0) {
    dest->digit[i_dest++] = src->digit[i_src++];
  }
}

// Set to 0
void bu_clear(bigunsigned *a_ptr) {
  memset(a_ptr->digit, 0, sizeof(uint32_t)*BU_DIGITS);
  a_ptr->used = 0;
  a_ptr->base = 0;
}

// Shift in place a bigunsigned by cnt bits to the left
// Example: beef shifted by 4 results in beef0
void bu_shl_ip(bigunsigned* a_ptr, uint16_t cnt) {
  uint16_t wrds = cnt >> 5; // # of whole words to shift
  uint16_t bits = cnt &0x1f;// number of bits in a word to shift

  uint32_t mask1 = 0xffffffff << (BU_BITS_PER_DIGIT - bits); //isolates the bits that may need to shift words
  uint32_t mask2 = 0xffffffff >> bits; //isolates the bits that will not shift words

  uint16_t i = 0;
  uint16_t pos = (a_ptr->used)+(a_ptr->base);

  while(i++ < a_ptr->used) {
    a_ptr->digit[pos+1] |= ((a_ptr->digit[pos] & mask1) >> (BU_BITS_PER_DIGIT - bits)); //moves bits up a word
    a_ptr->digit[pos] |= mask2; //removes the bits that have shifted registers
    a_ptr->digit[pos] <<= bits; //shifts the word
    pos--; //move down a word
  }

  //Implement: shift a_ptr->base down the number of whole words, use modulo to make circular(?)

  a_ptr->used += wrds; //Implement: way to keep track if overflow happened (overflow = extra +1)
}

void bu_shr_ip(bigunsigned* a_ptr, uint16_t cnt) {
  uint16_t wrds = cnt >> 5; // # of whole words to shift
  uint16_t bits = cnt &0x1f;// number of bits in a word to shift

  uint32_t mask1 = 0xffffffff >> (BU_BITS_PER_DIGIT - bits); //isolates the bits that may need to shift words
  uint32_t mask2 = 0xffffffff << bits; //isolates the bits that will not shift words
  uint32_t temp;

  uint16_t pos = 0;

  while(pos < a_ptr->used) {
    temp = ((a_ptr->digit[pos] & mask1) << (BU_BITS_PER_DIGIT - bits));
    a_ptr->digit[pos] &= mask2; //removes the bits that have shifted registers
    a_ptr->digit[pos] >>= bits; //shifts the word
    if(pos > 0)
      a_ptr->digit[pos-1] |= temp; //move bits down a register
    pos++; //move up a word
  }

  a_ptr->base += wrds; //shifts the index of the least significant position
  a_ptr->used -= wrds; //shifts number of words used
}

// Produce a = b + c
void bu_add(bigunsigned *a_ptr, bigunsigned *b_ptr, bigunsigned *c_ptr) {
  uint8_t carry = 0;
  uint64_t nxt;
  uint16_t cnt = 0;
  uint16_t min_used = b_ptr->used <= c_ptr->used
                      ? b_ptr->used : c_ptr->used;
  uint8_t  b_dig = b_ptr->base;
  uint8_t  c_dig = c_ptr->base;
  uint8_t  a_dig = 0;

  while (cnt < min_used) {
    nxt = ((uint64_t)b_ptr->digit[b_dig++])
          + (uint64_t)(c_ptr->digit[c_dig++]) + carry;
    carry = 0 != (nxt&0x100000000);
    a_ptr->digit[a_dig++] = (uint32_t)nxt;
    cnt++;
  }

  while (cnt < b_ptr->used && carry) {
    nxt = ((uint64_t)b_ptr->digit[b_dig++]) + carry;
    carry = 0 != (nxt&0x100000000);
    a_ptr->digit[a_dig++] = (uint32_t)nxt;
    cnt++;
  }

  while (cnt < b_ptr->used) {
    a_ptr->digit[a_dig++] = b_ptr->digit[b_dig++];
    cnt++;
  }

  while (cnt < c_ptr->used && carry) {
    nxt = ((uint64_t)c_ptr->digit[c_dig++]) + carry;
    carry = 0 != (nxt&0x100000000);
    a_ptr->digit[a_dig++] = (uint32_t)nxt;
    cnt++;
  }

  while (cnt < c_ptr->used) {
    a_ptr->digit[a_dig++] = c_ptr->digit[c_dig++];
    cnt++;
  }

  while (cnt < BU_DIGITS && carry) {
    a_ptr->digit[a_dig++] = 1;
    carry = 0;
    cnt++;
  }

  a_ptr->base = 0;
  a_ptr->used = cnt;
}

// return the length in bits (should always be less or equal to 32*a->used)
uint16_t bu_len(bigunsigned *a_ptr) {
  uint16_t res = a_ptr->used<<5;
  uint32_t bit_mask = 0x80000000;
  uint32_t last_wrd = a_ptr->digit[a_ptr->base+a_ptr->used-1];

  while (bit_mask && !(last_wrd&bit_mask)) {
    bit_mask >>= 1;
    res--;
  }
  return res;
}

// Read from a string of hex digits
//
// TODO: This is wrong. See the test main.c
//       Modify to resolve 'endian' conflict.
//       Also modify to permit strings to include whitespace
//        that will be ignored. For example, "DEAD BEEF" should
//        be legal input resulting in the value 0xDEADBEEF.

void bu_readhex(bigunsigned * a_ptr, char *s) {
  bu_clear(a_ptr);

  unsigned cnt = 0;
  unsigned pos = strlen(s)-1;
  char *s_ptr = s;
  while (*s_ptr && cnt < BU_MAX_HEX) {
    a_ptr->digit[pos>>3] |= (((uint32_t)hex2bin(*s_ptr)) << ((pos & 0x7)<<2));
    cnt++;
    pos--;
    s_ptr++;
  }
  a_ptr->used = (cnt>>3) + ((cnt&0x7)!=0);
}

//
void bu_dbg_printf(bigunsigned *a_ptr) {
  printf("Used %x\n", a_ptr->used);
  printf("Base %x\n", a_ptr->base);
  uint16_t i = a_ptr->used;
  printf("Digits: ");
  while (i-- > 0)
    printf("%8x ", a_ptr->digit[a_ptr->base+i]);
  printf("Length: %x\n", bu_len(a_ptr));
}
