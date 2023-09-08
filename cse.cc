#include "cse.hh"

std::string hex_to_str(const std::string _hex) {
    std::stringstream ss;
    for(int i=0;i<_hex.length();i+=2) {
        std::string x = _hex.substr(i, 2);
        char c = std::stoul(x, nullptr, 16);
        ss << c;
    };

    return ss.str();
}

std::string str_to_hex(const std::string _string) {
    static const char hex_digits[17] = "0123456789ABCDEF";
    std::stringstream ss;
    for(int i=0;i<_string.length();i++) {
        std::string x = _string.substr(i, 1);
        char c = std::stoul(x, nullptr, 10);
        ss << hex_digits[c>>4];
        ss << hex_digits[c&15];
    }

    return ss.str();
}

std::string double_to_hex(const double _dec, const uint precision = 8) {
    static const char hex_digits[17] = "0123456789ABCDEF";
    std::string hex;
    struct {
        uint64_t quotient;
        uint64_t remainder;
    } value;

    if(_dec==0) {
        hex = '0';
    } else {
        // Check for mantissa/significand
        if(floor(_dec)-_dec<0) {
            value.quotient = _dec*pow(16, precision);
            // std::cout << std::fixed;
            // std::cout << std::setprecision(16);
            // std::cout << _dec << " : " << value.quotient << std::endl;
        } else {
            value.quotient = _dec;
        }
        while(value.quotient!=0) {
            value.remainder = std::fmod(value.quotient, 16);
            value.quotient = static_cast<uint64_t>(value.quotient/16);
            //std::cout << value.quotient << std::endl;
            hex.insert(0, 1, hex_digits[value.remainder]);
        }
    }
    // std::cout << hex << std::endl;
    return hex;
}

std::string hex_add(const char a, const char b) {
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
    return double_to_hex(c);
}

size_t pad_dim_to_divisible(const size_t _num, const size_t _div) {
    size_t _n = 0;
    while((_num+_n)%_div!=0) _n++;
    return (_num+_n);
};

std::string cse(const std::string string, const uint32_t dim, const size_t digest_length) {
    double _c_re = sin(string.length());
    double _c_im = cos(string.length());
    // double _c_re = 0.0;
    // double _c_im = 0.8;

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
    delete[] _cs_res;
    std::string digest(digest_length, '0');

    // Compression
    // 1-byte, 8-bit, 2-hex
    // 2-byte, 16-bit, 4-hex
    // 4-byte, 32-bit, 8-hex
    // 8-byte, 64-bit, 16-hex
    // 16-byte, 128-bit, 32-hex
    // 32-byte, 256-bit, 64-hex
    // 64-byte, 512-bit, 128-hex
    uint _cs_step = 1;
    size_t _block_step = 0;
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
            _i_digest[_block_step] += ((double)string[idx]/(idx+1))*(_cs_new[cs_new_allocation*_cs_step+idx-cs_new_allocation*_cs_step]+1);
        }
    }
    
    uint _precision = 8; 
    std::string _i_hex;
    std::cout << "Block Size: " << _block_size << std::endl;
    std::cout << "Digest Length: " << digest_length << std::endl;
    // Convert to Hex and map to output
    for(size_t idx=0;idx<digest_length*_precision;idx++) {
        size_t pos = ((idx/2)*((idx+1)%2)+((idx+3)/2)*(idx%2))%digest_length;
        if(idx%_precision==0) {
            _i_hex = double_to_hex(_i_digest[idx/_precision]);
        }
        
        // NOT CARRYING EXCESS FROM HEX_ADD
        digest.replace(pos, 1, 1, hex_add(digest[pos], _i_hex[idx%_precision])[0]);
    }

    delete[] _i_digest;
    return digest;
}