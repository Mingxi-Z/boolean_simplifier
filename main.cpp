#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <Highs.h>
#include <glpk.h>
#include "BoolSimplifier.hpp"
#include "MCDC/mcdc/Include/mcdc.hpp"
#include "minbool/minbool.h"

using std::cin;
using std::cout;
using std::endl;

void test_glpk() {
    glp_prob *P = glp_create_prob();
    // glp_smcp *param = nullptr;
    // glp_init_smcp(param);
    glp_read_lp(P, nullptr, "../../ex.lp");
    
    // param->tol_bnd = 1e-9;
    int result = glp_exact(P, nullptr);
    
    cout << glp_get_status(P) << endl;
}

int test2(){
    // (void) getInput();
    string sExp = "a & b & c | !c & d & e";
    MintermCalculator cExp(sExp);
    MintermVector vExp = cExp.calculate();

    string sDc = "!a & !d + !b & !e + a & c & !e + !a & !c & e + b & c & !d + !b & !c & d";
    // a & !b & !e +a & c & !e +!a & b & !d +!a & !b & !c +!a & !b & !d +!a & !b & !e +!a & c & !d +!a & !c & !d +!a & !c & e +!a & !d +!a & !d & e +!a & !d & !e +!a & !d +!b & c & !e +!b & !c & !e +!b & d & !e +!b & !d & !e +!b & !e +c & !d & !e 
    MintermCalculator cDc(sDc);
    MintermVector vDc = cDc.calculate();
    for (int i : vExp) {
        std::cout << i << ", ";
    }
    std::cout << std::endl;

    for (int i : vDc) {
        std::cout << i << ", ";
    }
    std::cout << std::endl;

    std::vector<uint8_t> on {vExp.begin(), vExp.end()};
    std::vector<uint8_t> dcc {vDc.begin(), vDc.end()};
    auto solution = minbool::minimize_boolean<5>(on, dcc);

    for (auto& term : solution)
        std::cout << term << std::endl;
    return 0;
}

int main() {
    // test_glpk();
    std::cout << "Please enter boolean expression:\n";
    string source;
    std::getline(std::cin, source);
    if (source == "test") {
        test_glpk();
        return 0;
    }
    if (source == "test2") {
        test2();
        return 0;
    }
    BoolSimplifier simp(source);

    if (simp.nVar == -1)
        return 1;
    
    cout << "Please enter inequalities in \" x_1 + x_2 <= C \" form \n";
    for (int i = 0; i < simp.nVar; ++i) 
    {
        std::getline(cin, simp.vIneqns[i]);
    } 
    cout << endl;

    string result = simp.simplifyBoolExp();

    cout << result << endl;
}


