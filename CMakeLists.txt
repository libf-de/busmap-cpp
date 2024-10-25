cmake_minimum_required(VERSION 3.0.0)
project(cpp)

find_package(CURL REQUIRED)
set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(libs/json)
#add_subdirectory(libs/rpi_ws281x)

set(CMAKE_CXX_STANDARD 20)

add_executable(cpp main.cpp
        ShapeManager.cpp
        ShapeManager.h
        PublicTransport.h
        TripManager.cpp
        TripManager.h
        DataUpdater.cpp
        DataUpdater.h
        BusData.h
        PositionCalculator.cpp
        PositionCalculator.h
        LedManager.cpp
        LedManager.h
        RouteManager.cpp
        RouteManager.h)

include_directories(${CURL_INCLUDE_DIR})
#include_directories(rpi_ws281x)

#target_link_libraries(cpp ${CURL_LIBRARIES} ws2811 nlohmann_json::nlohmann_json)
target_link_libraries(cpp ${CURL_LIBRARIES} nlohmann_json::nlohmann_json)