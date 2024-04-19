#include <string>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <Highs.h>
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
    string findDC(void);
    void formatInputIneqnsAsLP(std::set<int> &idxes);
    string checkSubModel(std::set<int> &idxes);
    bool getNextComb(const std::vector<std::pair<int, int>> &allComb, std::set<int> &idxes);
    string formatDc(const std::set<int> &idxes);

    Highs highs;
    const char* tmpFileName = "input.lp";
    const double offset = std::numeric_limits<double>::epsilon();
    std::set<int> idxToNegate;
};
