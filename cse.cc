#include "cse.hh"

uint hex_to_dec(const char a) {
    std::map<char, int> m{
        {'0', 0},
        {'1', 1},
        {'2', 2},
        {'3', 3},
        {'4', 4},
        {'5', 5},
        {'6', 6},
        {'7', 7},
        {'8', 8},
        {'9', 9},
        {'A', 10},
        {'B', 11},
        {'C', 12},
        {'D', 13},
        {'E', 14},
        {'F', 15}
    };

    return m[a];
} 

std::string double_to_hex(const double _dec, const uint _precision = 8) {
    static const char hex_digits[17] = "0123456789ABCDEF";
    const double abs_dec = abs(_dec);
    std::string hex;
    struct {
        int64_t quotient;
        int64_t remainder;
    } value;

    if(abs_dec==0) {
        hex = "0";
    } else {
        // Check for mantissa/significand
        if(floor(abs_dec)-abs(abs_dec)<0) {
            value.quotient = abs_dec*pow(16, _precision);
            
            // std::cout << std::fixed;
            // std::cout << std::setprecision(16);
            // std::cout << _dec << " : " << value.quotient << std::endl;
        } else {
            value.quotient = abs_dec;
        }

        while(value.quotient!=0) {
            value.remainder = std::fmod(value.quotient, 16);
            value.quotient = static_cast<int64_t>(value.quotient/16);
            hex.insert(0, 1, hex_digits[value.remainder]);
        }
    }

    while(hex.length()<_precision) {
        hex += '0';
    }
        
    return hex;
}

std::string hex_add(const char a, const char b, const uint _precision = 8) {
    std::map<char, int> m{
        {'0', 0},
        {'1', 1},
        {'2', 2},
        {'3', 3},
        {'4', 4},
        {'5', 5},
        {'6', 6},
        {'7', 7},
        {'8', 8},
        {'9', 9},
        {'A', 10},
        {'B', 11},
        {'C', 12},
        {'D', 13},
        {'E', 14},
        {'F', 15}
    };

    int c = m[a]+m[b];
    return double_to_hex(c, _precision);
}

void array_ceil_simd(double* a) {
    for(size_t idx=0;idx<sizeof(a);idx+=2) {
        __m128d b = _mm_loadu_pd(&a[idx]);
        __m128d r = _mm_ceil_pd(b);
        _mm_storeu_pd(&a[idx], r);
    }
}

int ceil_simd(double a) {
    __m128d b = _mm_load_sd(&a);
    __m128d r = _mm_ceil_sd(b, b);
    return _mm_cvtsd_si64(r);
}

int bit_rotate_right(int x, size_t n) {
    static size_t bits = sizeof(int);
    return (x>>n) | (x<<(bits-n));
}

int bit_rotate_left(int x, size_t n) {
    static size_t bits = sizeof(int);
    return (x<<n) | (x>>(bits-n));
}

size_t pad_dim_to_divisible(const size_t _num, const size_t _div) {
    size_t _n = 0;
    while((_num+_n)%_div!=0) _n++;
    return (_num+_n);
};

void n_hex_add_carry(std::string &str, const char x, const uint pos, const size_t length) {
    std::string carry;
    carry = hex_add(str[pos], x);

    if(hex_to_dec(carry[0])+hex_to_dec(carry[1])!=hex_to_dec(carry[0])) {
        for(uint idx=pos;idx<length;idx++) {
            if(idx+1==length) break;
            carry = hex_add(str[idx+1], carry[1]);
            
            if(hex_to_dec(carry[0])+hex_to_dec(carry[1])!=hex_to_dec(carry[0])) {
                str.replace(idx, 1, 1, carry[1]);
            } else {
                str.replace(idx, 1, 1, carry[0]);
                break;
            }
        }
    } else{
        str.replace(pos, 1, 1, carry[0]);
    }
    
    return;
}

