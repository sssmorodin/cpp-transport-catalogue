#include <iostream>
#include <vector>
#include "json_reader.h"
//#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"

using namespace catalogue;

int main() {
    const auto json_requests = json::Load(std::cin);

    TransportCatalogue transport_catalogue;
    const renderer::MapRenderer map_renderer(parse_requests::ReadRenderSettings(json_requests));
    RequestHandler request_handler(transport_catalogue, map_renderer);

    for (const auto& stop : parse_requests::FirstIteration(json_requests)) {
        transport_catalogue.AddStop(stop);
    }
    const auto second_iter_parse = parse_requests::SecondIteration(json_requests);
    for (const auto& [stop, neighbour_stops] : second_iter_parse.distances) {
        transport_catalogue.AddDistances(stop, neighbour_stops);
    }
    for (const auto& [bus, stops] : second_iter_parse.bus) {
        Bus bus_to_add;
        bus_to_add.name = std::string(bus);
        for (const auto& stop: stops) {
            bus_to_add.stops.push_back(&transport_catalogue.FindStop(stop));
        }
        bus_to_add.is_roundtrip = second_iter_parse.busname_to_roundtrip.at(bus);
        transport_catalogue.AddBus(bus_to_add);
    }

    json::Document out_doc = request_handler.MakeJSONDocument(json_requests);
    json::Print(out_doc, std::cout);

    return 0;
}

