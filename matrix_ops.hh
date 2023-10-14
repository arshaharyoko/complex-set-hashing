#ifndef _MAT_OPS_
#define _MAT_OPS_
#include <cmath>
#include <cstring>
#include <fstream>
#include <complex>
#include <vector>
#include <iostream>
// #include <eigen3/Eigen/Core>

#define CL_TARGET_OPENCL_VERSION 300
#include <CL/cl.h>

uint64_t find_nth_work_size_divisor(const uint64_t size, const uint n);
char* load_source_file(const char* filename);
double* _vec_matmul(const double* a, const double* b, const uint sz_i, const uint sz_j);
void _fft(double* mat_real, double* mat_imag, uint mat_length, bool inverse = false);
void _ifft(double* mat_real, double* mat_imag, uint mat_length);
double* _vec_pool2x2(double* &mat);
// Eigen::RowVectorXf conv2d(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> &mat, Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> &kernel);
// Eigen::RowVectorXf flatten(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> &mat);
// Eigen::RowVectorXf vectorize(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> &mat);
// Eigen::RowVectorXf str_to_decimal(const std::string str);

#endif