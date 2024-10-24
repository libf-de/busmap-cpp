#pragma once
#include "PublicTransport.h"
#include "ShapeManager.h"
#include "TripManager.h"
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <vector>

using json = nlohmann::json;

class DataUpdater {
public:
    DataUpdater(ShapeManager& sm_, TripManager& tm_)
        : sm(sm_), tm(tm_) {}

    ~DataUpdater() {
        curl_global_cleanup();
    }

    std::vector<PublicTransport> updateData();

private:
    ShapeManager& sm;
    TripManager& tm;

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp);
    std::vector<Stop> toStopList(const json& locations, const std::string& shapeId);
};

void printJson(const std::vector<PublicTransport>& transports);
std::optional<time_t> convertISOToTimeT(const std::string& isoString);