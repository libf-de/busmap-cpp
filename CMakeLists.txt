cmake_minimum_required(VERSION 3.29)
project(cpp)

find_package(CURL REQUIRED)
set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(json)

set(CMAKE_CXX_STANDARD 20)

add_executable(cpp main.cpp
        ShapeManager.cpp
        ShapeManager.h
        PublicTransport.h
        TripManager.cpp
        TripManager.h
        DataUpdater.cpp
        DataUpdater.h)

include_directories(${CURL_INCLUDE_DIR})

target_link_libraries(cpp ${CURL_LIBRARIES} nlohmann_json::nlohmann_json)