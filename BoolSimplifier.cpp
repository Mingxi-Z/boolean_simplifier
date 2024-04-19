#include "BoolSimplifier.hpp"


void BoolSimplifier::getCombUtil(
    std::vector<std::vector<int>> &combs,
    std::vector<int> &current,
    int idx,
    const std::vector<std::pair<int, int>> &allIdxes)
{

    if (current.size() > 1)
        combs.emplace_back(current.begin(), current.end());

    if (current.size() == numX) {
        return;
    }
    
    if (idx == allIdxes.size()) {
        return;
    }
    
    // TODO: 跳过重复的， 例如 (a & !b) == (!a & b)
    current.push_back(allIdxes[idx].first);
    getCombUtil(combs, current, idx + 1, allIdxes);
    current.pop_back();
    current.push_back(allIdxes[idx].second);
    getCombUtil(combs, current, idx + 1, allIdxes);
    current.pop_back();
    getCombUtil(combs, current, idx + 1, allIdxes);
}

void BoolSimplifier::getCombs(
    std::vector<std::vector<int>> &combs, 
    const std::vector<std::pair<int, int>> &allIdxes) 
{
    std::vector<int> current;
    getCombUtil(combs, current, 0, allIdxes);
}

void BoolSimplifier::formatInputIneqnsAsLP(std::vector<int> &idxes)
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
    std::vector<int> allIdx(nVar);
    iota(allIdx.begin(), allIdx.end(), 1);
    formatInputIneqnsAsLP(allIdx);
    highs.readModel(tmpFileName);
    numRows = highs.getNumRow();

    findDC();

    MintermCalculator cDcs(sDcs);

    MintermVector vDc = cDcs.calculate();
    MintermVector vExp = cSource.calculate();

    std::vector<uint8_t> on {vExp.begin(), vExp.end()};
    std::vector<uint8_t> dcc {vDc.begin(), vDc.end()};
    // dcc.erase(dcc.begin() + 3);
    // dcc.pop_back();

    auto solution = minbool::minimize_boolean<5>(on, dcc);

    for (auto& term : solution)
        std::cout << term << std::endl;
    return "";
}

string BoolSimplifier::checkSubModel(std::vector<int> &idxes)
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
    idxToNegate = {};
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
 
void BoolSimplifier::findDC(void)
{
    std::vector<std::pair<int, int>> allIdxes;
    auto getAllIdxes = 
    [&](std::vector<std::pair<int, int>> &allIdxes) 
    {
        for (int i = 0; i < numRows; ++i)
        {
            allIdxes.push_back({i + 1, -(i + 1)});
        }   
    };
    getAllIdxes(allIdxes);
    
    //std::set<int> idxes = {1, -2};
    sDcs = "";

    std::vector<std::vector<int>> combs;
    getCombs(combs, allIdxes);

    for (auto idxes : combs) 
    {
        for (int i : idxes) 
        {
            cout << i << ",";
        }
        cout << endl;
        string token = checkSubModel(idxes);
        if (token.empty()) {
            continue;
        }
        sDcs += token;
        sDcs += "+";
    }
    
    sDcs.pop_back();
}


string BoolSimplifier::formatDc(const std::vector<int> &idxes) 
{
    SymbolTable table = cSource.getSymbolTable();
    
    string s = "";
    for (int idx : idxes) 
    {
        if (idx < 0) 
        {
            s += '!';
            idx *= -1;
        }
        cout << idx << endl;
        s += (*std::next(table.orderedSymbol.begin(), idx - 1));
        s += " & ";
    }
    s.pop_back();
    s.pop_back();
    return s;
}