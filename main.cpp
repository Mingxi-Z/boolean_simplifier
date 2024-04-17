#include <iostream>
#include <Highs.h>
#include "MCDC/mcdc/Include/mcdc.hpp"

int main(int, char**){
    string s = "a & b & c";
    MintermCalculator c(s);
    MintermVector v = c.calculate();
    for (int i : v) {
        std::cout << i << ", ";
    }
    std::cout << std::endl;
}
