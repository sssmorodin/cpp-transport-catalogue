#pragma once

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

#include <iostream>
#include <string>
#include <vector>
#include <istream>
#include <sstream>
#include <algorithm>

#include "geo.h"
#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
//#include "request_handler.h"

using namespace std::string_literals;

namespace catalogue {

    struct SecondIterationParse {
        std::unordered_map<std::string_view, bool> busname_to_roundtrip;
        std::unordered_map<std::string_view, std::vector<std::string_view>> bus;
        std::unordered_map<std::string_view, std::vector<std::pair<uint32_t, std::string_view>>> distances;
    };

    namespace parse_requests {
        std::deque<Stop> FirstIteration(const json::Document& json_requests);
        SecondIterationParse SecondIteration(const json::Document& json_requests);
        std::vector<std::pair<uint32_t, std::string_view>> Distances(const json::Node& node);
        svg::Color GetColor(const json::Node color);
        RenderSettings ReadRenderSettings(const json::Document& json_requests);
    }

} // namespace catalogue