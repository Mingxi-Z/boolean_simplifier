#include "BoolSimplifier.hpp"
#include <cfloat>
#include <fstream>


void BoolSimplifier::getCombUtil(
    std::vector<int> &current,
    int idx,
    const std::vector<std::pair<int, int>> &allIdxes)
{

    if (current.size() > 1) {
        string token = checkSubModel(current);
        if (!token.empty() & !tokenDcs.count(token)) {
            tokenDcs.insert(token);
            sDcs += token;
            sDcs += "+";
        }
    }

    if (current.size() == numX) {
        return;
    }
    
    if (idx == allIdxes.size()) {
        return;
    }
    
    // TODO: 跳过重复的， 例如 (a & !b) == (!a & b)
    current.push_back(allIdxes[idx].first);
    getCombUtil(current, idx + 1, allIdxes);
    current.pop_back();
    current.push_back(allIdxes[idx].second);
    getCombUtil(current, idx + 1, allIdxes);
    current.pop_back();
    getCombUtil(current, idx + 1, allIdxes);
}

void BoolSimplifier::getCombs(
    const std::vector<std::pair<int, int>> &allIdxes) 
{
    std::vector<int> current;
    getCombUtil(current, 0, allIdxes);
}

void BoolSimplifier::formatInputIneqnsAsLP(std::vector<int> &idxes)
{   
    if (std::tmpnam(tmpFileName) == nullptr) {
        return;
    }

    std::ofstream outfile(tmpFileName);

    std::string lp = "Minimize\nobj: z23456 \nSubject To\n";

    int cnt = 0;
    for (int i : idxes) 
    {
        if (i < 0) 
            idxToNegate.insert(cnt);
        
        lp += 'c';
        lp += (std::to_string(abs(i)));
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

    numRows = glp_get_num_rows(P);
    
    findDC();

    MintermCalculator cDcs(sDcs);

    MintermVector vDc = cDcs.calculate();
    MintermVector vExp = cSource.calculate();

    std::unordered_set<int> vDcSet(vDc.begin(), vDc.end());

    for (int i = 0; i < vExp.size(); ++i)
    {
        if (vDcSet.count(vExp[i])) {
            vExp.erase(vExp.begin() + i);
            --i;
        }
    }
    
    std::vector<uint16_t> on {vExp.begin(), vExp.end()};
    std::vector<uint16_t> dcc {vDc.begin(), vDc.end()};

    auto solution = minbool::minimize_boolean<11>(on, dcc);

    for (auto& term : solution)
        std::cout << term << std::endl;
    return "";
}

bool BoolSimplifier::isTrivSat(int colIdx) 
{
    int size = glp_get_num_rows(P);
    
    double *colVal = (double *) malloc(sizeof(double) * (size + 1));
    int len = glp_get_mat_col(P, colIdx, nullptr, colVal);

    if (len == 1)
    {
        free(colVal);
        return true;
    }

    for (int i = 1; i <= len; ++i) 
    {
        if (colVal[i] * colVal[i + 1] < 0)
        {
            free(colVal);
            return false;
        }
    }
    free(colVal);
    return true;
}


// void BoolSimplifier::getCombUtil(
//     std::vector<std::vector<int>> &combs,
//     std::vector<int> &current,
//     int idx,
//     const std::vector<int> &allIdxes)
// {

//     if (current.size() > 1)
//         combs.emplace_back(current.begin(), current.end());

//     if (current.size() == numX) {
//         return;
//     }
    
//     if (idx == allIdxes.size()) {
//         return;
//     }
    
//     // TODO: 跳过重复的， 例如 (a & !b) == (!a & b)
//     current.push_back(allIdxes[idx]);
//     getCombUtil(combs, current, idx + 1, allIdxes);
//     current.pop_back();
//     getCombUtil(combs, current, idx + 1, allIdxes);
// }

// void BoolSimplifier::getCombs(
//     std::vector<std::vector<int>> &combs, 
//     const std::vector<int> &allIdxes) 
// {
//     std::vector<int> current;
//     getCombUtil(combs, current, 0, allIdxes);
// }

// bool BoolSimplifier::subIsDc(std::vector<int> &idxes)
// {
//     std::vector<std::vector<int>> combs;
//     getCombs(combs, idxes);
    
//     for (const auto &comb : combs) {
//         string token = formatDc(comb);
//         if (tokenDcs.count(token))
//             return true;
//     }

//     return false;
// }

string BoolSimplifier::checkSubModel(std::vector<int> &idxes)
{

    formatInputIneqnsAsLP(idxes);
    int r = glp_read_lp(P, nullptr, tmpFileName);
    assert(r == 0);
    int colNum = glp_get_num_cols(P);

    for (int j = 2; j <= colNum; ++j) {
        if (isTrivSat(j)) {
            idxToNegate.clear();
            return "";
        }
    }

    // if (subIsDc(idxes)) {
    //     idxToNegate = {};
    //     return "";
    // }

    for (int j = 2; j <= colNum; ++j) {
        glp_set_col_bnds(P, j, GLP_FR, 0, 0);
    }

    // Reverse the row with negative index
    for (int rowIdx : idxToNegate) 
    {   
        double lo = glp_get_row_lb(P, rowIdx + 1);
        double newUb = 
            ((DBL_MAX - abs(lo)) <= std::numeric_limits<double>::epsilon()) ? DBL_MAX : (lo - 1e-6);
        double uB = glp_get_row_ub(P, rowIdx + 1);
        double newLo = 
            (abs(DBL_MAX - abs(uB)) <= std::numeric_limits<double>::epsilon()) ? -DBL_MAX : (uB + 1e-6);
        glp_set_row_bnds(P, rowIdx + 1, GLP_DB, newLo, newUb);
    }

    idxToNegate.clear();
    ++glpCalls;
    int result = glp_exact(P, nullptr);
    //assert(result == 0);

    int status = glp_get_status(P);
    if (status == GLP_INFEAS || status == GLP_NOFEAS) 
    {
        return formatDc(idxes);
    }
    return "";
}
 
void BoolSimplifier::findDC(void)
{
    auto *allIdxes = new std::vector<std::pair<int, int>>(numRows);
    auto getAllIdxes = 
    [&](std::vector<std::pair<int, int>> &allIdxes) 
    {
        for (int i = 0; i < numRows; ++i)
        {
            allIdxes[i] = {i + 1, -(i + 1)};
        }   
    };
    getAllIdxes(*allIdxes);
    
    sDcs = "";
    std::vector<std::vector<int>> combs;
    getCombs(*allIdxes);
    sDcs.pop_back();
   
    delete allIdxes;
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