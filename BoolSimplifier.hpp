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
    BoolSimplifier() = default;
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

    void simplifyBoolExp(void);

    int nVar = -1;
    int glpCalls = 0;
    string m_source = "";
    string sDcs = "";
    std::vector<string> vIneqns;
    MintermCalculator cSource;

private:
    void findDC(void);
    void formatInputIneqnsAsLP(std::vector<int> &idxes);
    string checkSubModel(std::vector<int> &idxes);
    void getCombs(const std::vector<std::pair<int, int>> &allIdxes);
    string formatDc(const std::vector<int> &idxes);
    bool isTrivSat(int colIdx);
    bool unSatDueToSubSet(void);
    bool varAppearOnce(void);
    void 
    getCombUtil
    (
        std::vector<std::bitset<100>> &unSats,
        std::unordered_set<string> &visited,
        std::vector<int> &current,
        int idx,
        const std::vector<std::pair<int, int>> &allIdxes,
        int lim
    );

    void 
    formatEspressoInput
    (
        MintermVector& mintermVector, 
        MintermVector& dontCareVector, 
        const SymbolTable& symbolTable 
    );

    int numCols = -1;
    int numRows = -1;
    
    glp_prob *P_total = glp_create_prob();
    glp_prob *P = glp_create_prob();
    char tmpFileName[256];
    const double offset = std::numeric_limits<double>::epsilon();
    std::set<int> idxToNegate;
};
