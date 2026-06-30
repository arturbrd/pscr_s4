#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct CostEntry {
    double cen_cost;
    std::string dtime;
};

struct CostMsg {
    std::vector<CostEntry> value;
};


struct FlowEntry {
    std::string dtime;
    std::string section_code;
    double value;
};


struct FlowMsg {
    std::vector<FlowEntry> value;
};

struct UnbalancedEntry {
    double balance;
    std::string dtime;
    double en_d;
    double en_w;
};

struct UnbalancedMsg {
    std::vector<UnbalancedEntry> value;
};

struct GridRawMessage {
    CostMsg cost;
    FlowMsg flow;
    UnbalancedMsg unbalanced;
};

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

std::ostream& operator<<(std::ostream& os, const FlowEntry& e);
std::ostream& operator<<(std::ostream& os, const FlowMsg& g);

int64_t to_epoch_ns(const std::string& dtime);

std::string to_influx(const FlowEntry& e);
std::string to_influx(const CostEntry& e);
std::string to_influx(const UnbalancedEntry& e);

std::string to_influx(const FlowMsg& msg);
std::string to_influx(const CostMsg& msg);
std::string to_influx(const UnbalancedMsg& msg);