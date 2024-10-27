//
// Created by Fabian Schillig on 25.10.24.
//

#ifndef LEDMANAGER_H
#define LEDMANAGER_H
#include <map>
#include <string>
#include <vector>
#include <cmath>
#include <iomanip>
#include <algorithm>

#include "BusData.h"


class LedManager {
    public:
        // Constructor
        explicit LedManager(const std::string& ledFile = "leds.txt");
        static uint32_t hexToColor(const std::string& hex);
        static std::string colorToHex(uint32_t color);
        std::map<int, uint32_t> getLeds(const std::vector<BusData> &busData);
    private:
        int defaultBrightness;
        std::vector<LedData> leds;
        static std::vector<int> parseLedList(const std::string& input);
        static uint32_t scaleColor(uint32_t color, int brightness);
        static uint32_t createColor(uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0);
        static uint32_t addColors(uint32_t color1, uint32_t color2);
        static void extractColors(uint32_t color, uint8_t& w, uint8_t& r, uint8_t& g, uint8_t& b);
};



#endif //LEDMANAGER_H
