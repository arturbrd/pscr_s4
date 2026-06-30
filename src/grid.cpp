#include <iostream>
#include "grid.hpp"

std::ostream& operator<<(std::ostream& os, const FlowEntry& e) {
    return os << "FlowEntry{"
              << "dtime=" << e.dtime
              << ", section_code=" << e.section_code
              << ", value=" << e.value
              << "}";
}

std::ostream& operator<<(std::ostream& os, const GridMessage& g) {
    os << "GridMessage{\n";

    for (const auto& e : g.flow) {
        os << "  " << e << "\n";
    }

    return os << "}";
}

inline void from_json(const nlohmann::json& j, GridMessage& g)
{
    g.flow.clear();

    const auto& arr = j.at("flow").at("value");

    for (const auto& item : arr)
    {
        g.flow.push_back(item.get<FlowEntry>());
    }
}