Digest cse(const std::string string, const uint dim, const size_t digest_length) {
    Digest _digest;
    uint _c_mean = 0;
    for(uint acc=0;acc<string.length();acc++) {
        _c_mean += string[acc];
    }

    _c_mean /= string.length();
    double _c_re = sin(string.length()^_c_mean);
    double _c_im = cos(string.length()^_c_mean);

    double* _cs_res = generate_fractal_set(dim, dim, 0.0, 0.0, 1.0, _c_re, _c_im);
    double _cs_avg = 0;
    for(uint32_t idx=0;idx<dim*dim;idx++) {
        _cs_avg += _cs_res[idx];
    }
    _cs_avg = _cs_avg/(dim*dim);
    
    size_t cs_new_allocation = 0;
    for(uint32_t idx=0;idx<dim*dim;idx++) {
        if(_cs_res[idx]>_cs_avg) cs_new_allocation++;
    }
    double* _cs_new = (double*)malloc(sizeof(double) * cs_new_allocation);
    memset(_cs_new, 0, sizeof(double) * cs_new_allocation);

    size_t cs_new_allocate = 0;
    for(uint32_t idx=0;idx<dim*dim;idx++) {
        if(_cs_res[idx]>_cs_avg) {
            _cs_new[cs_new_allocate] = _cs_res[idx];
            cs_new_allocate++;
        } 
    }
    array_ceil_simd(_cs_new);
    // for(size_t idx=0;idx<sizeof(_cs_new)/sizeof(double);idx++) {
    //    std::cout << _cs_new << std::endl; 
    // }

    // Compression
    // 1-byte, 8-bit, 2-hex
    // 2-byte, 16-bit, 4-hex
    // 4-byte, 32-bit, 8-hex
    // 8-byte, 64-bit, 16-hex
    // 16-byte, 128-bit, 32-hex
    // 32-byte, 256-bit, 64-hex
    // 64-byte, 512-bit, 128-hex
    uint _cs_step = 0;
    size_t _block_step = -1;
    size_t _block_size = digest_length;
    if(string.length()>digest_length) {
        _block_size = pad_dim_to_divisible(string.length(), digest_length);
    }
    
    double* _i_digest = (double*)malloc(sizeof(double) * digest_length);
    memset(_i_digest, 0, sizeof(double) * digest_length);
    for(size_t idx=0;idx<_block_size;idx++) {
        // If exceeds the generated fractal data
        if(idx>cs_new_allocation*_cs_step) _cs_step++;

        // If exceeds size 
        if(idx%(_block_size/digest_length)==0) _block_step++;
        if(idx>digest_length) _block_step = 0;

        if(idx>string.length()) {
            _i_digest[_block_step] += 0;
        } else {        
            _i_digest[_block_step] += bit_rotate_right(ceil_simd(string[idx]/(idx+1)),(_cs_new[cs_new_allocation*_cs_step+idx-cs_new_allocation*_cs_step]+1));
        }
    }

    uint _precision = 8;
    std::string digest(digest_length, '0');
    std::string _i_hex;
    std::string _i_hex_add;
    // std::cout << "Block Size: " << _block_size << std::endl;
    // std::cout << "Digest Length: " << digest_length << std::endl;
    // Convert to Hex and map to output
    for(size_t idx=0;idx<digest_length*_precision;idx++) {
        size_t pos = ((idx/2)*((idx+1)%2)+((idx+3)/2)*(idx%2))%digest_length;
        if(idx%_precision==0) {
            // std::cout << std::fixed;
            // std::cout<< std::setprecision(2) << double_to_hex(std::stod(std::to_string(_i_digest[idx/_precision]).substr(0,4))) <<" : "<<double_to_hex(_i_digest[idx/_precision]) <<std::endl;
            _i_hex = double_to_hex(std::stoi(std::to_string(_i_digest[idx/_precision]).substr(0,4))^std::stoi(std::to_string(_i_digest[(idx+_precision)/_precision]).substr(0,4)));
        }
        
        if(_i_hex=="00000000") {
            idx += _precision-1;
        } else {
            n_hex_add_carry(digest, _i_hex[idx%_precision], pos, digest.length());
        }
    }
    
    _digest.hash = digest;
    _digest.key = (double*)malloc(dim*dim*sizeof(double));
    memcpy(_digest.key, _cs_res, dim*dim*sizeof(double));
    free(_i_digest);
    free(_cs_new);
    free(_cs_res);
    return _digest;
}