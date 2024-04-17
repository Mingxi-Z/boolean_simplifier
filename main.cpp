#include <iostream>
#include <Highs.h>
#include "MCDC/mcdc/Include/mcdc.hpp"
#include "minbool/minbool.h"

int main() {
    string source;
    std::cout << "Please enter boolean expression:\n";
    std::getline(std::cin, source);

    MintermCalculator cSource(source);
    if (!cSource.runCompiler())
    {
        std::cout << "Boolean expression is invalid\n" << std::endl;
        return 1;
    }
    uint nVar = cSource.getSymbolTable().numberOfSymbols(); 
    std::cout << nVar << std::endl;
    std::vector<string> vIneqns(nVar);
    std::cout << "Please enter inequalities in \" x_1 + x_2 <= C \" form \n";
    for (int i = 0; i < nVar; ++i) 
    {
        std::getline(std::cin, vIneqns[i]);
    } 

}

// int main(int, char**){
//     (void) getInput();
//     string sExp = "a & b & c | !c & d & e";
//     MintermCalculator cExp(sExp);
//     MintermVector vExp = cExp.calculate();

//     string sDc = "!a & !d + !b & !e + a & c & !e + !a & !c & e + b & c & !d + !b & !c & d";
//     MintermCalculator cDc(sDc);
//     MintermVector vDc = cDc.calculate();
//     for (int i : vExp) {
//         std::cout << i << ", ";
//     }
//     std::cout << std::endl;

//     for (int i : vDc) {
//         std::cout << i << ", ";
//     }
//     std::cout << std::endl;

//     std::vector<uint8_t> on {vExp.begin(), vExp.end()};
//     std::vector<uint8_t> dcc {vDc.begin(), vDc.end()};
//     auto solution = minbool::minimize_boolean<5>(on, dcc);

//     for (auto& term : solution)
//         std::cout << term << std::endl;
// }
