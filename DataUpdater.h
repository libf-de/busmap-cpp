#pragma once
#include "PublicTransport.h"
#include "ShapeManager.h"
#include "TripManager.h"
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <condition_variable>

#include "BusData.h"

using json = nlohmann::json;

class DataUpdater {
public:
    DataUpdater(ShapeManager& sm_, TripManager& tm_);
    ~DataUpdater();

    // Stop the update thread
    void stop();

    // Get current data (thread-safe)
    std::vector<PublicTransport> getCurrentData();

    // Force an immediate update
    void triggerUpdate();

    // Change update interval
    void setUpdateInterval(std::chrono::minutes newInterval);

private:
    ShapeManager& sm;
    TripManager& tm;

    // Thread-related members
    std::thread updateThread;
    std::mutex dataMutex;
    std::condition_variable cv;
    std::atomic<bool> running;
    std::atomic<bool> forceUpdate;
    std::chrono::minutes updateInterval;

    // Store the current data
    std::vector<PublicTransport> currentData;

    // Last update timestamp
    std::chrono::system_clock::time_point lastUpdateTime;

    // The original update function
    std::vector<PublicTransport> updateData();

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp);
    std::vector<Stop> toStopList(const json& locations, const std::string& shapeId);

    // Update loop that runs in the separate thread
    void updateLoop();
};

void printBusJson(const std::vector<BusData>& busses);
void printJson(const std::vector<PublicTransport>& transports);
std::optional<time_t> convertISOToTimeT(const std::string& isoString);