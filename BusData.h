//
// Created by Fabian Schillig on 25.10.24.
//

#ifndef BUSDATA_H
#define BUSDATA_H
#include <string>

struct BusData {
    std::string name;
    std::string originId;
    uint32_t lineColor;
    std::vector<double> positions;
};

struct LedData {
    std::string lineName;
    std::string originId;
    std::vector<int> ledIndices;
};

#endif //BUSDATA_H
