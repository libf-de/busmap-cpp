#include <iostream>
#include <string>
#include <curl/curl.h>

#include "DataUpdater.h"
#include "PublicTransport.h"
#include "ShapeManager.h"
#include "TripManager.h"
#include "nlohmann/json.hpp"


using json = nlohmann::json;

ShapeManager sm("shapes.txt");
TripManager tm("trips.txt");
DataUpdater updater(sm, tm);

int main() {
    try {
        auto transports = updater.updateData();
        printJson(transports);
        // for (const auto& transport : transports) {
        //     std::cout << "Transport: " << transport.name << std::endl;
        //     std::cout << "Number of stops: " << transport.stops.size() << std::endl;
        // }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}