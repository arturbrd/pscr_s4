#include "main.hpp"
#include <iostream>
#include "threads.hpp"
#include "weather.hpp"
#include "grid.hpp"

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

// void* reader_thread_func(void* arg) {
//     std::cout << "Hello from reader thread\n";
//     char buffer[4096];
//     while (true) {
//         ssize_t bytes_received = mq_receive(mqtt_reader_queue, buffer, 4096, NULL);
        
//         if (bytes_received == -1) {
//             std::cerr << "Error: Couldn't read from a queue" << std::endl;
//             break;
//         } else {
//             // std::cout << "Reader_thread: " << buffer << std::endl;
//             json j = json::parse(buffer);
//             Data d = j.get<Data>();
            
//             pthread_mutex_lock(&weather_data_mutex);

//             long key = find_nearest(d.timestamp);
//             if (key == -1)
//                 key = d.timestamp;

//             std::cout << "timestamp: " << d.timestamp << "; normalized: " << key << std::endl;

//             if (weather_data.count(key)) {
//                 weather_data[key].records.insert(weather_data[key].records.end(), d.records.begin(), d.records.end());
//                 std::cout << "Records count: " << weather_data[key].records.size() << std::endl;
//                 if (weather_data[key].records.size() == 125) {
//                     std::cout << "WeatherMap full" << std::endl;
//                     mq_send(ready_map_queue, (char*)&key, sizeof(long), 0);
//                 }
//             } else {
//                 WeatherMap map = {key, d.records};
//                 weather_data[key] = map;
//                 std::cout << "Created new WeatherMap" << std::endl;
//             }
//             pthread_mutex_unlock(&weather_data_mutex);

//             mq_send(mqtt_sender_queue_raw, buffer, bytes_received, 0);
//         }
//     }

//     return nullptr;
// }

// void* average_thread_func(void* arg) {
//     std::cout << "Hello from average thread\n";
//     char buffer[sizeof(long)];

//     while (true) {
//         ssize_t bytes_received = mq_receive(ready_map_queue, buffer, sizeof(long), NULL);
//         long index;
//         std::memcpy(&index, buffer, sizeof(long));
        
//         if (bytes_received == -1) {
//             std::cerr << "Error: Couldn't read from a queue" << std::endl;
//             break;
//         } else {
//             pthread_mutex_lock(&weather_data_mutex);

//             auto it = weather_data.find(index);
//             if (it == weather_data.end()) {
//                 pthread_mutex_unlock(&weather_data_mutex);
//                 std::cerr << "For some reason, there are no such records" << std::endl;
//                 continue;
//             }

//             std::vector<Record> records = std::move(it->second.records);
//             weather_data.erase(it);
            

//             pthread_mutex_unlock(&weather_data_mutex);

//             if (records.empty()) {
//                 std::cerr << "For some reason, there are no such records" << std::endl;
//                 continue;
//             }

//             double avg_temp = 0;
//             for (auto& r : records) avg_temp += r.temp_c;
//             avg_temp /= records.size();

//             double avg_wind = 0;
//             for (auto& r : records) avg_wind += r.wind_mps;
//             avg_wind /= records.size();

//             double avg_clouds = 0;
//             for (auto& r : records) avg_clouds += r.clouds_pct;
//             avg_clouds /= records.size();

//             std::cout << "Timestamp: " << index
//                     << "\nAverage temperature: " << avg_temp
//                     << "\nAverage wind: " << avg_wind
//                     << "\nAverage clouds: " << avg_clouds
//                     << std::endl;
            

//             AverageMsg msg = {
//                 .timestamp = index,
//                 .average_temp_c = avg_temp,
//                 .average_wind_mps = avg_wind,
//                 .average_cloud_pct = avg_clouds
//             };

//             nlohmann::json j = msg;   // <-- TO robi magia
//             std::string out = j.dump();

//             mq_send(mqtt_sender_queue_avg, out.c_str(), out.size() + 1, 0);
//         }
//     }

//     return nullptr;
// }

// void* sender_thread_avg_func(void* arg) {
//     std::cout << "Hello from sender thread\n";
//     char buffer[1024];
//     while (true) {
//         ssize_t bytes_received = mq_receive(mqtt_sender_queue_avg, buffer, 1024, NULL);
        
//         if (bytes_received == -1) {
//             std::cerr << "Error: Couldn't read from a queue" << std::endl;
//             break;
//         } else {

//             client.publish(
//                 "weather/avg",
//                 buffer,
//                 bytes_received,
//                 0,
//                 false
//             );
//         }
//     }

//     return nullptr;
// }

// void* sender_thread_raw_func(void* arg) {
//     std::cout << "Hello from sender thread\n";
//     char buffer[4096];
//     while (true) {
//         ssize_t bytes_received = mq_receive(mqtt_sender_queue_raw, buffer, 4096, NULL);
        
//         if (bytes_received == -1) {
//             std::cerr << "Error: Couldn't read from a queue" << std::endl;
//             break;
//         } else {

//             client.publish(
//                 "weather/raw",
//                 buffer,
//                 bytes_received,
//                 0,
//                 false
//             );
//         }
//     }

//     return nullptr;
// }


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

void* influx_thread_func(void* arg) {
    InfluxClient client(...);

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

        std::string lp(buffer, bytes);

        if (!client.write(lp))
        {
            std::cerr << "Influx write failed\n";
        }
    }
}