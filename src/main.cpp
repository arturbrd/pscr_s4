#include <iostream>
#include <pthread.h>
#include <mutex>
#include <mqueue.h>
#include <cerrno>
#include <cstring>
#include "main.hpp"
#include "threads.hpp"

mqd_t mqtt_grid_queue;
mqd_t influx_queue;
mqd_t mqtt_weather_raw_queue;
mqd_t mqtt_weather_avg_queue;
std::unordered_map<long, WeatherMap> weather_data;

mqtt::async_client client(SERVER_ADDRESS, CLIENT_ID);


class MqttCallback : public virtual mqtt::callback {
    public:
        void message_arrived(mqtt::const_message_ptr msg) override {
            std::string msg_string = msg->to_string();
            std::string topic = msg->get_topic();
            std::cout << "Topic: " << msg->get_topic() << std::endl;
            std::cout << "Message: " << msg_string << std::endl;
            if (topic == "station/c3/grid/raw") {
                if (mq_send(mqtt_grid_queue, msg_string.c_str(), msg_string.size() + 1, 0) == -1) {
                    std::cerr << "Error: Couldn't send msg to queue" << std::endl;
                }
            } else if (topic == "weather/raw") {
                if (mq_send(mqtt_weather_raw_queue, msg_string.c_str(), msg_string.size() + 1, 0) == -1) {
                    std::cerr << "Error: Couldn't send msg to queue" << std::endl;
                }
            } else if (topic == "weather/avg") {
                if (mq_send(mqtt_weather_avg_queue, msg_string.c_str(), msg_string.size() + 1, 0) == -1) {
                    std::cerr << "Error: Couldn't send msg to queue" << std::endl;
                }
            }
            
            
        }
};

int main() {

    mqtt::connect_options connOpts;
    connOpts.set_mqtt_version(MQTTVERSION_3_1_1);

    MqttCallback mqtt_callback;
    client.set_callback(mqtt_callback);

    struct mq_attr attr_mqtt;
    attr_mqtt.mq_maxmsg = 10;
    attr_mqtt.mq_msgsize = 8192;
    
    mqtt_grid_queue = mq_open("/mqtt_grid_queue", O_CREAT | O_RDWR, 0666, &attr_mqtt);
    if (mqtt_grid_queue == (mqd_t)-1) {
        std::cerr << "Error: Couldn't open queue /mqtt_grid_queue" << std::endl
                    << strerror(errno) << std::endl;
        return 1;
    }

    struct mq_attr attr_mqtt_weather_raw_queue;
    attr_mqtt_weather_raw_queue.mq_maxmsg = 10;
    attr_mqtt_weather_raw_queue.mq_msgsize = 4096;

    mqtt_weather_raw_queue = mq_open("/mqtt_weather_raw_queue", O_CREAT | O_RDWR, 0666, &attr_mqtt_weather_raw_queue);
    if (mqtt_weather_raw_queue == (mqd_t)-1) {
        std::cerr << "Error: Couldn't open queue /mqtt_weather_raw_queue" << std::endl
                    << strerror(errno) << std::endl;
        return 1;
    }

    struct mq_attr attr_mqtt_weather_avg_queue;
    attr_mqtt_weather_avg_queue.mq_maxmsg = 4;
    attr_mqtt_weather_avg_queue.mq_msgsize = 1024;

    mqtt_weather_avg_queue = mq_open("/mqtt_weather_avg_queue", O_CREAT | O_RDWR, 0666, &attr_mqtt_weather_avg_queue);
    if (mqtt_weather_avg_queue == (mqd_t)-1) {
        std::cerr << "Error: Couldn't open queue /mqtt_weather_avg_queue" << std::endl
                    << strerror(errno) << std::endl;
        return 1;
    }

    struct mq_attr attr_influx_queue;
    attr_influx_queue.mq_maxmsg = 10;
    attr_influx_queue.mq_msgsize = 8192;

    influx_queue = mq_open("/influx_queue", O_CREAT | O_RDWR, 0666, &attr_influx_queue);
    if (influx_queue == (mqd_t)-1) {
        std::cerr << "Error: Couldn't open queue /ready_map_queue" << std::endl
                    << strerror(errno) << std::endl;
        return 1;
    }

    try {
        client.connect(connOpts)->wait();
        client.subscribe("station/c3/grid/raw", 1);

        client.subscribe("weather/avg", 1);
        client.subscribe("weather/raw", 1);
        std::cout << "Connected!" << std::endl;
    } catch (const mqtt::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    pthread_t grid_thread;
    pthread_create(&grid_thread, nullptr, grid_thread_func, nullptr);

    pthread_t weather_avg_thread;
    pthread_create(&weather_avg_thread, nullptr, weather_avg_thread_func, nullptr);

    pthread_t weather_raw_thread;
    pthread_create(&weather_raw_thread, nullptr, weather_raw_thread_func, nullptr);

    pthread_t influx_thread;
    pthread_create(&influx_thread, nullptr, influx_thread_func, nullptr);
    

    pthread_join(grid_thread, nullptr);
    pthread_join(weather_avg_thread, nullptr);
    pthread_join(weather_raw_thread, nullptr);
    pthread_join(influx_thread, nullptr);

    mq_close(mqtt_grid_queue);
    mq_unlink("/mqtt_grid_queue");

    mq_close(influx_queue);
    mq_unlink("/influx_queue");

    mq_close(mqtt_weather_avg_queue);
    mq_unlink("/mqtt_weather_avg_queue");

    mq_close(mqtt_weather_raw_queue);
    mq_unlink("/mqtt_weather_raw_queue");

    return 0;
}