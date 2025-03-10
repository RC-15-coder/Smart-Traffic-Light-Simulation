#include "QTableLoader.hpp"
#include <fstream>
#include <iostream>

QTable QTableLoader::loadQTable(const std::string& filename) {
    QTable table;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Unable to open Q-table file: " << filename << std::endl;
        return table;
    }
    nlohmann::json j;
    file >> j;
    // Convert each key-value pair from JSON into the QTable.
    for (auto it = j.begin(); it != j.end(); ++it) {
        // it.key() is a string key (e.g., "(1, 5)") and it.value() is an array of numbers.
        table[it.key()] = it.value().get<std::vector<double>>();
    }
    return table;
}
