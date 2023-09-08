#ifndef _COMPLEX_SET_ENCRYPTION_
#define _COMPLEX_SET_ENCRYPTION_

#define CSE_128 128
#define CSE_256 256
#define CSE_512 512
#define CSE_1024 1024
#define CSE_2048 2048

#include <string>
#include <map>
#include "fractal.hh"
#include "matrix_ops.hh"

std::string hex_to_str(const std::string hex);
std::string str_to_hex(const std::string string);
std::string double_to_hex(const double dec, const uint precision);
std::string hex_add(const char a, const char b);
size_t pad_dim_to_divisible(const size_t num, const size_t div);
std::string cse(const std::string string, const uint32_t dim, const size_t digest_length);

#endif