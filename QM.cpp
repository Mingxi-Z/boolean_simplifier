
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <unordered_set>
#include <unordered_map>
#include <bitset>
#include <sstream>
#include <iomanip>
#include <numeric>
std::vector<std::string> mul(const std::vector<std::string>& x, const std::vector<std::string>& y) { // Multiply 2 minterms
    std::vector<std::string> res;
    for (const auto& i : x) {
        bool found = false;
        for (const auto& j : y) {
            if (j.length() == i.length() + 1 && j.find(i) != std::string::npos) {
                found = true;
                break;
            }
        }
        if (!found) {
            res.push_back(i);
        }
    }
    for (const auto& i : y) {
        if (std::find(res.begin(), res.end(), i) == res.end()) {
            res.push_back(i);
        }
    }
    return res;
}

std::vector<std::vector<std::string>> multiply(const std::vector<std::vector<std::string>>& x, const std::vector<std::vector<std::string>>& y) { // Multiply 2 expressions
    std::vector<std::vector<std::string>> res;
    for (const auto& i : x) {
        for (const auto& j : y) {
            std::vector<std::string> tmp = mul(i, j);
            if (!tmp.empty()) {
                res.push_back(tmp);
            }
        }
    }
    return res;
}

std::vector<std::string> refine(const std::vector<std::string>& my_list, const std::vector<int>& dc_list) { // Removes don't care terms from a given list and returns refined list
    std::vector<std::string> res;
    for (const auto& i : my_list) {
        int minterm = std::stoi(i, nullptr, 2);
        if (std::find(dc_list.begin(), dc_list.end(), minterm) == dc_list.end()) {
            res.push_back(i);
        }
    }
    return res;
}

std::vector<std::string> findEPI(const std::unordered_map<std::string, std::vector<std::string>>& x) { // Function to find essential prime implicants from prime implicants chart
    std::vector<std::string> res;
    for (const auto& [key, value] : x) {
        if (value.size() == 1) {
            res.push_back(value[0]);
        }
    }
    return res;
}

std::vector<std::string> findVariables(const std::string& x) { // Function to find variables in a meanterm. For example, the minterm --01 has C' and D as variables
    std::vector<std::string> var_list;
    for (size_t i = 0; i < x.length(); ++i) {
        if (x[i] == '0') {
            var_list.push_back(std::string(1, 'A' + i) + "'");
        } else if (x[i] == '1') {
            var_list.push_back(std::string(1, 'A' + i));
        }
    }
    return var_list;
}

std::vector<std::string> flatten(const std::unordered_map<int, std::vector<std::string>>& x) { // Flattens a list
    std::vector<std::string> flattened_items;
    for (const auto& [key, value] : x) {
        flattened_items.insert(flattened_items.end(), value.begin(), value.end());
    }
    return flattened_items;
}

std::vector<std::string> findminterms(const std::string& a) { //Function for finding out which minterms are merged. For example, 10-1 is obtained by merging 9(1001) and 11(1011)
    int gaps = std::count(a.begin(), a.end(), '-');
    if (gaps == 0) {
        return {a};
    }
    std::vector<std::string> x;
    for (int i = 0; i < std::pow(2, gaps); ++i) {
        std::string bin_str = std::bitset<8>(i).to_string().substr(8 - gaps);
        x.push_back(bin_str);
    }
    std::vector<std::string> temp;
    for (int i = 0; i < std::pow(2, gaps); ++i) {
        std::string temp2 = a;
        int ind = -1;
        for (char j : x[0]) {
            if (ind != -1) {
                ind += temp2.substr(ind + 1).find('-') + 1;
            } else {
                ind = temp2.find('-');
            }
            temp2 = temp2.substr(0, ind) + j + temp2.substr(ind + 1);
        }
        temp.push_back(temp2);
        x.erase(x.begin());
    }
    return temp;
}

std::pair<bool, int> compare(const std::string& a, const std::string& b) { // Function for checking if 2 minterms differ by 1 bit only
    int c = 0;
    int mismatch_index = -1;
    for (size_t i = 0; i < a.length(); ++i) {
        if (a[i] != b[i]) {
            mismatch_index = i;
            ++c;
            if (c > 1) {
                return {false, -1};
            }
        }
    }
    return {true, mismatch_index};
}

