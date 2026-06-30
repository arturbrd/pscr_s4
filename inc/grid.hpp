#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

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

void from_json(const json& j, CostMsg& m);
void from_json(const json& j, FlowMsg& m);
void from_json(const json& j, UnbalancedMsg& m);
void from_json(const json& j, GridRawMessage& m);

std::ostream& operator<<(std::ostream& os, const FlowEntry& e);
std::ostream& operator<<(std::ostream& os, const FlowMsg& g);

int64_t to_epoch_ns(const std::string& dtime);

std::string to_influx(const FlowEntry& e);
std::string to_influx(const CostEntry& e);
std::string to_influx(const UnbalancedEntry& e);

std::string to_influx(const FlowMsg& msg);
std::string to_influx(const CostMsg& msg);
std::string to_influx(const UnbalancedMsg& msg);