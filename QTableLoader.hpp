#ifndef QTABLELOADER_HPP
#define QTABLELOADER_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include "json.hpp"  

// Define QTable as an unordered_map with string keys and vector<double> values.
using QTable = std::unordered_map<std::string, std::vector<double>>;

class QTableLoader {
public:
    // Loads the Q-table from a JSON file.
    static QTable loadQTable(const std::string& filename);
};

#endif 
