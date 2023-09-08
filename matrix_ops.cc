#include "matrix_ops.hh"

using namespace std::complex_literals;

void fft(double* &mat, int mat_length) {
    int N = mat_length;
    if(N<=1) return;

    double* mat_even = (double*)malloc(sizeof(double) * mat_length/2);
    double* mat_odd = (double*)malloc(sizeof(double) * mat_length/2);
    memset(mat_even, 0, sizeof(double) * mat_length/2);
    memset(mat_odd, 0, sizeof(double) * mat_length/2);

    for(int i=0;i<N/2;i++) {
        mat_even[i] = mat[i*2];
        mat_odd[i] = mat[i*2+1];
    }
    
    fft(mat_even, mat_length/2);
    fft(mat_odd, mat_length/2);
    
    for(int k=0; k<N/2; k++) {
        // std::complex<float> o_k = mat_odd[k]*pow(EPSILON, ( ((-2.0i) *PI*k )/N) );
        // mat[k] = mat_even[k] + o_k;
        // mat[k+N/2] = mat_even[k] - o_k;
    }

    delete[] mat_even;
    delete[] mat_odd;
}

double* _vec_pool2x2(double* &mat, size_t mat_row_length) {
    double* pooled_mat = (double*)malloc(sizeof(mat)/4);
    memset(pooled_mat, 0, sizeof(mat)/4);
    for(int u=0;u<sizeof(pooled_mat)/2;u++) {
        for(int v=0;v<sizeof(pooled_mat)/2;v++) {
                pooled_mat[u+v]= (mat[2*u+2*v]+mat[2*u+2*v+1]+mat[2*u+1+2*v]
                                +mat[2*u+1+2*v+1])/4;
        }
    }
    return pooled_mat;
}

Eigen::RowVectorXf conv2d(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> &mat, Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> &kernel) {
    Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> sparse_mat;
    Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> out;
    sparse_mat.resize(kernel.rows()^2, kernel.cols()^2);
    out.resize(kernel.rows(), kernel.cols());
    // Sparse input matrix
    
    // Matmul

    return out;
}

Eigen::RowVectorXf flatten(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> &mat) {
    Eigen::RowVectorXf flattened_mat;
    flattened_mat.resize(mat.cols());
    flattened_mat.setZero();
    for(int u=mat.rows()-1;u>0;u--) {
        for(int v=0;v<mat.cols();v++) {
            flattened_mat(0, v) = flattened_mat(0, v)+mat(u, v); 
        }
    }

    return flattened_mat;
}

Eigen::RowVectorXf vectorize(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> &mat) {
    Eigen::RowVectorXf vector_mat;
    vector_mat.resize(mat.rows()*mat.cols());
    int c = 0; 
    for(int u=0;u<mat.rows();u++) {
        for(int v=0;v<mat.cols();v++) {
            vector_mat(0, c) = mat(u, v);
            c++;
        }
    }

    return vector_mat;
}

Eigen::RowVectorXf str_to_decimal(const std::string str) {
    Eigen::RowVectorXf Decimal1xX;
    Decimal1xX.resize(str.length());
    for(int i=0;i<str.length();i++) {
        Decimal1xX(0, i) = float(str[i])/255.0f;
    }

    return Decimal1xX;
}