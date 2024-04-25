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

    if (current.size() == numCols) {
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

    // cout << lp << endl;
    outfile << lp;
    outfile.close();
    
}

bool BoolSimplifier::getCombUtil(
    std::vector<int> &current,
    int idx,
    const std::vector<int> &allIdxes)
{

    if (current.size() > 1)
    {
        string token = formatDc(current);
        if (tokenDcs.count(token))
            return true;
    }

    if (current.size() == numCols) {
        return false;
    }
    
    if (idx == allIdxes.size()) {
        return false;
    }
    
    // TODO: 跳过重复的， 例如 (a & !b) == (!a & b)
    current.push_back(allIdxes[idx]);
    if (getCombUtil(current, idx + 1, allIdxes))
        return true;
    current.pop_back();
    return getCombUtil(current, idx + 1, allIdxes);
}

bool BoolSimplifier::subIsDc(std::vector<int> &idxes)
{
    std::vector<int> current;
    return getCombUtil(current, 0, idxes);
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

// Theorem 3.2
bool BoolSimplifier::isTrivSat(int colIdx) 
{
    int size = glp_get_num_rows(P);
    
    double *colVal = (double *) malloc(sizeof(double) * (size + 1));
    int len = glp_get_mat_col(P, colIdx, nullptr, colVal);

    // if (len == 1)
    // {
    //     free(colVal);
    //     return true;
    // }

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

    formatInputIneqnsAsLP(idxes);
    int r = glp_read_lp(P, nullptr, tmpFileName);
    assert(r == 0);
    int colNum = glp_get_num_cols(P);

    // TODO: Theorem 3.3
    // if (subIsDc(idxes)) {
    //     idxToNegate.clear();
    //     return "";
    // }

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

        double newUb = (glp_get_row_ub(P, rowIdx + 1) + 1e-8) * -1;
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
        if (isTrivSat(j)) {
            idxToNegate.clear();
            return "";
        }
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
    glp_term_out(GLP_OFF);
    std::vector<int> allIdx(nVar);
    iota(allIdx.begin(), allIdx.end(), 1);
    formatInputIneqnsAsLP(allIdx);
    glp_read_lp(P, nullptr, tmpFileName);

    numRows = glp_get_num_rows(P);
    // numCols = glp_get_num_cols(P);
    numCols = 3;
    findDC();
    cout << sDcs << endl;

    MintermCalculator cDcs(sDcs);
    MintermVector vDc = cDcs.calculate();
    MintermVector vExp = cSource.calculate();

    //DEBUG
    for (int i : vExp)
    {
        cout << i << " ";
    }
    cout << endl;

    for (int i : vDc)
    {
        cout << i << " ";
    }
    cout << endl;
    
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