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

void bu_shl(bigunsigned* a_ptr, bigunsigned* b_ptr, uint16_t cnt){
  bu_clear(a_ptr);

  uint16_t wrds = cnt >> 5; // # of whole words to shift
  uint16_t bits = cnt &0x1f;// number of bits in a word to shift

  uint32_t mask1 = 0xffffffff << (BU_BITS_PER_DIGIT - bits); //isolates the bits that may need to shift words
  uint32_t mask2 = 0xffffffff >> bits; //isolates the bits that will not shift words
  uint32_t temp;

  a_ptr->base = 0;
  a_ptr->used = b_ptr->used + wrds;
  uint16_t index = a_ptr->used - 1;

  uint16_t i = 0;
  uint16_t pos = ((b_ptr->used)+(b_ptr->base)-1)%BU_DIGITS;

  while(i++ < b_ptr->used) {
    temp = (b_ptr->digit[pos] & mask1) >> (BU_BITS_PER_DIGIT - bits);
    a_ptr->digit[index] = b_ptr->digit[pos] & mask2; //removes the bits that have shifted registers
    a_ptr->digit[index] <<= bits; //shifts the word
    a_ptr->digit[index+1] |= temp; //moves bits up a word
    pos--; //move down a word
    index--;
  }
}

// Shift in place a bigunsigned by cnt bits to the left
// Example: beef shifted by 4 results in beef0
void bu_shl_ip(bigunsigned* a_ptr, uint16_t cnt) {
  uint16_t wrds = cnt >> 5; // # of whole words to shift
  uint16_t bits = cnt &0x1f;// number of bits in a word to shift

  uint32_t mask1 = 0xffffffff << (BU_BITS_PER_DIGIT - bits); //isolates the bits that may need to shift words
  uint32_t mask2 = 0xffffffff >> bits; //isolates the bits that will not shift words
  uint32_t temp;

  uint16_t i = 0;
  uint16_t pos = ((a_ptr->used)+(a_ptr->base)-1) % BU_DIGITS;

  //note: seg fault happens if base > used (aka wraparound). unsure of cause.

  while(i++ < a_ptr->used) {
    temp = (a_ptr->digit[pos] & mask1) >> (BU_BITS_PER_DIGIT - bits);
    a_ptr->digit[pos] &= mask2; //removes the bits that have shifted registers
    a_ptr->digit[pos] <<= bits; //shifts the word
    a_ptr->digit[pos+1] |= temp; //moves bits up a word
    pos--; //move down a word
  }

  a_ptr->base -= wrds;
  a_ptr->used += wrds;
}

void bu_shr(bigunsigned* a_ptr, bigunsigned* b_ptr, uint16_t cnt) {
  bu_clear(a_ptr);

  uint16_t wrds = cnt >> 5; // # of whole words to shift
  uint16_t bits = cnt &0x1f;// number of bits in a word to shift

  uint32_t mask1 = 0xffffffff >> (BU_BITS_PER_DIGIT - bits); //isolates the bits that may need to shift words
  uint32_t mask2 = 0xffffffff << bits; //isolates the bits that will not shift words
  uint32_t temp;

  uint16_t pos = b_ptr->base + wrds;
  uint16_t index = 0;

  a_ptr->base = 0;

  while(pos < b_ptr->used) {
    temp = ((b_ptr->digit[pos] & mask1) << (BU_BITS_PER_DIGIT - bits));
    a_ptr->digit[index] = b_ptr->digit[pos] & mask2; //removes the bits that have shifted registers
    a_ptr->digit[index] >>= bits; //shifts the word
    if(index != a_ptr->base)
      a_ptr->digit[index-1] |= temp; //move bits down a register
    pos++; //move up a word
    index++;
  }

  a_ptr->used = b_ptr->used - wrds; //shifts number of words used
}

