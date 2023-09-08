#ifndef _FRACTAL_ALGORITHM_
#define _FRACTAL_ALGORITHM_
#include "matrix_ops.hh"

#include <cstdlib>
#include <fstream>
#include <iostream>

#define CL_TARGET_OPENCL_VERSION 300
#include <CL/cl.h>

using namespace std::complex_literals;

uint64_t find_nth_work_size_divisor(const uint64_t size, const uint n);
char* load_source_file(const char* filename);
double* generate_fractal_set(const uint32_t i_, const uint32_t j_,
                            const double x_translate_, const double y_translate_,
                            const double zoom, const double c_re, const double c_im);

#endif