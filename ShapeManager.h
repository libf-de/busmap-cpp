#pragma once

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <utility>

// Shape structure definition
struct Shape {
    std::string id;
    double lat;
    double lng;
    int seq;
    std::optional<double> shapeDist;

    Shape(std::string id_, double lat_, double lng_, int seq_,
          std::optional<double> shapeDist_ = std::nullopt);
};

class ShapeManager {
public:
    // Constructor
    explicit ShapeManager(const std::string& shapeFile = "shapes.txt");

    // Public methods
    double getPercentageDistance(const std::string& shapeId,
                                const std::string& stopId,
                                const std::pair<double, double>& coord);

    double calculatePercentageDistance(const std::string& shapeId,
                                     const std::pair<double, double>& coord);

private:
    // Private member variables
    std::map<std::string, double> stopDists;
    std::map<std::string, std::vector<Shape>> shapeData;

    // Private helper methods
    static double toRadians(double deg);
    static double haversine(double lat1, double lng1, double lat2, double lng2);
    static double calculateDistance(const Shape& shape,
                                  const std::vector<Shape>& wholeShape);
};
