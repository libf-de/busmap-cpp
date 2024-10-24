#include "TripManager.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <regex>
#include <cmath>
#include <iostream>
#include <numeric>

Trip::Trip(std::string tripId_, std::string routeId_,
           std::string shapeId_, std::string headsign_)
    : tripId(std::move(tripId_))
    , routeId(std::move(routeId_))
    , shapeId(std::move(shapeId_))
    , headsign(std::move(headsign_)) {}

TripManager::TripManager(const std::string& tripFile) {
    std::ifstream file(tripFile);
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

        if (cols.size() >= 8) {
            trips.emplace_back(cols[2], cols[0], cols[7], cols[3]);
        }
    }
}

std::string TripManager::normalize(const std::string& input) {
    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);

    // Replace German characters
    result = std::regex_replace(result, std::regex("ä"), "ae");
    result = std::regex_replace(result, std::regex("ö"), "oe");
    result = std::regex_replace(result, std::regex("ü"), "ue");
    result = std::regex_replace(result, std::regex("ß"), "ss");

    // Remove non-word characters
    result = std::regex_replace(result, std::regex("\\W+"), "");

    // Trim
    result.erase(0, result.find_first_not_of(" \t\n\r\f\v"));
    result.erase(result.find_last_not_of(" \t\n\r\f\v") + 1);

    return result;
}

int TripManager::levenshteinDistance(const std::string& a, const std::string& b) {
    const size_t n = a.length();
    const size_t m = b.length();

    std::vector<int> currentRow(n + 1);
    std::iota(currentRow.begin(), currentRow.end(), 0);

    for (size_t i = 1; i <= m; ++i) {
        std::vector<int> previousRow = currentRow;
        currentRow[0] = i;

        for (size_t j = 1; j <= n; ++j) {
            int insertCost = previousRow[j] + 1;
            int deleteCost = currentRow[j - 1] + 1;
            int replaceCost = previousRow[j - 1] + (a[j - 1] != b[i - 1]);
            currentRow[j] = std::min({insertCost, deleteCost, replaceCost});
        }
    }

    return currentRow[n];
}

double TripManager::similarity(const std::string& a, const std::string& b) {
    const size_t maxLen = std::max(a.length(), b.length());
    if (maxLen == 0) return 1.0;
    const int dist = levenshteinDistance(a, b);
    return 1.0 - static_cast<double>(dist) / maxLen;
}

std::optional<int> TripManager::getSecondNumericPart(const std::string& str) {
    std::stringstream ss(str);
    std::string part;
    int count = 0;

    while (std::getline(ss, part, '_')) {
        if (count == 1) {
            try {
                return std::stoi(part);
            } catch (...) {
                return std::nullopt;
            }
        }
        count++;
    }
    return std::nullopt;
}

std::optional<std::string> TripManager::findShapeId(
    const std::string& lineNumber,
    const std::string& startStation,
    const std::string& endStation) {

    const std::string tid = lineNumber + "-" + startStation + "-" + endStation;
    const std::string normStartStation = normalize(startStation);
    const std::string normEndStation = normalize(endStation);

    // Filter matching trips
    std::vector<Trip> matchingTrips;
    std::copy_if(trips.begin(), trips.end(), std::back_inserter(matchingTrips),
        [&](const Trip& trip) { return trip.routeId == lineNumber; });

    if (matchingTrips.empty()) {
        skipIds.push_back(tid);
        return std::nullopt;
    }

    double bestScoreStart = -1.0;
    std::vector<Trip> bestMatchesStart;
    double bestScoreEnd = -1.0;
    std::vector<Trip> bestMatchesEnd;

    for (const auto& trip : matchingTrips) {
        double startScore = similarity(normStartStation, normalize(trip.headsign));
        if (startScore > bestScoreStart) {
            bestMatchesStart.clear();
            bestMatchesStart.push_back(trip);
            bestScoreStart = startScore;
        } else if (startScore == bestScoreStart) {
            bestMatchesStart.push_back(trip);
        }

        double endScore = similarity(normEndStation, normalize(trip.headsign));
        if (endScore > bestScoreEnd) {
            bestMatchesEnd.clear();
            bestMatchesEnd.push_back(trip);
            bestScoreEnd = endScore;
        } else if (endScore == bestScoreEnd) {
            bestMatchesEnd.push_back(trip);
        }
    }

    if (bestMatchesStart.empty() && bestMatchesEnd.empty()) {
        skipIds.push_back(tid);
        return std::nullopt;
    }

    if (bestMatchesEnd.size() == 1) {
        storedTrips[tid] = bestMatchesEnd.front().shapeId;
        return bestMatchesEnd.front().shapeId;
    }

    for (const auto& endCandidate : bestMatchesEnd) {
        auto endSP = getSecondNumericPart(endCandidate.shapeId);
        if (!endSP) continue;

        for (const auto& start : bestMatchesStart) {
            auto startSP = getSecondNumericPart(start.shapeId);
            if (startSP && *startSP == *endSP) {
                storedTrips[tid] = start.shapeId;
                return start.shapeId;
            }
        }
    }

    if (!bestMatchesEnd.empty()) {
        storedTrips[tid] = bestMatchesEnd.front().shapeId;
        return bestMatchesEnd.front().shapeId;
    }

    skipIds.push_back(tid);
    return std::nullopt;
}
