__kernel void cl_generate_fractal_set(__global double* out_mat,
                                      int i_, int j_,
                                      double x_translate_, double y_translate_,
                                      double zoom, double c_re, double c_im, float max_iter) {
    
    const double scale_x = 2.0 / j_ * zoom;
    const double scale_y = 2.0 / i_ * zoom;

    const double x_translate_centered = x_translate_ + (j_ * scale_x / 2.0);
    const double y_translate_centered = y_translate_ + (i_ * scale_y / 2.0);
    
    // Global ID (0) - x: j
    // Global ID (1) - y: i
    // Global SZ (0) - x: j
    // Global SZ (1) - y: i
    int idx = get_global_id(1)*get_global_size(0)+get_global_id(0);
    if(idx<i_*j_) {
        const int i = idx / j_;
        const int j = idx % j_;

        const double x = (j * scale_x) - x_translate_centered;
        const double y = (i * scale_y) - y_translate_centered;

        double z_re = x;
        double z_im = y;

        #pragma unroll
        for(float t=0;t<max_iter;t++) {
            if(t==max_iter-1) out_mat[idx] = 0; 
            if(z_re * z_re + z_im * z_im>8.0f) {
                out_mat[idx] = t / (float)max_iter * 255;
                break;
            }  
            
            double _z_re = (z_re*z_re)-(z_im*z_im)+c_re;
            double _z_im = 2*(z_re*z_im)+c_im;

            z_re = _z_re;
            z_im = _z_im;
        }
    };
}