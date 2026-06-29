cmake_minimum_required(VERSION 3.10)
project(pscr_s2)

set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)
find_package(PahoMqttCpp REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(CURL REQUIRED)
find_package(fmt REQUIRED)

add_executable(app.exe 
    src/main.cpp
    src/threads.cpp
    src/weather.cpp
    src/grid.cpp
)


target_include_directories(app.exe
    PRIVATE ${CMAKE_SOURCE_DIR}/inc
)
target_link_libraries(app.exe
    PahoMqttCpp::paho-mqttpp3
    Threads::Threads
    nlohmann_json::nlohmann_json
    CURL::libcurl
    fmt::fmt
)