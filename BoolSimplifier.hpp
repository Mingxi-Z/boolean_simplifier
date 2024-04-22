#include <string>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <Highs.h>
#include <glpk.h>
#include "MCDC/mcdc/Include/mcdc.hpp"
#include "minbool/minbool.h"

using std::cin;
using std::cout;

using std::endl;
class BoolSimplifier {

public:
    BoolSimplifier(string source): m_source(source), cSource(source){
        if (!cSource.runCompiler())
        {
            std::cout << "Boolean expression is invalid\n" << std::endl;
        }
        else
        {
            nVar = cSource.getSymbolTable().numberOfSymbols(); 
            vIneqns.resize(nVar);
        }
    }
    ~BoolSimplifier(){
        std::remove(tmpFileName);
    }

    string simplifyBoolExp(void);

    int nVar = -1;
    string m_source;
    string sDcs;
    std::vector<string> vIneqns;
    MintermCalculator cSource;

private:
    void findDC(void);
    void formatInputIneqnsAsLP(std::vector<int> &idxes);
    string checkSubModel(std::vector<int> &idxes);
    void getCombs(std::vector<std::vector<int>> &combs, const std::vector<std::pair<int, int>> &allIdxes);
    string formatDc(const std::vector<int> &idxes);
    
    void 
    getCombUtil
    (
        std::vector<std::vector<int>> &combs,
        std::vector<int> &current,
        int idx,
        const std::vector<std::pair<int, int>> &allIdxes
    );

    int numX = 3;
    int numRows = -1;
    Highs highs;
    glp_prob *P = glp_create_prob();
    const char* tmpFileName = "input.lp";
    const double offset = std::numeric_limits<double>::epsilon();
    std::set<int> idxToNegate;
};
