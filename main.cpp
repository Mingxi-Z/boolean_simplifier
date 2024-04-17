#include <iostream>
#include <Highs.h>
#include "MCDC/mcdc/Include/mcdc.hpp"
#include "minbool/minbool.h"

int main(int, char**){
    // string source;
    // std::cout << "Please enter boolean expression:\n";
    // std::getline(std::cin, source);
    string s = "a & b & c + !c & d & e";
    MintermCalculator c(s);
    MintermVector v = c.calculate();

    string dc = "!a & !d + !b & !e + a & c & !e + !a & !c & e + b & c & !d + !b & !c & d";
    MintermCalculator d(dc);
    MintermVector dv = d.calculate();
    for (int i : v) {
        std::cout << i << ", ";
    }
    std::cout << std::endl;

    for (int i : dv) {
        std::cout << i << ", ";
    }
    std::cout << std::endl;

    std::vector<uint8_t> on {v.begin(), v.end()};
    std::vector<uint8_t> dcc {dv.begin(), dv.end()};
    auto solution = minbool::minimize_boolean<5>(on, dcc);

    for (auto& term : solution)
        std::cout << term << std::endl;
}
