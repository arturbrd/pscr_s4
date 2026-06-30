#include "weather.hpp"
#include <ostream>
#include <string>

std::ostream& operator<<(std::ostream& os, const Record& r) {
    return os << "Record{"
              << "id=" << r.point_id
              << ", lat=" << r.lat
              << ", lon=" << r.lon
              << ", temp=" << r.temp_c
              << ", wind=" << r.wind_mps
              << ", clouds=" << r.clouds_pct
              << ", ts=" << r.timestamp
              << ", valid=" << r.valid
              << "}";
}

std::ostream& operator<<(std::ostream& os, const Data& d) {
    os << "Data{ task_id=" << d.task_id
       << ", timestamp=" << d.timestamp
       << ", count=" << d.count
       << ", records=[\n";

    for (const auto& r : d.records) {
        os << "  " << r << "\n";
    }

    return os << "] }";
}

std::ostream& operator<<(std::ostream& os, const AverageMsg& r)
{
    os << "AverageMsg{"
       << "timestamp=" << r.timestamp
       << ", temp=" << r.average_temp_c
       << ", wind=" << r.average_wind_mps
       << ", clouds=" << r.average_cloud_pct
       << "}";

    return os;
}

std::string to_influx(const Record& r) {
    return "weather_raw,point_id=" + std::to_string(r.point_id) +
           " lat=" + std::to_string(r.lat) +
           ",lon=" + std::to_string(r.lon) +
           ",temp_c=" + std::to_string(r.temp_c) +
           ",wind_mps=" + std::to_string(r.wind_mps) +
           ",clouds_pct=" + std::to_string(r.clouds_pct) +
           ",valid=" + std::to_string(r.valid) +
           ",timestamp=" + std::to_string(r.timestamp);
}

std::string to_influx(const Data& d) {
    std::string out;
    out.reserve(d.records.size() * 128);

    for (const auto& r : d.records) {
        out += to_influx(r);
        out += "\n";
    }

    return out;
}

std::string to_influx(const AverageMsg& a) {
    return "weather_avg "
           "average_temp_c=" + std::to_string(a.average_temp_c) +
           ",average_wind_mps=" + std::to_string(a.average_wind_mps) +
           ",average_cloud_pct=" + std::to_string(a.average_cloud_pct) +
           " " + std::to_string(a.timestamp);
}