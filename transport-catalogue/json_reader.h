#pragma once

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
#include "request_handler.h"

using namespace std::string_literals;

namespace catalogue {

    class JSONReader {
    public:
        JSONReader(std::istream& input);

        struct SecondIterationParse {
            std::unordered_map<std::string_view, bool> busname_to_roundtrip;
            std::unordered_map<std::string_view, std::vector<std::string_view>> bus;
            std::unordered_map<std::string_view, std::vector<std::pair<uint32_t, std::string_view>>> distances;
        };

        TransportCatalogue BuildTransportCatalogue();
        std::deque<Stop> FirstIteration(const json::Document &json_requests);
        SecondIterationParse SecondIteration(const json::Document &json_requests);
        std::vector<std::pair<uint32_t, std::string_view>> Distances(const json::Node &node);
        svg::Color GetColor(const json::Node color);
        RenderSettings ReadRenderSettings();
        RoutingSettings ReadRouteSettings(const json::Document& json_requests);
        const json::Document& GetJSONRequests();
        const TransportCatalogue& GetTransportCatalogue();
        const renderer::MapRenderer& GetRenderer();
        const std::string GetFilename();


    private:
        const json::Document json_requests_;
        std::unique_ptr<renderer::MapRenderer> renderer_;
        std::string filename_;
    };

} // namespace catalogue