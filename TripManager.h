#pragma once

#include <string>
#include <vector>
#include <map>
#include <list>
#include <optional>

struct Trip {
    std::string tripId;
    std::string routeId;
    std::string shapeId;
    std::string headsign;

    Trip(std::string tripId_, std::string routeId_,
         std::string shapeId_, std::string headsign_);
};

class TripManager {
public:
    explicit TripManager(const std::string& tripFile = "trips.txt");

    std::optional<std::string> findShapeId(const std::string& lineNumber,
                                         const std::string& startStation,
                                         const std::string& endStation);

private:
    std::vector<Trip> trips;
    std::list<std::string> skipIds;
    std::map<std::string, std::string> storedTrips;

    // Helper methods
    static std::string normalize(const std::string& input);
    static int levenshteinDistance(const std::string& a, const std::string& b);
    static double similarity(const std::string& a, const std::string& b);
    static std::optional<int> getSecondNumericPart(const std::string& str);
};
