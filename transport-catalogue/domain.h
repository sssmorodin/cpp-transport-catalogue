#pragma once

#include <string>
#include <vector>

#include "geo.h"

namespace catalogue {

    struct Stop {
        std::string name;
        geo::Coordinates coordinates;
    };

    struct Bus {
        std::string name;
        std::vector<Stop*> stops;
        double distance = 0;
        uint64_t route_length = 0;
        bool is_roundtrip;

        std::pair<double, uint64_t> ComputeRouteLength();
    };
}
