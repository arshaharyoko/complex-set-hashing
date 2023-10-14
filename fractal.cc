#include "fractal.hh"

double* generate_fractal_set(const uint32_t i_, const uint32_t j_,
                            const double x_translate_, const double y_translate_,
                            const double zoom, const double c_re, const double c_im) {

    size_t max_work_group_size;
    float max_iter = 128.0f;
    double* out_mat = (double*)malloc(sizeof(double) * (i_*j_));
    memset(out_mat, 0, sizeof(double) * (i_*j_));

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

    size_t global_work_sz[2] = {i_, j_};
    size_t local_work_sz[2] {global_work_sz[0], global_work_sz[1]};

    size_t work_group_allocation = 1;
    while(local_work_sz[0]*local_work_sz[1]>max_work_group_size||local_work_sz[0]*local_work_sz[1]==global_work_sz[0]*global_work_sz[1]) {
        local_work_sz[0] = find_nth_work_size_divisor(local_work_sz[0], work_group_allocation);
        local_work_sz[1] = find_nth_work_size_divisor(local_work_sz[1], work_group_allocation);
        work_group_allocation++;
    }

    cl_mem vec_mat_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(double) * (i_*j_), NULL, NULL);

    char* cl_source_file = load_source_file("fractal.cl");
    const char* cl_source_arr[1] = {cl_source_file};
    program = clCreateProgramWithSource(context, 1, cl_source_arr, NULL, NULL);
    delete[] cl_source_file;

    clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
    kernel = clCreateKernel(program, "cl_generate_fractal_set", NULL);
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &vec_mat_buffer); // vec_mat
    clSetKernelArg(kernel, 1, sizeof(int), &i_); // i
    clSetKernelArg(kernel, 2, sizeof(int), &j_); // j
    clSetKernelArg(kernel, 3, sizeof(double), &x_translate_); // x_translate
    clSetKernelArg(kernel, 4, sizeof(double), &y_translate_); // y_translate
    clSetKernelArg(kernel, 5, sizeof(double), &zoom); // zoom
    clSetKernelArg(kernel, 6, sizeof(double), &c_re); // c (complex constant real)
    clSetKernelArg(kernel, 7, sizeof(double), &c_im); // c (complex constant imaginary)
    clSetKernelArg(kernel, 8, sizeof(float), &max_iter); // max_iter
    
    err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_work_sz, local_work_sz, 0, NULL, NULL);
    if (err != CL_SUCCESS) {
        std::cerr << "Error executing NDRange kernel: " << err << std::endl;
    }

    err = clEnqueueReadBuffer(queue, vec_mat_buffer, CL_TRUE, 0, sizeof(double) * (i_*j_), out_mat, 0, NULL, NULL);
    if (err != CL_SUCCESS) {
        std::cerr << "Error reading buffer: " << err << std::endl;
    }
    
    clReleaseMemObject(vec_mat_buffer);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    return out_mat;
}