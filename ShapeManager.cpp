//
// Created by Fabian Schillig on 24.10.24.
//

#include "ShapeManager.h"
#include <cmath>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <limits>

// Shape constructor implementation
Shape::Shape(std::string id_, double lat_, double lng_, int seq_,
             std::optional<double> shapeDist_)
    : id(std::move(id_))
    , lat(lat_)
    , lng(lng_)
    , seq(seq_)
    , shapeDist(shapeDist_) {}

// ShapeManager constructor implementation
ShapeManager::ShapeManager(const std::string& shapeFile) {
    std::ifstream file(shapeFile);
    std::string line;
    bool firstLine = true;

    while (std::getline(file, line)) {
        if (firstLine) {
            firstLine = false;
            continue;
        }

        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> cols;

        while (std::getline(ss, token, ',')) {
            cols.push_back(token);
        }

        if (cols.size() >= 4) {
            std::optional<double> shapeDist = std::nullopt;
            if (cols.size() > 4 && !cols[4].empty()) {
                try {
                    shapeDist = std::stod(cols[4]);
                } catch (...) {}
            }

            Shape shape(cols[0], std::stod(cols[1]), std::stod(cols[2]),
                       std::stoi(cols[3]), shapeDist);
            shapeData[cols[0]].push_back(shape);
        }
    }
}

double ShapeManager::toRadians(double deg) {
    return deg / 180.0 * M_PI;
}

double ShapeManager::haversine(double lat1, double lng1, double lat2, double lng2) {
    double dLat = toRadians(lat2 - lat1);
    double dLon = toRadians(lng2 - lng1);
    double originLat = toRadians(lat1);
    double destinationLat = toRadians(lat2);

    double a = std::pow(std::sin(dLat / 2), 2) +
               std::pow(std::sin(dLon / 2), 2) *
               std::cos(originLat) *
               std::cos(destinationLat);
    double c = 2 * std::asin(std::sqrt(a));
    return 6371.0 * c;
}

double ShapeManager::calculateDistance(const Shape& shape,
                                     const std::vector<Shape>& wholeShape) {
    double totalDist = 0.0;
    for (size_t i = 1; i <= shape.seq && i < wholeShape.size(); ++i) {
        totalDist += haversine(
            wholeShape[i-1].lat, wholeShape[i-1].lng,
            wholeShape[i].lat, wholeShape[i].lng
        );
    }
    return totalDist;
}

double ShapeManager::getPercentageDistance(const std::string& shapeId,
                                         const std::string& stopId,
                                         const std::pair<double, double>& coord) {
    std::string sid = shapeId + "-" + stopId;
    auto it = stopDists.find(sid);
    if (it != stopDists.end()) {
        return it->second;
    }
    double dist = calculatePercentageDistance(shapeId, coord);
    stopDists[sid] = dist;
    return dist;
}

double ShapeManager::calculatePercentageDistance(const std::string& shapeId,
                                               const std::pair<double, double>& coord) {
    auto it = shapeData.find(shapeId);
    if (it == shapeData.end()) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    auto& shapeCoords = it->second;

    auto stopPointIt = std::min_element(shapeCoords.begin(), shapeCoords.end(),
        [&](const Shape& a, const Shape& b) {
            return haversine(a.lat, a.lng, coord.first, coord.second) <
                   haversine(b.lat, b.lng, coord.first, coord.second);
        });

    if (!stopPointIt->shapeDist.has_value()) {
        stopPointIt->shapeDist = calculateDistance(*stopPointIt, shapeCoords);
    }

    auto& lastPoint = shapeCoords.back();
    if (!lastPoint.shapeDist.has_value()) {
        lastPoint.shapeDist = calculateDistance(lastPoint, shapeCoords);
    }

    return stopPointIt->shapeDist.value() / lastPoint.shapeDist.value();
}
