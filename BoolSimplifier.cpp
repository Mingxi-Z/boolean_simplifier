#include "BoolSimplifier.hpp"
#include <climits>


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

    std::string lp = "Minimize\nobj: a \nSubject To\n";

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
    // highs.setOptionValue("primal_feasibility_tolerance", "1e-9");
    std::vector<int> allIdx(nVar);
    iota(allIdx.begin(), allIdx.end(), 1);
    formatInputIneqnsAsLP(allIdx);
    glp_read_lp(P, nullptr, tmpFileName);
    // highs.readModel(tmpFileName);
    // numRows = highs.getNumRow();
    numRows = glp_get_num_rows(P);
    
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
    glp_read_lp(P, nullptr, tmpFileName);
    int colNum = glp_get_num_cols(P);


    // highs.readModel(tmpFileName);

    // HighsLp model = highs.getLp();

    // Reverse the row with negative index
    for (int rowIdx : idxToNegate) 
    {   
        double newLo = 0;
        double newUb = 0;
        double lo = glp_get_row_lb(P, rowIdx + 1);
        newUb = (lo == -DBL_MAX) ? DBL_MAX : (lo - offset);
        double uB = glp_get_row_ub(P, rowIdx + 1);
        newLo = (uB == DBL_MAX) ? -DBL_MAX : (uB + offset);
        glp_set_row_bnds(P, rowIdx + 1, GLP_DB, newLo, newUb);
        // model.row_lower_[rowIdx] = (model.row_upper_[rowIdx] + offset); 
        // model.row_upper_[rowIdx] = std::numeric_limits<double>::infinity();
    }

    for (int j = 2; j < colNum + 1; ++j) {
        string name = glp_get_col_name(P, j);
        int lb = glp_get_col_lb(P, j);
        int ub = glp_get_col_ub(P, j);
        glp_set_col_bnds(P, j, GLP_FR, lb, ub);
    }
    idxToNegate = {};
    // highs.passModel(model);

    // HighsStatus s = highs.run();
    int result = glp_exact(P, nullptr);
    //assert(result == 0);

    // HighsModelStatus x = highs.getModelStatus();
    int status = glp_get_status(P);
    if (status == GLP_INFEAS || status == GLP_NOFEAS) 
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