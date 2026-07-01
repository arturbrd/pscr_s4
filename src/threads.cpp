#include "main.hpp"
#include <iostream>
#include "threads.hpp"
#include "weather.hpp"
#include "grid.hpp"
#include "influx.hpp"
#include <cstdlib>

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
        GridRawMessage msg = j.get<GridRawMessage>();

        std::string batch[3];

        batch[0] = to_influx(msg.cost);
        batch[1] = to_influx(msg.flow);
        batch[2] = to_influx(msg.unbalanced);
        for (int i = 0; i < 3; i++) {
            std::string* influx_payload = new std::string(std::move(batch[i]));

            if (mq_send(influx_queue, reinterpret_cast<const char*>(&influx_payload), sizeof(std::string*), 0) == -1) {
                std::cerr << "Error: Couldn't send msg to queue" << std::endl;
                std::cerr << "mq_send errno: " << errno 
                        << " (" << strerror(errno) << ")" << std::endl;
                delete influx_payload;
            }
        }
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

        // std::cout << "[AVG] " << msg << std::endl;

        // TODO: logika
        std::string lp = to_influx(msg);

        std::string* payload = new std::string(std::move(lp));

        if (mq_send(influx_queue, reinterpret_cast<const char*>(&payload), sizeof(std::string*), 0) == -1) {
            std::cerr << "Error: Couldn't send msg to queue" << std::endl;
            std::cerr << "mq_send errno: " << errno 
                    << " (" << strerror(errno) << ")" << std::endl;
            delete payload;
        }
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

        // std::cout << "[RAW] " << d << std::endl;

        // TODO: logika

        std::string lp = to_influx(d);

        std::string* payload = new std::string(std::move(lp));

        if (mq_send(influx_queue, reinterpret_cast<const char*>(&payload), sizeof(std::string*), 0) == -1) {
            std::cerr << "Error: Couldn't send msg to queue" << std::endl;
            std::cerr << "mq_send errno: " << errno 
                    << " (" << strerror(errno) << ")" << std::endl;
            delete payload;
        }        
    }
    return nullptr;
}

void* influx_thread_func(void* arg) {
    const char* val = std::getenv("INFLUX_TOKEN");
    std::string token = val ? val : "";
    InfluxWriter influx(
        "http://localhost:8181",
        "weather",
        token);

    char buffer[1024];
    std::cout << "Hello from influx_thread\n";
    while (true)
    {
        std::string* payload = nullptr;

        ssize_t bytes = mq_receive(
            influx_queue,
            reinterpret_cast<char*>(&payload),
            sizeof(payload),
            nullptr
        );

        if (bytes == -1) {
            std::cerr << "influx_queue error: "
                      << strerror(errno) << std::endl;
            continue;
        }

        if (payload == nullptr) continue;

        std::string data = std::move(*payload);
        delete payload;

        // std::cout << "Received: " << data << std::endl;

        influx.write(data);
    }
}