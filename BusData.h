//
// Created by Fabian Schillig on 25.10.24.
//

#ifndef BUSDATA_H
#define BUSDATA_H
#include <string>

struct BusData {
    std::string name;
    std::vector<double> positions;
};

struct LedData {
    std::string lineName;
    std::vector<int> ledIndices;
};

#endif //BUSDATA_H
