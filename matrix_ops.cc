#include "matrix_ops.hh"

using namespace std::complex_literals;

uint64_t find_nth_work_size_divisor(const uint64_t size, const uint n) {
    uint64_t nth_divisor = size;
    uint step = 0;

    for(uint64_t i=size; i>0; i--) {
        if(step==n) break;
        if((size%i)==0&&(size/i)%2==0) {
            nth_divisor = i;
            step++;
        }
    }

    return nth_divisor;
} 

char* load_source_file(const char* filename) {
    std::ifstream cl_source(filename);
    if(!cl_source.is_open()) {
        std::cout << "Failed to load OpenCL source file" << std::endl;
        return const_cast<char*>("\0");
    }

    cl_source.std::istream::seekg(0, std::ios::end);
    std::streampos file_sz = cl_source.std::istream::tellg();
    cl_source.std::istream::seekg(0, std::ios::beg);

    char* cl_source_code = new char[static_cast<size_t>(file_sz)+1];
    cl_source.std::istream::read(cl_source_code, file_sz);
    cl_source_code[file_sz] = '\0';
    cl_source.std::ifstream::close();
    return cl_source_code;
}

void _fft(double* mat_real, double* mat_imag, uint mat_length, bool inverse) {
    int N = mat_length;
    if (N <= 1) return;

    double* mat_real_even = new double[N / 2];
    double* mat_real_odd = new double[N / 2];
    double* mat_imag_even = new double[N / 2];
    double* mat_imag_odd = new double[N / 2];

    for (int i = 0; i < N / 2; i++) {
        mat_real_even[i] = mat_real[i * 2];
        mat_imag_even[i] = mat_imag[i * 2];
        mat_real_odd[i] = mat_real[i * 2 + 1];
        mat_imag_odd[i] = mat_imag[i * 2 + 1];
    }

    _fft(mat_real_even, mat_imag_even, N / 2, inverse);
    _fft(mat_real_odd, mat_imag_odd, N / 2, inverse);

    double angle = 2.0 * M_PI / N * (inverse ? -1 : 1);
    double cos_angle = cos(angle);
    double sin_angle = sin(angle);

    for (int k = 0; k < N / 2; k++) {
        double ok_real = cos_angle * mat_real_odd[k] - sin_angle * mat_imag_odd[k];
        double ok_imag = cos_angle * mat_imag_odd[k] + sin_angle * mat_real_odd[k];

        double ek_real = mat_real_even[k];
        double ek_imag = mat_imag_even[k];

        mat_real[k] = ek_real + ok_real;
        mat_imag[k] = ek_imag + ok_imag;

        mat_real[k + N / 2] = ek_real - ok_real;
        mat_imag[k + N / 2] = ek_imag - ok_imag;

        if (inverse) {
            mat_real[k] /= 2.0; // Scale down for IFFT
            mat_imag[k] /= 2.0;
            mat_real[k + N / 2] /= 2.0;
            mat_imag[k + N / 2] /= 2.0;
        }
    }

    delete[] mat_real_even;
    delete[] mat_imag_even;
    delete[] mat_real_odd;
    delete[] mat_imag_odd;
}

void _ifft(double* mat_real, double* mat_imag, uint mat_length) {
    _fft(mat_real, mat_imag, mat_length, true);
}

