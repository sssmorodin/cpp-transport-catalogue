#pragma once

#include <string>
#include <vector>

#include "geo.h"

namespace catalogue {

    struct Stop {
        std::string name;
        geo::Coordinates coordinates;
        size_t id_wait_start = 0;
        size_t id_wait_end = 0;
    };

    struct Bus {
        std::string name;
        std::vector<Stop*> stops;
        double distance = 0;
        uint64_t route_length = 0;
        bool is_roundtrip;

        //std::pair<double, uint64_t> ComputeRouteLength();
    };

    struct RoutingSettings {
        size_t bus_wait_time = 0;  // minutes
        double bus_velocity = 0.0; // kmph
    };

    enum class RouteActType {
        WAIT,
        BUS
    };

    struct RouteAct {
        RouteActType route_act_type;
        std::string_view name;
        double time;
        size_t span_count;
    };

    struct RouteInfo {
        double total_time;
        std::vector<const RouteAct*> items;
        bool not_found_flag = false;
    };

} // namespace catalogue
