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
    ~BoolSimplifier(){
        std::remove(tmpFileName);
    }

    string simplifyBoolExp(void);

    int nVar;
    string source;
    string sDcs;
    std::vector<string> vIneqns;
    
private:
    string findDC(void);
    void formatInputIneqnsAsLP(std::set<int> &idxes);
    string checkSubModel(std::set<int> &idxes);
    bool getNextComb(std::vector<std::pair<int, int>> &allComb, std::set<int> &idxes);
    
    Highs highs;
    const char* tmpFileName = "input.lp";
    const double offset = std::numeric_limits<double>::epsilon();
    std::set<int> idxToNegate;
};

bool BoolSimplifier::getNextComb(std::vector<std::pair<int, int>> &allComb, std::set<int> &idxes) {
    return false;
}

void BoolSimplifier::formatInputIneqnsAsLP(std::set<int> &idxes)
{
    std::ofstream outfile (tmpFileName);

    std::string lp = "Minimize\nobj: x + y\nSubject To\n";

    int cnt = 0;
    for (int i : idxes) 
    {
        if (i < 0) {
            idxToNegate.insert(cnt);
        }
        lp += 'c';
        lp += ('0' + abs(i));
        lp += ": ";
        lp += vIneqns[abs(i) - 1];
        lp += '\n';

        ++cnt;
    } 

    lp += "END\n";

    cout << lp << endl;
    outfile << lp;

    outfile.close();
}

string BoolSimplifier::simplifyBoolExp(void)
{
    findDC();
    return "";
}

string BoolSimplifier::checkSubModel(std::set<int> &idxes)
{
    formatInputIneqnsAsLP(idxes);
    highs.readModel(tmpFileName);

    HighsLp model = highs.getLp();


    for (int rowIdx : idxToNegate) 
    {
        
        model.row_lower_[rowIdx] = model.row_upper_[rowIdx] + offset; 
        cout << "row_upper_" << model.row_upper_[rowIdx] << endl;
        model.row_upper_[rowIdx] = std::numeric_limits<int>::max();
    }
    highs.passModel(model);

    HighsStatus s = highs.run();
    cout << (int) s << endl;
    HighsModelStatus x = highs.getModelStatus();
    cout << (int) x << endl;
    if (x == HighsModelStatus::kInfeasible) 
    {
        return "";
    }
    return "";
}

string BoolSimplifier::findDC(void)
{
    std::vector<std::pair<int, int>> allCombs;
    auto getAllCombs = 
    [&](std::vector<std::pair<int, int>> &allCombs) 
    {
        for (int i = 0; i < highs.getNumRow(); ++i)
        {
            allCombs.emplace_back(i + 1, -(i + 1));
        }   
    };
    getAllCombs(allCombs);
    
    std::set<int> idxes = {1, -2};
    string sDcs = "";
    do {
        sDcs += checkSubModel(idxes);
        
    } while (getNextComb(allCombs, idxes));
        
    return "";
}

int main() {
    BoolSimplifier simp;

    std::cout << "Please enter boolean expression:\n";
    std::getline(std::cin, simp.source);

    MintermCalculator cSource(simp.source);
    if (!cSource.runCompiler())
    {
        std::cout << "Boolean expression is invalid\n" << std::endl;
        return 1;
    }
    simp.nVar = cSource.getSymbolTable().numberOfSymbols(); 
    
    simp.vIneqns.resize(simp.nVar);
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
