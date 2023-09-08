#ifndef _MAT_OPS_
#define _MAT_OPS_
#include <cmath>
#include <eigen3/Eigen/Core>

void fft(double* &mat, int mat_length);
double* _vec_pool2x2(double* &mat);
Eigen::RowVectorXf conv2d(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> &mat, Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> &kernel);
Eigen::RowVectorXf flatten(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> &mat);
Eigen::RowVectorXf vectorize(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> &mat);
Eigen::RowVectorXf str_to_decimal(const std::string str);

#endif