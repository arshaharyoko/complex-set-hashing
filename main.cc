#include <iostream>
#include "matrix_ops.hh"
#include "fractal.hh"
#include "cse.hh"

int main() {
    const uint dim = 128;
    Digest digest_1 = cse("test", dim, 16);
    Digest digest_2 = cse("trst", dim, 16);
    std::cout << digest_1.hash << std::endl;
    std::cout << digest_2.hash << std::endl;
    return 0;
}