double* _vec_matmul(const double* a, const double* b, const uint sz_i, const uint sz_j) {
    size_t max_work_group_size;
    double* c = (double*)malloc(sizeof(double)*(sz_i*sz_j));
    memset(c, 0, sizeof(double)*sz_i*sz_j);

    cl_int err;
    cl_platform_id platform_id;
    cl_device_id device_id;
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel kernel;
    
    clGetPlatformIDs(1, &platform_id, NULL);
    clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);
    clGetDeviceInfo(device_id, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &max_work_group_size, NULL);
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, NULL);
    queue = clCreateCommandQueueWithProperties(context, device_id, 0, NULL);

    size_t global_work_sz[2] = {sz_i, sz_j};
    size_t local_work_sz[2] {global_work_sz[0], global_work_sz[1]};

    size_t work_group_allocation = 1;
    while(local_work_sz[0]*local_work_sz[1]>max_work_group_size||local_work_sz[0]*local_work_sz[1]==global_work_sz[0]*global_work_sz[1]) {
        local_work_sz[0] = find_nth_work_size_divisor(local_work_sz[0], work_group_allocation);
        local_work_sz[1] = find_nth_work_size_divisor(local_work_sz[1], work_group_allocation);
        work_group_allocation++;
    }
    
    cl_mem mat_a_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(double)*(sz_i*sz_j), (void*)a, NULL);
    cl_mem mat_b_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(double)*(sz_i*sz_j), (void*)b, NULL);
    cl_mem mat_c_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(double)*(sz_i*sz_j), NULL, NULL);

    clEnqueueWriteBuffer(queue, mat_a_buffer, CL_TRUE, 0, sizeof(double)*(sz_i*sz_j), a, 0, NULL, NULL);
    clEnqueueWriteBuffer(queue, mat_b_buffer, CL_TRUE, 0, sizeof(double)*(sz_i*sz_j), b, 0, NULL, NULL);

    char* cl_source_file = load_source_file("fractal.cl");
    const char* cl_source_arr[1] = {cl_source_file};
    program = clCreateProgramWithSource(context, 1, cl_source_arr, NULL, NULL);
    delete[] cl_source_file;
    
    clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
    kernel = clCreateKernel(program, "vec_matmul", NULL);
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &mat_a_buffer);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &mat_b_buffer);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &mat_c_buffer);
    clSetKernelArg(kernel, 3, sizeof(uint), &sz_i);
    clSetKernelArg(kernel, 4, sizeof(uint), &sz_j);

    err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_work_sz, local_work_sz, 0, NULL, NULL);
    if (err != CL_SUCCESS) {
        std::cerr << "Error executing NDRange kernel: " << err << std::endl;
    }
    
    err = clEnqueueReadBuffer(queue, mat_c_buffer, CL_TRUE, 0, sizeof(double)*sz_i*sz_j, c, 0, NULL, NULL);
    if (err != CL_SUCCESS) {
        std::cerr << "Error reading buffer: " << err << std::endl;
    }
    
    clReleaseMemObject(mat_a_buffer);
    clReleaseMemObject(mat_b_buffer);
    clReleaseMemObject(mat_c_buffer);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    return c;
}

double* _vec_pool2x2(double* &mat) {
    double* pooled_mat = (double*)malloc(sizeof(mat)/4);
    memset(pooled_mat, 0, sizeof(mat)/4);
    for(size_t u=0;u<static_cast<size_t>(sizeof(pooled_mat))/2;u++) {
        for(size_t v=0;v<static_cast<size_t>(sizeof(pooled_mat))/2;v++) {
                pooled_mat[u+v]= (mat[2*u+2*v]+mat[2*u+2*v+1]+mat[2*u+1+2*v]
                                +mat[2*u+1+2*v+1])/4;
        }
    }
    return pooled_mat;
}

// Eigen::RowVectorXf conv2d(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> &mat, Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> &kernel) {
//     Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> sparse_mat;
//     Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> out;
//     sparse_mat.resize(kernel.rows()^2, kernel.cols()^2);
//     out.resize(kernel.rows(), kernel.cols());
//     // Sparse input matrix
    
//     // Matmul

//     return out;
// }

// Eigen::RowVectorXf flatten(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> &mat) {
//     Eigen::RowVectorXf flattened_mat;
//     flattened_mat.resize(mat.cols());
//     flattened_mat.setZero();
//     for(int u=mat.rows()-1;u>0;u--) {
//         for(int v=0;v<mat.cols();v++) {
//             flattened_mat(0, v) = flattened_mat(0, v)+mat(u, v); 
//         }
//     }

//     return flattened_mat;
// }

// Eigen::RowVectorXf vectorize(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> &mat) {
//     Eigen::RowVectorXf vector_mat;
//     vector_mat.resize(mat.rows()*mat.cols());
//     int c = 0; 
//     for(int u=0;u<mat.rows();u++) {
//         for(int v=0;v<mat.cols();v++) {
//             vector_mat(0, c) = mat(u, v);
//             c++;
//         }
//     }

//     return vector_mat;
// }

// Eigen::RowVectorXf str_to_decimal(const std::string str) {
//     Eigen::RowVectorXf Decimal1xX;
//     Decimal1xX.resize(str.length());
//     for(int i=0;i<str.length();i++) {
//         Decimal1xX(0, i) = float(str[i])/255.0f;
//     }

//     return Decimal1xX;
// }