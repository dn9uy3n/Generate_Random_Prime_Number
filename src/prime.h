#ifndef __PRIME_H__
#define __PRIME_H__
#include "GInt.h"

//#define kDigits = 256; // max num : 2^(256*8) = 2^2048
// sieve
bool is_prime(gint n, size_t trials);

void generate_prime_number(uint8_t number[], int nbits);

#endif