void removeTerms(std::unordered_map<std::string, std::vector<std::string>>& _chart, const std::vector<std::string>& terms) { // Removes minterms which are already covered from chart
    for (const auto& i : terms) {
        for (const auto& j : findminterms(i)) {
            _chart.erase(j);
        }
    }
}

int main() {
    std::cout << "Enter the minterms: ";
    std::string input;
    std::getline(std::cin, input);
    std::vector<int> mt;
    std::istringstream iss(input);
    int num;
    while (iss >> num) {
        mt.push_back(num);
    }

    std::cout << "Enter the don't cares(If any): ";
    std::getline(std::cin, input);
    std::vector<int> dc;
    iss = std::istringstream(input);
    while (iss >> num) {
        dc.push_back(num);
    }

    std::sort(mt.begin(), mt.end());
    std::vector<int> minterms(mt);
    minterms.insert(minterms.end(), dc.begin(), dc.end());
    std::sort(minterms.begin(), minterms.end());
    int size = std::ceil(std::log2(minterms.back() + 1));
    std::unordered_map<int, std::vector<std::string>> groups;

    // Primary grouping starts
    for (int minterm : minterms) {
        std::string bin_str = std::bitset<8>(minterm).to_string().substr(8 - size);
        int ones = std::count(bin_str.begin(), bin_str.end(), '1');
        groups[ones].push_back(bin_str);
    }
    // Primary grouping ends

    //Primary group printing starts
    std::cout << "\n\n\n\nGroup No.\tMinterms\tBinary of Minterms\n" << std::string(50, '=') << "\n";
    for (const auto& [group_no, group_minterms] : groups) {
        std::cout << std::setw(5) << group_no << ":\n";
        for (const auto& minterm : group_minterms) {
            std::cout << "\t\t    " << std::setw(20) << std::bitset<8>(std::stoi(minterm, nullptr, 2)).to_ulong() << minterm << "\n";
        }
        std::cout << std::string(50, '-') << "\n";
    }
    //Primary group printing ends

    // Process for creating tables and finding prime implicants starts
    std::unordered_set<std::string> all_pi;
    while (true) {
        std::unordered_map<int, std::vector<std::string>> tmp = groups;
        groups.clear();
        int m = 0;
        std::unordered_set<std::string> marked;
        bool should_stop = true;
        std::vector<int> l(tmp.size());
        std::transform(tmp.begin(), tmp.end(), l.begin(), [](const auto& pair) { return pair.first; });
        std::sort(l.begin(), l.end());
        for (size_t i = 0; i < l.size() - 1; ++i) {
            for (const auto& j : tmp[l[i]]) { // Loop which iterates through current group elements
                for (const auto& k : tmp[l[i + 1]]) { // Loop which iterates through next group elements
                    auto res = compare(j, k); // Compare the minterms
                    if (res.first) { // If the minterms differ by 1 bit only
                        std::string merged_minterm = j.substr(0, res.second) + "-" + j.substr(res.second + 1);
                        groups[m].push_back(merged_minterm); // Put a '-' in the changing bit and add it to corresponding group
                        should_stop = false;
                        marked.insert(j); // Mark element j
                        marked.insert(k); // Mark element k
                    }
                }
            }
            m += 1;
        }
        std::vector<std::string> local_unmarked = flatten(tmp);
        std::vector<std::string> unmarked_vec(local_unmarked.size());
        std::vector<std::string>::iterator it = std::set_difference(local_unmarked.begin(), local_unmarked.end(), marked.begin(), marked.end(), unmarked_vec.begin());
        unmarked_vec.resize(it - unmarked_vec.begin());
        all_pi.insert(unmarked_vec.begin(), unmarked_vec.end()); // Adding Prime Implicants to global list
        std::cout << "Unmarked elements(Prime Implicants) of this table: " << (unmarked_vec.empty() ? "None" : std::accumulate(unmarked_vec.begin(), unmarked_vec.end(), std::string(), [](const std::string& a, const std::string& b) { return a.empty() ? b : a + ", " + b; })) << "\n";
        if (should_stop) { // If the minterms cannot be combined further
            std::cout << "\n\nAll Prime Implicants: " << (all_pi.empty() ? "None" : std::accumulate(all_pi.begin(), all_pi.end(), std::string(), [](const std::string& a, const std::string& b) { return a.empty() ? b : a + ", " + b; })) << "\n";
            break;
        }
        // Printing of all the next groups starts
        std::cout << "\n\n\n\nGroup No.\tMinterms\tBinary of Minterms\n" << std::string(50, '=') << "\n";
        for (const auto& [group_no, group_minterms] : groups) {
            std::cout << std::setw(5) << group_no << ":\n";
            for (const auto& minterm : group_minterms) {
                std::cout << "\t\t" << std::setw(24) << std::accumulate(findminterms(minterm).begin(), findminterms(minterm).end(), std::string(), [](const std::string& a, const std::string& b) { return a.empty() ? b : a + ", " + b; }) << minterm << "\n";
            }
            std::cout << std::string(50, '-') << "\n";
        }
        // Printing of all the next groups ends
    }
    // Process for creating tables and finding prime implicants ends

    // Printing and processing of Prime Implicant chart starts
    std::unordered_map<std::string, std::vector<std::string>> chart;
    int sz = std::ceil(std::log10(mt.back() + 1));
    std::cout << "\n\n\nPrime Implicants chart:\n\n    Minterms    |" << std::accumulate(mt.begin(), mt.end(), std::string(), [&sz](const std::string& a, int i) { return a + std::string(sz - std::to_string(i).length(), ' ') + std::to_string(i) + " "; }) << "\n" << std::string(mt.size() * (sz + 1) + 16, '=') << "\n";
    for (const auto& i : all_pi) {
        std::vector<std::string> merged_minterms = findminterms(i);
        std::cout << std::accumulate(merged_minterms.begin(), merged_minterms.end(), std::string(), [](const std::string& a, const std::string& b) { return a.empty() ? b : a + ", " + b; }) << std::string(16 - std::accumulate(merged_minterms.begin(), merged_minterms.end(), 0, [](int a, const std::string& b) { return a + b.length() + 2; }), ' ') << "|";
        int y = 0;
        for (const auto& j : refine(merged_minterms, dc)) {
            int x = std::distance(mt.begin(), std::find(mt.begin(), mt.end(), std::stoi(j, nullptr, 2))) * (sz + 1); // The position where we should put 'X'
            std::cout << std::string(std::abs(x - y), ' ') << std::string(sz - 1, ' ') << "X";
            y = x + sz;
            chart[j].push_back(i);
        }
        std::cout << "\n" << std::string(mt.size() * (sz + 1) + 16, '-') << "\n";
    }
    // Printing and processing of Prime Implicant chart ends

    std::vector<std::string> EPI = findEPI(chart); // Finding essential prime implicants
    std::cout << "\nEssential Prime Implicants: " << std::accumulate(EPI.begin(), EPI.end(), std::string(), [](const std::string& a, const std::string& b) { return a.empty() ? b : a + ", " + b; }) << "\n";
    removeTerms(chart, EPI); // Remove EPI related columns from chart

    std::vector<std::vector<std::string>> final_result;
    if (chart.empty()) { // If no minterms remain after removing EPI related columns
        final_result = std::vector<std::vector<std::string>>(EPI.size());
        std::transform(EPI.begin(), EPI.end(), final_result.begin(), findVariables); // Final result with only EPIs
    } else { // Else follow Petrick's method for further simplification
        std::vector<std::vector<std::vector<std::string

        std::vector<std::vector<std::string>> P;
        for (const auto& row : chart) {
            std::vector<std::string> temp;
            for (int j : row) {
                temp.push_back(findVariables(j));
            }
            P.push_back(temp);
        }

        while (P.size() > 1) { // Keep multiplying until we get the SOP form of P
            P[1] = multiply(P[0], P[1]);
            P.erase(P.begin());
        }

        auto minTerm = std::min_element(P[0].begin(), P[0].end(), 
            [](const std::string& a, const std::string& b) { return a.length() < b.length(); });
        final_result.push_back(*minTerm);

        for (int i : EPI) {
            final_result.push_back(findVariables(i));
        }
    }

    std::cout << "\n\nSolution: F = ";
    for (const auto& term : final_result) {
        std::cout << term << " + ";
    }
    std::cout << "\b\b "; // To remove the last " + "

    std::cin.get(); // Wait for user to press enter
    return 0;
    }



}
