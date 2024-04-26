#include "BoolSimplifier.hpp"
#include <cfloat>
#include <bitset>
#include <fstream>


void BoolSimplifier::getCombUtil(
    std::vector<std::bitset<100>> &unSats,
    std::unordered_set<string> &visited,
    std::vector<int> &current,
    int idx,
    const std::vector<std::pair<int, int>> &allIdxes,
    int lim)
{
    if (current.size() > lim)
        return;

    std::bitset<100> curSet(0);
    for (int i : current)
    {
        if (i < 0)
            curSet.set(100 + i);
        else 
            curSet.set(i);
    }

    for (auto &st : unSats) 
    {
        if ((st | curSet) == curSet)
            return;
    }
    
    if (current.size() > 1) 
    {
        string idxToken = formatDc(current);
        if (!visited.count(idxToken)) {
            visited.insert(idxToken);
            string token = checkSubModel(current);
            if (!token.empty())
            {
                unSats.push_back(curSet);
                sDcs += token;
                sDcs += "|";
            }
        }
    }

    if (idx == allIdxes.size())
        return;
    
    // TODO: skip duplicate
    current.push_back(allIdxes[idx].first);
    getCombUtil(unSats, visited, current, idx + 1, allIdxes, lim);
    current.pop_back();
    current.push_back(allIdxes[idx].second);
    getCombUtil(unSats, visited, current, idx + 1, allIdxes, lim);
    current.pop_back();
    getCombUtil(unSats, visited, current, idx + 1, allIdxes, lim);
}

void BoolSimplifier::getCombs(
    const std::vector<std::pair<int, int>> &allIdxes) 
{
    std::vector<int> current;
    std::unordered_set<string> visited;
    std::vector<std::bitset<100>> unSats;
    for (int i = 2; i <= numCols; ++i) {
        getCombUtil(unSats, visited, current, 0, allIdxes, i);
    }
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

    // cout << lp << endl;
    outfile << lp;
    outfile.close();
    
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
        //cout << idx << endl;
        s += (*std::next(table.orderedSymbol.begin(), idx - 1));
        s += " & ";
    }
    s.pop_back();
    s.pop_back();
    return s;
}

// Theorem 3.3
bool BoolSimplifier::unSatDueToSubSet(void)
{
    int size = glp_get_num_rows(P);
    int colNum = glp_get_num_cols(P);

    double *colVal = (double *) malloc(sizeof(double) * (size + 1));
    for (int j = 2; j <= colNum; ++j) {
        int len = glp_get_mat_col(P, j, nullptr, colVal);
        // cout << glp_get_col_name(P, j) << ": ";
        bool found = true;
        for (int i = 1; i < len; ++i) 
        {
            //cout << colVal[i] << ", ";
            if (colVal[i] * colVal[i + 1] < 0)
            {
                found = false;
                break;
            }
        }
        //cout << endl;
        // if there exist rows with coef(x) == 0
        if (found && len < size)
        {
            free(colVal);
            return true;
        }
    }
    free(colVal);
    return false;
}

