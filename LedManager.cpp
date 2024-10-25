//
// Created by Fabian Schillig on 25.10.24.
//

#include "LedManager.h"

#include <fstream>
#include <sstream>

uint32_t LedManager::createColor(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
    return (w << 24) | (r << 16) | (g << 8) | b;
}

std::vector<int> LedManager::parseLedList(const std::string& input) {
    std::vector<int> numbers;
    std::stringstream ss(input);
    std::string item;

    while (std::getline(ss, item, ',')) {
        numbers.push_back(std::stoi(item));
    }

    return numbers;
}

std::string LedManager::colorToHex(uint32_t color) {
    std::stringstream ss;
    ss << "#" << std::hex << std::setfill('0') << std::setw(6)
       << ((color >> 16) & 0xFF)         // Red
       << std::setw(2) << ((color >> 8) & 0xFF)  // Green
       << std::setw(2) << (color & 0xFF);        // Blue
    return ss.str();
}

uint32_t LedManager::hexToColor(const std::string& hex) {
    // Entferne # wenn vorhanden
    std::string cleanHex = hex;
    if (cleanHex[0] == '#') {
        cleanHex = cleanHex.substr(1);
    }

    // Konvertiere Hex zu RGB
    std::stringstream ss;
    ss << std::hex << cleanHex;
    int hexColor;
    ss >> hexColor;

    uint8_t r = (hexColor >> 16) & 0xFF;
    uint8_t g = (hexColor >> 8) & 0xFF;
    uint8_t b = hexColor & 0xFF;

    return createColor(r, g, b);
}

uint32_t LedManager::scaleColor(uint32_t color, int brightness) {
    double factor = brightness / 255.0;
    uint8_t w = (color >> 24) & 0xFF;
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;

    return createColor(
        static_cast<uint8_t>(r * factor),
        static_cast<uint8_t>(g * factor),
        static_cast<uint8_t>(b * factor),
        static_cast<uint8_t>(w * factor)
    );
}

void LedManager::extractColors(uint32_t color, uint8_t& w, uint8_t& r, uint8_t& g, uint8_t& b) {
    w = (color >> 24) & 0xFF;
    r = (color >> 16) & 0xFF;
    g = (color >> 8) & 0xFF;
    b = color & 0xFF;
}

// Addiert zwei Farben mit Begrenzung auf 255 pro Kanal
uint32_t LedManager::addColors(uint32_t color1, uint32_t color2) {
    uint8_t w1, r1, g1, b1, w2, r2, g2, b2;
    extractColors(color1, w1, r1, g1, b1);
    extractColors(color2, w2, r2, g2, b2);

    return createColor(
        std::min(255, r1 + r2),
        std::min(255, g1 + g2),
        std::min(255, b1 + b2),
        std::min(255, w1 + w2)
    );
}

LedManager::LedManager(const std::string &ledFile) {
    std::ifstream file(ledFile);
    std::string line;
    bool firstLine = true;

    while (std::getline(file, line)) {
        if (firstLine) {
            defaultBrightness = std::stoi(line);
            firstLine = false;
            continue;
        }

        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> cols;

        while (std::getline(ss, token, ';')) {
            cols.push_back(token);
        }

        if (cols.size() == 3) {
            auto ledList = parseLedList(cols[2]);
            LedData ledLine(cols[0], cols[1], ledList);
            leds.push_back(ledLine);
        }
    }
}

std::map<int, uint32_t> LedManager::getLeds(const std::vector<BusData> &busData) {
    std::map<int, uint32_t> result;

    // Behandle Randfälle
    if (leds.empty()) {
        return result;
    }

    for (auto bus: busData) {
        auto ledSegment = std::find_if(leds.begin(), leds.end(), [&bus](LedData& ledData) {
            return ledData.lineName == bus.name && ledData.originId == bus.originId;
        });

        if (ledSegment == leds.end()) continue;

        for (int ledIndex : ledSegment->ledIndices) {
            result[ledIndex] = scaleColor(bus.lineColor, defaultBrightness);
        }

        // Verarbeite jeden Fortschrittspunkt
        for (double p : bus.positions) {
            // Begrenze progress auf [0,1]
            p = std::max(0.0, std::min(1.0, p));

            // Berechne die theoretische Position
            double position = p * (ledSegment->ledIndices.size() - 1);

            // Bestimme die umgebenden LED-Indizes
            int lowIndex = std::floor(position);
            int highIndex = std::ceil(position);

            if (lowIndex == highIndex) {
                // Exakt auf einer LED
                uint32_t newColor = scaleColor(bus.lineColor, 255);
                result[ledSegment->ledIndices[lowIndex]] = addColors(result[ledSegment->ledIndices[lowIndex]], newColor);
            } else {
                // Zwischen zwei LEDs
                double fraction = position - lowIndex;
                int lowBrightness = static_cast<int>((1.0 - fraction) * 255);
                int highBrightness = static_cast<int>(fraction * 255);

                uint32_t lowColor = scaleColor(bus.lineColor, lowBrightness);
                uint32_t highColor = scaleColor(bus.lineColor, highBrightness);

                result[ledSegment->ledIndices[lowIndex]] = addColors(result[ledSegment->ledIndices[lowIndex]], lowColor);
                result[ledSegment->ledIndices[highIndex]] = addColors(result[ledSegment->ledIndices[highIndex]], highColor);
            }
        }
    }

    return result;
}