#include "BoolSimplifier.hpp"


bool BoolSimplifier::getNextComb(const std::vector<std::pair<int, int>> &allComb, std::set<int> &idxes) {
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


    // Reverse the row with negative index
    for (int rowIdx : idxToNegate) 
    {
        model.row_lower_[rowIdx] = model.row_upper_[rowIdx] + offset; 
        model.row_upper_[rowIdx] = std::numeric_limits<int>::max();
    }
    highs.passModel(model);

    HighsStatus s = highs.run();
    assert(s == HighsStatus::kOk);

    HighsModelStatus x = highs.getModelStatus();
    if (x == HighsModelStatus::kInfeasible) 
    {
        return formatDc(idxes);
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


string BoolSimplifier::formatDc(const std::set<int> &idxes) {
    for (int idx : idxes)
    return "";
}