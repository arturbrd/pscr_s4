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