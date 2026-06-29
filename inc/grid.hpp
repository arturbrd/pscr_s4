#pragma once

#include <string.h>
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

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GridMessage,
    flow)

std::ostream& operator<<(std::ostream& os, const FlowEntry& e);
std::ostream& operator<<(std::ostream& os, const GridMessage& g);