void bu_shr_ip(bigunsigned* a_ptr, uint16_t cnt) {
  uint16_t wrds = cnt >> 5; // # of whole words to shift
  uint16_t bits = cnt &0x1f;// number of bits in a word to shift

  uint32_t mask1 = 0xffffffff >> (BU_BITS_PER_DIGIT - bits); //isolates the bits that may need to shift words
  uint32_t mask2 = 0xffffffff << bits; //isolates the bits that will not shift words
  uint32_t temp;

  a_ptr->base += wrds; //shifts the index of the least significant position
  uint16_t pos = a_ptr->base;
  uint16_t i = 0;

  //note: seg fault happens if base > used (aka: wraparound). unsure of cause.

  while(i++ < a_ptr->used) {
    temp = ((a_ptr->digit[pos] & mask1) << (BU_BITS_PER_DIGIT - bits));
    a_ptr->digit[pos] &= mask2; //removes the bits that have shifted registers
    a_ptr->digit[pos] >>= bits; //shifts the word
    if(pos != a_ptr->base)
      a_ptr->digit[pos-1] |= temp; //move bits down a register
    pos++; //move up a word
  }

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
    nxt = ((uint64_t)b_ptr->digit[b_dig++]) + (uint64_t)(c_ptr->digit[c_dig++]) + carry;
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

void bu_add_ip(bigunsigned *a_ptr, bigunsigned *b_ptr)
{
  uint8_t carry = 0;
  uint64_t nxt;
  uint16_t cnt = 0;
  uint16_t min_used = a_ptr->used <= b_ptr->used
                      ? a_ptr->used : b_ptr->used;
  uint8_t  a_dig = a_ptr->base;
  uint8_t  b_dig = b_ptr->base;

  while (cnt < min_used) {
    nxt = ((uint64_t)a_ptr->digit[a_dig++]) + (uint64_t)(b_ptr->digit[b_dig++]) + carry;
    carry = 0 != (nxt&0x100000000);
    a_ptr->digit[a_dig-1] = (uint32_t)nxt;
    cnt++;
  }

  while (cnt < a_ptr->used && carry) {
    nxt = ((uint64_t)a_ptr->digit[a_dig++]) + carry;
    carry = 0 != (nxt&0x100000000);
    a_ptr->digit[a_dig-1] = (uint32_t)nxt;
    cnt++;
  }

  while (cnt < a_ptr->used) {
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

  while (cnt < BU_DIGITS && carry) {
    a_ptr->digit[a_dig++] = 1;
    carry = 0;
    cnt++;
  }

  a_ptr->used = cnt;
}

// adapted from this: https://stackoverflow.com/questions/1815367/catch-and-compute-overflow-during-multiplication-of-two-large-integers
void bu_mul_digit(bigunsigned *a_ptr, bigunsigned *b_ptr, uint32_t d) {
  bu_clear(a_ptr);

  uint8_t i = b_ptr->base;
  uint8_t prev = 0;
  uint8_t place;
  uint16_t cnt = 0;

  uint64_t nxt;
  uint32_t temp;
  bigunsigned* carry = (bigunsigned*) malloc(sizeof(uint32_t)*BU_DIGITS);
  uint16_t crry_cnt = 1;

  while(cnt < b_ptr->used) {
    nxt = (uint64_t)b_ptr->digit[i++] * d;
    place = i - b_ptr->base;
    temp = (uint32_t)(nxt >> 32);
    if(temp != 0){
      carry->digit[place] = temp;
      crry_cnt += (place - prev);
      prev = place;
    }
    a_ptr->digit[cnt] = (uint32_t)nxt;
    cnt++;
  }
  a_ptr->used = cnt;

  carry->base = 0;
  carry->digit[carry->base] = 0x0;
  carry->used = crry_cnt;
  bu_add_ip(a_ptr, carry);
  free(carry);
}

void bu_mul_digit_ip(bigunsigned *a_ptr, uint32_t d) {
  uint8_t i = a_ptr->base;
  uint8_t place = 0;
  uint8_t prev = 0;
  uint16_t cnt = 0;

  uint64_t nxt;
  uint32_t temp;
  bigunsigned* carry = (bigunsigned*) malloc(sizeof(uint32_t)*BU_DIGITS);
  uint16_t crry_cnt = 1;

  while(cnt < a_ptr->used) {
    nxt = (uint64_t)a_ptr->digit[i] * d;
    temp = (uint32_t)(nxt >> 32);
    a_ptr->digit[i++] = (uint32_t)nxt;
    place = i - a_ptr->base;
    if(temp != 0){
      carry->digit[place] = temp;
      crry_cnt += (place - prev);
      prev = place;
    }
    cnt++;
  }

  carry->base = 0;
  carry->digit[carry->base] = 0x0;
  carry->used = crry_cnt;
  bu_add_ip(a_ptr, carry);
  free(carry);
}

void bu_mul_digit_sh(bigunsigned *a_ptr, bigunsigned *b_ptr, uint32_t d, uint8_t shift) {
  bu_clear(a_ptr);

  uint8_t i = b_ptr->base;
  uint8_t prev = 0;
  uint8_t place;
  uint16_t cnt = 0;

  uint64_t nxt;
  uint32_t temp;
  bigunsigned* carry = (bigunsigned*) malloc(sizeof(uint32_t)*BU_DIGITS);
  uint16_t crry_cnt = 1;

  while(cnt < b_ptr->used) {
    nxt = (uint64_t)b_ptr->digit[i++] * d;
    place = i - b_ptr->base + shift;
    temp = (uint32_t)(nxt >> 32);
    if(temp != 0){
      carry->digit[place] = temp;
      crry_cnt += (place - prev);
      prev = place;
    }
    a_ptr->digit[cnt + shift] = (uint32_t)nxt;
    cnt++;
  }
  a_ptr->used = cnt + shift;

  carry->base = 0;
  for(uint8_t k = 0; k <= shift; k++) {
    carry->digit[k] = 0x0;
  }
  carry->used = crry_cnt;

  bu_add_ip(a_ptr, carry);
  free(carry);
}

void bu_mul(bigunsigned *a_ptr, bigunsigned *b_ptr, bigunsigned *c_ptr) {
  bu_clear(a_ptr);

  bigunsigned* placeholder = (bigunsigned*) malloc(sizeof(uint32_t)*BU_DIGITS);

  uint16_t cnt = 0;

  if(b_ptr->used <= c_ptr->used) //if b is smaller than c
  {
    while(cnt < b_ptr->used)
    {
      bu_mul_digit_sh(placeholder, c_ptr, b_ptr->digit[cnt], cnt);
      bu_add_ip(a_ptr, placeholder);
      cnt++;
    }

    while(cnt < c_ptr->used)
    {
      a_ptr->digit[cnt] += c_ptr->digit[cnt];
      cnt++;
    }
  }
  else //if c is smaller than b
  {
    while(cnt < c_ptr->used)
    {
      bu_mul_digit_sh(placeholder, b_ptr, c_ptr->digit[cnt], cnt);
      bu_add_ip(a_ptr, placeholder);
      cnt++;
    }

    while(cnt < b_ptr->used)
    {
      a_ptr->digit[cnt] += b_ptr->digit[cnt];
      cnt++;
    }
  }
  free(placeholder);
}
// a *= b
void bu_mul_ip(bigunsigned *a_ptr, bigunsigned *b_ptr) {
  bigunsigned* placeholder = (bigunsigned*) malloc(sizeof(uint32_t)*BU_DIGITS);
  bigunsigned* copy_a = (bigunsigned*) malloc(sizeof(uint32_t)*BU_DIGITS);
  bu_cpy(copy_a, a_ptr);
  bu_clear(a_ptr);

  uint16_t cnt = 0;

  if(copy_a->used <= b_ptr->used) //if a is smaller than b
  {
    while(cnt < copy_a->used)
    {
      bu_mul_digit_sh(placeholder, b_ptr, copy_a->digit[cnt], cnt);
      bu_add_ip(a_ptr, placeholder);
      cnt++;
    }

    while(cnt < b_ptr->used)
    {
      a_ptr->digit[cnt] += b_ptr->digit[cnt];
      cnt++;
    }
  }
  else //if c is smaller than b
  {
    while(cnt < b_ptr->used)
    {
      bu_mul_digit_sh(placeholder, copy_a, b_ptr->digit[cnt], cnt);
      bu_add_ip(a_ptr, placeholder);
      cnt++;
    }

    while(cnt < copy_a->used)
    {
      a_ptr->digit[cnt] += copy_a->digit[cnt];
      cnt++;
    }
  }
  free(copy_a);
  free(placeholder);
}

// a = b^2
void bu_sqr(bigunsigned *a_ptr, bigunsigned *b_ptr) {
  bu_mul(a_ptr, b_ptr, b_ptr);
}
// a *= a
void bu_sqr_ip(bigunsigned *a_ptr) {
  bigunsigned* copy_a = (bigunsigned*) malloc(sizeof(uint32_t)*BU_DIGITS);
  bu_cpy(copy_a, a_ptr);
  bu_mul_ip(a_ptr, copy_a);
  free(copy_a);
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
    printf("%8x ", a_ptr->digit[(a_ptr->base+i)%BU_DIGITS]);
  printf("Length: %x\n", bu_len(a_ptr));
}
