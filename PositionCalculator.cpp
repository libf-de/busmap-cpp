//
// Created by Fabian Schillig on 25.10.24.
//

#include "PositionCalculator.h"
#include <ctime>
#include <algorithm>

std::vector<BusData> PositionCalculator::calculatePositions(const std::vector<PublicTransport>& inputData) {
    std::vector<BusData> positions;
    auto now = std::time(nullptr);

    for (auto lineData: inputData) {
        auto existingData = std::find_if(positions.begin(), positions.end(), [&lineData](BusData& busData) {
            return lineData.name == busData.name && lineData.originId == busData.originId;
        });

        BusData& busDataRef = (existingData != positions.end())
                ? *existingData
                : positions.emplace_back(lineData.name, lineData.originId, lineData.lineColor, std::vector<double>());
        std::optional<double> busPosition = std::nullopt;

        for (int i = 0; i < lineData.stops.size() - 1; ++i) {
            auto currentStop = lineData.stops[i];
            auto nextStop = lineData.stops[i + 1];

            auto currentDeparture = currentStop.departure;
            auto nextArrival = nextStop.arrival;

            if(currentDeparture.has_value() && nextArrival.has_value() && currentDeparture.value() <= now && nextArrival.value() > now) {
                auto segmentDurationSecs = std::difftime(currentDeparture.value(), nextArrival.value());
                auto secsSinceDeparture = std::difftime(currentDeparture.value(), now);
                auto progressInSegment = secsSinceDeparture / segmentDurationSecs;
                busPosition = currentStop.perc + (nextStop.perc - currentStop.perc) * progressInSegment;
                break;
            }
        }

        if(!busPosition.has_value()) {
            auto firstStop = lineData.stops.front();
            auto lastStop = lineData.stops.back();

            if(firstStop.departure.has_value() && now < firstStop.departure.value()) {
                busDataRef.positions.emplace_back(0.0);
            } else if(lastStop.arrival.has_value() && now > lastStop.arrival.value()) {
                busDataRef.positions.emplace_back(1.0);
            }
        } else {
            busDataRef.positions.emplace_back(busPosition.value());
        }
    }

    return positions;
}