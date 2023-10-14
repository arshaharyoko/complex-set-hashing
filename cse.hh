#ifndef _COMPLEX_SET_ENCRYPTION_
#define _COMPLEX_SET_ENCRYPTION_

#define CSE_128 128
#define CSE_256 256
#define CSE_512 512
#define CSE_1024 1024
#define CSE_2048 2048

#include <iomanip>
#include <string>
#include <sstream>
#include <cmath>
#include <map>
#include <immintrin.h>
#include "fractal.hh"

struct Digest {
    std::string hash;
    double* key;
};

std::string double_to_hex(const double dec, const uint precision);
std::string hex_add(const char a, const char b, const uint precision);
void array_ceil_simd(double* a);
int ceil_simd(double a);
int bit_rotate_right(int x, size_t n);
int bit_rotate_left(int x, size_t n);
size_t pad_dim_to_divisible(const size_t num, const size_t div);
Digest cse(const std::string string, const uint dim, const size_t digest_length);

#endif