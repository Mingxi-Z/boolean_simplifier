#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <Highs.h>
#include "BoolSimplifier.hpp"
#include "MCDC/mcdc/Include/mcdc.hpp"
#include "minbool/minbool.h"

using std::cin;
using std::cout;
using std::endl;

int main() {

    std::cout << "Please enter boolean expression:\n";
    string source;
    std::getline(std::cin, source);

    BoolSimplifier simp(source);

    if (simp.nVar == -1)
        return 1;
    
    cout << "Please enter inequalities in \" x_1 + x_2 <= C \" form \n";
    for (int i = 0; i < simp.nVar; ++i) 
    {
        std::getline(cin, simp.vIneqns[i]);
    } 

    string result = simp.simplifyBoolExp();

    cout << result << endl;
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
