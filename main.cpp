#include <cstdio>
#include <cfloat>
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
    
    double lo = glp_get_row_lb(P, 1);
    double newUb = 
        ((DBL_MAX - abs(lo)) <= std::numeric_limits<double>::epsilon()) ? DBL_MAX : (lo - 1e-7);
    double uB = glp_get_row_ub(P, 1);
    double newLo = 
        (abs(DBL_MAX - abs(uB)) <= std::numeric_limits<double>::epsilon()) ? -DBL_MAX : (uB + 1e-7);
    glp_set_row_bnds(P, 1, GLP_DB, newLo, newUb);

    double cUb = glp_get_col_ub(P, 2);
    double cLo = glp_get_col_lb(P, 2);

    glp_set_col_bnds(P, 2, GLP_FR, 0, 0);
    cUb = glp_get_col_ub(P, 2);
    cLo = glp_get_col_lb(P, 2);

    result = glp_exact(P, nullptr);
}

int test2(){
    string sExp = "a & b & c & d & e & f & g | i & b & c & h &!f & d & g | b &!a & c & e & f & g | b&c&j&d&k&e&f&g | i&b&c&j&d&e&!f&g";
    MintermCalculator cExp(sExp);
    MintermVector vExp = cExp.calculate();
    string sDc = "";
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

    std::vector<uint16_t> on {vExp.begin(), vExp.end()};
    std::vector<uint16_t> dcc {vDc.begin(), vDc.end()};
    // std::vector<uint8_t> dcc = {0, 1, 2, 4, 5, 6, 8, 9, 12, 13, 16, 18, 20, 22};
    auto solution = minbool::minimize_boolean<11>(on, dcc);


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
    BoolSimplifier *simp = new BoolSimplifier(source);

    if (simp->nVar == -1)
        return 1;
    
    cout << "Please enter inequalities in \" x_1 + x_2 <= C \" form \n";
    for (int i = 0; i < simp->nVar; ++i) 
    {
        std::getline(cin, simp->vIneqns[i]);
    } 
    cout << endl;

    string result = simp->simplifyBoolExp();

    cout << result << endl;

    cout << simp->glpCalls << endl;
    delete simp;
}


