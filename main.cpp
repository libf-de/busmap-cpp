#include <iostream>
#include <string>
#include <curl/curl.h>

#include "DataUpdater.h"
#include "LedManager.h"
#include "PositionCalculator.h"
#include "PublicTransport.h"
#include "ShapeManager.h"
#include "TripManager.h"
#include "libs/rpi_ws281x/ws2811.h"
#include "nlohmann/json.hpp"

// ShapeManager sm("shapes.txt");
// TripManager tm("trips.txt");
// RouteManager rm("routes.txt");
// DataUpdater updater(sm, tm, rm);
LedManager lm("leds.txt");

int main() {
    // Your main program loop

    ws2811_t ledstring =
    {
        .freq = WS2811_TARGET_FREQ,
        .dmanum = 10,
        .channel =
        {
            [0] =
            {
                .gpionum = 12,
                .invert = 0,
                .count = 5,
                .strip_type = WS2811_STRIP_GBR,
                .brightness = 255,
            },
            [1] =
            {
                .gpionum = 0,
                .invert = 0,
                .count = 0,
                .brightness = 0,
            },
        },

    };

    ws2811_return_t ret;
    ws2811_led_t *matrix;

    matrix = static_cast<ws2811_led_t *>(malloc(sizeof(ws2811_led_t) * 5));

    if ((ret = ws2811_init(&ledstring)) != WS2811_SUCCESS)
    {
        fprintf(stderr, "ws2811_init failed: %s\n", ws2811_get_return_t_str(ret));
        return ret;
    }

    while (true) {
        // auto transports = updater.getCurrentData();
        // auto busses = PositionCalculator::calculatePositions(transports);
        //
        // // printJson(transports);
        // printBusJson(busses);
        for (int i = 0; i < 100; ++i) {
            std::vector<BusData> busses;
            std::vector<double> distances;
            distances.push_back(i / 100);

            BusData bus("1", "0", 0x00FF0000, distances);
            busses.push_back(bus);

            auto leds = lm.getLeds(busses);

            for (auto buss: leds) {
                ledstring.channel[0].leds[buss.first] = buss.second;
            }

            if ((ret = ws2811_render(&ledstring)) != WS2811_SUCCESS)
            {
                fprintf(stderr, "ws2811_render failed: %s\n", ws2811_get_return_t_str(ret));
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }


        // Process data...
        // std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    return 0;
}