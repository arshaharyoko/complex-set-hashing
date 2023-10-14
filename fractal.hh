#ifndef _FRACTAL_ALGORITHM_
#define _FRACTAL_ALGORITHM_

#include <cstdlib>
#include <cstring>
#include <iostream>
#include "matrix_ops.hh"

#define CL_TARGET_OPENCL_VERSION 300
#include <CL/cl.h>

double* generate_fractal_set(const uint32_t i_, const uint32_t j_,
                            const double x_translate_, const double y_translate_,
                            const double zoom, const double c_re, const double c_im);

#endif