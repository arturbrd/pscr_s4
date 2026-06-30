#include <iostream>
#include "grid.hpp"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>

std::ostream& operator<<(std::ostream& os, const FlowEntry& e) {
    return os << "FlowEntry{"
              << "dtime=" << e.dtime
              << ", section_code=" << e.section_code
              << ", value=" << e.value
              << "}";
}

std::ostream& operator<<(std::ostream& os, const FlowMsg& g) {
    os << "FlowMsg{\n";

    for (const auto& e : g.value) {
        os << "  " << e << "\n";
    }

    return os << "}";
}

int64_t to_epoch_ns(const std::string& dtime) {
    std::tm tm{};
    std::istringstream ss(dtime);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");

    auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        tp.time_since_epoch()
    ).count();
}

std::string to_influx(const FlowEntry& e) {
    return "grid_flow,section=" + e.section_code +
           " value=" + std::to_string(e.value) +
           " " + std::to_string(to_epoch_ns(e.dtime));
}

std::string to_influx(const CostEntry& e) {
    return "grid_cost "
           "cen_cost=" + std::to_string(e.cen_cost) +
           " " + std::to_string(to_epoch_ns(e.dtime));
}

std::string to_influx(const UnbalancedEntry& e) {
    return "grid_unbalanced "
           "balance=" + std::to_string(e.balance) +
           ",en_d=" + std::to_string(e.en_d) +
           ",en_w=" + std::to_string(e.en_w) +
           " " + std::to_string(to_epoch_ns(e.dtime));
}

std::string to_influx(const FlowMsg& msg) {
    std::string batch;
    batch.reserve(msg.value.size() * 128);

    for (const auto& e : msg.value) {
        batch += to_influx(e);
        batch += "\n";
    }

    return batch;
}

std::string to_influx(const CostMsg& msg) {
    std::string batch;
    batch.reserve(msg.value.size() * 128);

    for (const auto& e : msg.value) {
        batch += to_influx(e);
        batch += "\n";
    }

    return batch;
}

std::string to_influx(const UnbalancedMsg& msg) {
    std::string batch;
    batch.reserve(msg.value.size() * 128);

    for (const auto& e : msg.value) {
        batch += to_influx(e);
        batch += "\n";
    }

    return batch;
}

void from_json(const json& j, CostMsg& m) {
    m.value.clear();

    for (const auto& item : j.at("value")) {
        m.value.push_back({
            item.at("cen_cost").get<double>(),
            item.at("dtime").get<std::string>()
        });
    }
}

void from_json(const json& j, FlowMsg& m) {
    m.value.clear();

    for (const auto& item : j.at("value")) {
        m.value.push_back({
            item.at("dtime").get<std::string>(),
            item.at("section_code").get<std::string>(),
            item.at("value").get<double>()
        });
    }
}


void from_json(const json& j, UnbalancedMsg& m) {
    m.value.clear();

    for (const auto& item : j.at("value")) {
        m.value.push_back({
            item.at("balance").get<double>(),
            item.at("dtime").get<std::string>(),
            item.at("en_d").get<double>(),
            item.at("en_w").get<double>()
        });
    }
}


void from_json(const json& j, GridRawMessage& m) {
    m.cost = j.at("cost");
    m.flow = j.at("flow");
    m.unbalanced = j.at("unbalanced");
}