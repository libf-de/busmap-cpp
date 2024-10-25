//
// Created by Fabian Schillig on 25.10.24.
//

#ifndef POSITIONCALCULATOR_H
#define POSITIONCALCULATOR_H
#include <vector>

#include "BusData.h"
#include "PublicTransport.h"


class PositionCalculator {
    public:
        static std::vector<BusData> calculatePositions(const std::vector<PublicTransport>& inputData);
};

#endif //POSITIONCALCULATOR_H