// Theorem 3.2
bool BoolSimplifier::isTrivSat(int colIdx) 
{
    int size = glp_get_num_rows(P);
    
    double *colVal = (double *) malloc(sizeof(double) * (size + 1));
    int len = glp_get_mat_col(P, colIdx, nullptr, colVal);

    if (len != size)
    {
        free(colVal);
        return false;
    }

    for (int i = 1; i < len; ++i) 
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

// Corollary 3.1
bool BoolSimplifier::varAppearOnce(void) 
{
    int size = glp_get_num_rows(P);
    int colNum = glp_get_num_cols(P);
    double *colVal = (double *) malloc(sizeof(double) * (size + 1));
    
    for (int j = 2; j <= colNum; ++j) {
        int len = glp_get_mat_col(P, j, nullptr, colVal);
        if (len == 1)
            return true;
    }

    return false;
}   

// Call glpk to solve the satisfiability of submodel
string BoolSimplifier::checkSubModel(std::vector<int> &idxes)
{
    string token = formatDc(idxes);

    formatInputIneqnsAsLP(idxes);
    int r = glp_read_lp(P, nullptr, tmpFileName);
    assert(r == 0);
    int colNum = glp_get_num_cols(P);

    for (int j = 2; j <= colNum; ++j) {
        // set variable range from -inf to inf
        glp_set_col_bnds(P, j, GLP_FR, 0, 0);
    }

    // Reverse the row with negative index
    double *rowVal = (double *) malloc(sizeof(double) * (colNum + 1));
    int *rowIdxes = (int *) malloc(sizeof(int) * (colNum + 1)); 
    for (int rowIdx : idxToNegate) 
    {   
        int len = glp_get_mat_row(P, rowIdx + 1, rowIdxes, rowVal); 

        for (int i = 1; i <= len; ++i) {
            rowVal[i] *= -1;
        }

        glp_set_mat_row(P, rowIdx + 1, len, rowIdxes, rowVal);

        double newUb = (glp_get_row_ub(P, rowIdx + 1) + 1e-5) * -1;
        glp_set_row_bnds(P, rowIdx + 1, GLP_UP, 0, newUb);
    }
    free(rowVal);
    free(rowIdxes);
    // Corollary 3.1
    if (varAppearOnce()) {
        idxToNegate.clear();
        return "";
    }    

    // Theorem 3.2
    for (int j = 2; j <= colNum; ++j) {
        //cout << j << ": " << glp_get_col_name(P, j) << endl;
        if (isTrivSat(j)) {
            idxToNegate.clear();
            return "";
        }
    }

    // Theorem 3.3
    if (unSatDueToSubSet()) {
        idxToNegate.clear();
        return "";
    }
    
    // DEBUG
    // for (int i : idxes) {
    //     cout << i << ", ";
    // }
    // cout << endl;

    idxToNegate.clear();
    ++glpCalls;
    int result = glp_interior(P, nullptr);
    //assert(result == 0);
    
    int status = glp_ipt_status(P);
    if (status == GLP_INFEAS || status == GLP_NOFEAS) 
    {
        return token;
    }
    return "";
}
 
// Main routine of finding don't cares
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

// Main simplification function
string BoolSimplifier::simplifyBoolExp(void)
{
    auto startTime = std::chrono::high_resolution_clock::now();
    glp_term_out(GLP_OFF);
    std::vector<int> allIdx(nVar);
    iota(allIdx.begin(), allIdx.end(), 1);
    formatInputIneqnsAsLP(allIdx);
    glp_read_lp(P_total, nullptr, tmpFileName);

    numRows = glp_get_num_rows(P_total);
    numCols = std::min(glp_get_num_cols(P_total) + 1, std::min(numRows, 5));
    // numCols = 5;
    findDC();
    
    // Dummy string to ensure the order of minterm input correct
    string dummyHead = formatDc(allIdx);
    iota(allIdx.begin(), allIdx.end(), - nVar);
    dummyHead += "&" + formatDc(allIdx);
    
    sDcs = dummyHead + "|" + sDcs;

    cout << sDcs << endl;

    MintermCalculator cDcs(sDcs);
    MintermVector vDc = cDcs.calculate();
    //vDc.pop_back();
    MintermVector vExp = cSource.calculate();
    
    std::unordered_set<int> vDcSet(vDc.begin(), vDc.end());
    for (int i = 0; i < vExp.size(); ++i)
    {
        if (vDcSet.count(vExp[i])) {
            vExp.erase(vExp.begin() + i);
            --i;
        }
    }

    // DEBUG
    for (int i : vExp)
    {
        cout << i << " ";
    }
    cout << endl;
    cout << endl;
    for (int i : vDc)
    {
        cout << i << " ";
    }
    cout << endl;

    std::vector<uint16_t> on {vExp.begin(), vExp.end()};
    std::vector<uint16_t> dcc {vDc.begin(), vDc.end()};

    auto solution = minbool::minimize_boolean<16>(on, dcc);
    auto endTime = std::chrono::high_resolution_clock::now();

    for (auto& term : solution)
        std::cout << term << std::endl;
    
    cout << "Duration: " 
        << std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() 
        << " millisecond(s)"
        << endl;
    return "";
}