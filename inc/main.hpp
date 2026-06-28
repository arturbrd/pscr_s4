#pragma once

#include <pthread.h>
#include <mqueue.h>
#include <string>
#include <unordered_map>
#include "weather.hpp"
#include <mqtt/async_client.h>


const std::string SERVER_ADDRESS = "tcp://25.34.193.58:1883";
const std::string CLIENT_ID = "test_sub";
const std::string TOPIC = "weathert/c1";
const int READER_THREADS_COUNT = 12;

extern mqd_t mqtt_grid_queue;
extern mqd_t influx_queue;
extern mqd_t mqtt_weather_raw_queue;
extern mqd_t mqtt_weather_avg_queue;

extern mqtt::async_client client;

extern std::unordered_map<long, WeatherMap> weather_data;