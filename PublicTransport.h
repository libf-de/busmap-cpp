#pragma once
#include <string>
#include <vector>
#include <optional>
#include <cstdint>

struct Stop {
    std::string id;
    std::optional<time_t> arrival;
    std::optional<time_t> departure;
    double perc;

    Stop(std::string id_,
         std::optional<time_t> arrival_,
         std::optional<time_t> departure_,
         double perc_)
        : id(std::move(id_))
        , arrival(std::move(arrival_))
        , departure(std::move(departure_))
        , perc(perc_) {}
};


struct PublicTransport {
    std::string name;
    std::string originId;
    uint32_t lineColor{};
    std::vector<Stop> stops;

    PublicTransport(std::string name_, std::vector<Stop> stops_):
        name(std::move(name_)),
        stops(std::move(stops_)) {}

    PublicTransport(std::string name_, std::string originId_, uint32_t lineColor_, std::vector<Stop> stops_):
        name(std::move(name_)),
        originId(std::move(originId_)),
        lineColor(lineColor_),
        stops(std::move(stops_)) {}
};
