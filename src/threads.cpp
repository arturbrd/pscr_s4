#include "main.hpp"
#include <iostream>
#include "threads.hpp"
#include "weather.hpp"
#include "grid.hpp"
#include "influx.hpp"

using json = nlohmann::json;

constexpr long WINDOW = 120; // 2 minuty

long find_nearest(long ts) {
    long best_key = -1;
    long best_diff = WINDOW + 1;

    for (auto& [key, map] : weather_data) {
        long diff = std::abs(key - ts);

        if (diff <= WINDOW && diff < best_diff) {
            best_diff = diff;
            best_key = key;
        }
    }

    return best_key;
}

void* grid_thread_func(void* arg) {
    std::cout << "Hello from grid_thread\n";
    while (1) {
        std::string* payload = nullptr;

        ssize_t bytes = mq_receive(
            mqtt_grid_queue,
            reinterpret_cast<char*>(&payload),
            sizeof(payload),
            nullptr
        );

        if (bytes == -1) {
            std::cerr << "grid mq_receive error: "
                      << strerror(errno) << std::endl;
            continue;
        }

        if (payload == nullptr) continue;

        std::string data = std::move(*payload);
        delete payload;

        auto j = nlohmann::json::parse(data);
        GridMessage msg = j.get<GridMessage>();

        std::string batch;

        for (const auto& e : msg.flow) {
            batch += "grid_flow,section=" + e.section_code +
                     " value=" + std::to_string(e.value) +
                     " " + e.dtime + "\n";
        }

        mq_send(influx_queue, batch.c_str(), batch.size(), 0);

        // TODO: twoja logika
        std::cout << "[GRID] " << msg << std::endl;
    }
    return nullptr;
}

void* weather_avg_thread_func(void* arg) {
    char buffer[1024];
    std::cout << "Hello from weather_avg_thread\n";

    while (1) {
        ssize_t bytes = mq_receive(
            mqtt_weather_avg_queue,
            buffer,
            sizeof(buffer),
            nullptr
        );

        if (bytes == -1) {
            std::cerr << "weather_avg mq_receive error: "
                      << strerror(errno) << std::endl;
            continue;
        }

        auto j = nlohmann::json::parse(buffer);
        AverageMsg msg = j.get<AverageMsg>();

        std::cout << "[AVG] " << msg << std::endl;

        // TODO: logika
        std::string lp = to_influx(msg);

        mq_send(
            influx_queue,
            lp.data(),
            lp.size(),
            0
        );
    }
    return nullptr;
}

void* weather_raw_thread_func(void* arg) {
    char buffer[4096];
    std::cout << "Hello from weather_raw_thread\n";

    while (1) {
        ssize_t bytes = mq_receive(
            mqtt_weather_raw_queue,
            buffer,
            sizeof(buffer),
            nullptr
        );

        if (bytes == -1) {
            std::cerr << "weather_raw mq_receive error: "
                      << strerror(errno) << std::endl;
            continue;
        }

        json j = json::parse(buffer);
        Data d = j.get<Data>();

        std::cout << "[RAW] " << d << std::endl;

        // TODO: logika

        std::string lp = to_influx(d);

        mq_send(
            influx_queue,
            lp.data(),
            lp.size(),
            0
        );
    }
    return nullptr;
}

void* influx_thread_func(void* arg) {
    InfluxWriter influx(
        "http://localhost:8181",
        "weather",
        "TOKEN");

    char buffer[16384];

    while (true)
    {
        ssize_t bytes = mq_receive(
            influx_queue,
            buffer,
            sizeof(buffer),
            nullptr
        );

        if (bytes == -1)
            continue;

        influx.write(std::string_view(buffer, bytes));
    }
}