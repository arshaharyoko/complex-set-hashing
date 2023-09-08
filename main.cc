#include <iostream>
#include "matrix_ops.hh"
#include "fractal.hh"
#include "cse.hh"

using namespace std;

int main() {
    std::string hash = cse("test", 128, 32);
    std::cout<< hash << std::endl;
    return 0;
}