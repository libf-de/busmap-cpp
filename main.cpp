#include <iostream>
#include <string>
#include <curl/curl.h>

#include "DataUpdater.h"
#include "PositionCalculator.h"
#include "PublicTransport.h"
#include "ShapeManager.h"
#include "TripManager.h"
#include "nlohmann/json.hpp"

ShapeManager sm("shapes.txt");
TripManager tm("trips.txt");
DataUpdater updater(sm, tm);

int main() {
    // Your main program loop
    while (true) {
        auto transports = updater.getCurrentData();
        auto busses = PositionCalculator::calculatePositions(transports);

        // printJson(transports);
        printBusJson(busses);

        // Process data...
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }


    try {
        auto transports = updater.getCurrentData();
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