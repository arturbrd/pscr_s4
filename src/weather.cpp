#include "weather.hpp"
#include <ostream>


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