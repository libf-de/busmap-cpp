//
// Created by Fabian Schillig on 25.10.24.
//

#ifndef ROUTEMANAGER_H
#define ROUTEMANAGER_H
#include <map>
#include <string>
#include <cstdint>


class RouteManager {
    public:
        // Constructor
        explicit RouteManager(const std::string& routeFile = "routes.txt");
        uint32_t getRouteColor(const std::string &line);
    private:
        std::map<std::string, uint32_t> routeColorMap;
};



#endif //ROUTEMANAGER_H
