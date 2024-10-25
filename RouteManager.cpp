//
// Created by Fabian Schillig on 25.10.24.
//

#include "RouteManager.h"

#include <fstream>
#include <sstream>

#include "LedManager.h"

RouteManager::RouteManager(const std::string& routeFile) {
    std::ifstream file(routeFile);
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

        if (cols.size() >= 8)
            routeColorMap[cols[0]] = LedManager::hexToColor(cols[7]);
    }
}

uint32_t RouteManager::getRouteColor(const std::string &line) {
    if(routeColorMap.contains(line)) return routeColorMap[line];
    return 0xFFFFFFFF;
}
