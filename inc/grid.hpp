#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct FlowEntry {
    std::string dtime;
    std::string section_code;
    double value;
};

struct GridMessage {
    std::vector<FlowEntry> flow;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FlowEntry,
    dtime, section_code, value)

inline void from_json(const nlohmann::json& j, GridMessage& g)
{
    g.flow.clear();

    const auto& arr = j.at("flow").at("value");

    for (const auto& item : arr)
    {
        g.flow.push_back(item.get<FlowEntry>());
    }
}

std::ostream& operator<<(std::ostream& os, const FlowEntry& e);
std::ostream& operator<<(std::ostream& os, const GridMessage& g);