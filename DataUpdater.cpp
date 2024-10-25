#include "DataUpdater.h"
#include <iostream>
#include <sstream>

#include <nlohmann/json.hpp>

void to_json(json& j, const Stop& stop) {
    j = json::object();  // Initialize as JSON object
    j["id"] = stop.id;
    j["percentage"] = stop.perc;

    // Handle optional arrival
    if (stop.arrival) {
        j["arrival"] = *stop.arrival;
    } else {
        j["arrival"] = nullptr;
    }

    // Handle optional departure
    if (stop.departure) {
        j["departure"] = *stop.departure;
    } else {
        j["departure"] = nullptr;
    }
}

void to_json(json& j, const PublicTransport& transport) {
    j = json::object();  // Initialize as JSON object
    j["name"] = transport.name;
    j["stops"] = transport.stops;
}

size_t DataUpdater::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::vector<PublicTransport> DataUpdater::updateData() {
    std::vector<PublicTransport> result;
    CURL* curl = curl_easy_init();

    if (!curl) {
        throw std::runtime_error("Failed to initialize CURL");
    }

    std::string response_string;
    const char* url = "https://westfalenfahrplan.de/nwl-efa/XML_DM_REQUEST"
        "?calcOneDirection=1"
        "&deleteAssignedStops_dm=1"
        "&depSequence=30"
        "&depType=stopEvents"
        "&doNotSearchForStops=1"
        "&genMaps=0"
        "&inclMOT_5=true"
        "&includeCompleteStopSeq=1"
        "&includedMeans=checkbox"
        "&itOptionsActive=1"
        "&itdDateTimeDepArr=dep"
        "&language=de"
        "&lineRestriction=400"
        "&locationServerActive=1"
        "&maxTimeLoop=1"
        "&mode=direct"
        "&name_dm=de%3A05515%3A41000"
        "&nwlDMMacro=1"
        "&outputFormat=rapidJSON"
        "&ptOptionsActive=1"
        "&routeType=LEASTTIME"
        "&serverInfo=1"
        "&sl3plusDMMacro=1"
        "&trITMOTvalue100=10"
        "&type_dm=any"
        "&useAllStops=1"
        "&useRealtime=1"
        "&coordOutputFormat=WGS84%5Bdd.ddddd%5D";

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        curl_easy_cleanup(curl);
        throw std::runtime_error(std::string("curl_easy_perform() failed: ") +
                               curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);

    // Parse JSON response
    json dmr = json::parse(response_string);

    // Process stop events
    for (const auto& event : dmr["stopEvents"]) {
        std::cout << "Processing Line " << event["transportation"]["number"] << std::endl;

        auto shapeId = tm.findShapeId(
            event["transportation"]["number"],
            event["transportation"]["origin"]["name"],
            event["transportation"]["destination"]["name"]
        );

        if (!shapeId) {
            std::cerr << event["transportation"]["number"] << " from "
                      << event["transportation"]["origin"]["name"] << " not found!" << std::endl;
            continue;
        }

        auto prevStops = event.contains("previousLocations")
                            ? toStopList(event["previousLocations"], *shapeId)
                            : std::vector<Stop>();
        auto nextStops = event.contains("onwardLocations")
                            ? toStopList(event["onwardLocations"], *shapeId)
                            : std::vector<Stop>();

        std::vector<Stop> thisStop;
        {
            std::string id = event["location"]["parent"]["id"].get<std::string>();
            id = id.substr(id.find_last_of(':') + 1);

            // Handle arrival time
            std::optional<time_t> arrival = std::nullopt;
            if (event.contains("arrivalTimeEstimated") && !event["arrivalTimeEstimated"].is_null()) {
                arrival = convertISOToTimeT(event["arrivalTimeEstimated"].get<std::string>());
            } else if (event.contains("arrivalTimePlanned") && !event["arrivalTimePlanned"].is_null()) {
                arrival = convertISOToTimeT(event["arrivalTimePlanned"].get<std::string>());
            }

            // Handle departure time
            std::optional<time_t> departure = std::nullopt;
            if (event.contains("departureTimeEstimated") && !event["departureTimeEstimated"].is_null()) {
                departure = convertISOToTimeT(event["departureTimeEstimated"].get<std::string>());
            } else if (event.contains("departureTimePlanned") && !event["departureTimePlanned"].is_null()) {
                departure = convertISOToTimeT(event["departureTimePlanned"].get<std::string>());
            }

            double perc = sm.getPercentageDistance(
                *shapeId,
                event["location"]["parent"]["id"],
                {event["location"]["coord"][0], event["location"]["coord"][1]}
            );

            thisStop.emplace_back(id, arrival, departure, perc);
        }

        std::vector<Stop> allStops;
        allStops.insert(allStops.end(), prevStops.begin(), prevStops.end());
        allStops.insert(allStops.end(), thisStop.begin(), thisStop.end());
        allStops.insert(allStops.end(), nextStops.begin(), nextStops.end());

        result.emplace_back(
            event["transportation"]["number"].get<std::string>(),
            std::move(allStops)
        );
    }

    return result;
}

std::optional<time_t> convertISOToTimeT(const std::string& isoString) {
    struct tm tm = {};
    // Parse the ISO 8601 string format: "2024-10-24T23:05:00Z"
    if (strptime(isoString.c_str(), "%Y-%m-%dT%H:%M:%SZ", &tm) == nullptr) {
        return std::nullopt; // Return -1 on parsing error
    }

    // Convert to time_t (UTC)
    return timegm(&tm);  // Use timegm for UTC time
}

std::vector<Stop> DataUpdater::toStopList(const json& locations, const std::string& shapeId) {
    std::vector<Stop> stops;
    stops.reserve(locations.size());

    for (const auto& loc : locations) {
        if (!loc.contains("parent") || !loc["parent"].contains("id")) continue;

        std::string id = loc["parent"]["id"].get<std::string>();
        id = id.substr(id.find_last_of(':') + 1);

        // Handle arrival time
        std::optional<time_t> arrival = std::nullopt;
        if (loc.contains("arrivalTimeEstimated") && !loc["arrivalTimeEstimated"].is_null()) {
            arrival = convertISOToTimeT(loc["arrivalTimeEstimated"].get<std::string>());
        } else if (loc.contains("arrivalTimePlanned") && !loc["arrivalTimePlanned"].is_null()) {
            arrival = convertISOToTimeT(loc["arrivalTimePlanned"].get<std::string>());
        }

        // Handle departure time
        std::optional<time_t> departure = std::nullopt;
        if (loc.contains("departureTimeEstimated") && !loc["departureTimeEstimated"].is_null()) {
            departure = convertISOToTimeT(loc["departureTimeEstimated"].get<std::string>());
        } else if (loc.contains("departureTimePlanned") && !loc["departureTimePlanned"].is_null()) {
            departure = convertISOToTimeT(loc["departureTimePlanned"].get<std::string>());
        }

        if (!loc.contains("coord") || !loc["coord"].is_array() || loc["coord"].size() < 2) continue;
        double perc = sm.getPercentageDistance(
            shapeId,
            loc["parent"]["id"],
            {loc["coord"][0], loc["coord"][1]}
        );

        stops.emplace_back(id, arrival, departure, perc);
    }

    return stops;
}

void printJson(const std::vector<PublicTransport>& transports) {
    json j = transports;
    std::cout << j.dump(4) << std::endl;
}