#include <iostream>
#include <vector>
#include "input_reader.h"
#include "transport_catalogue.h"
#include "stat_reader.h"

using namespace catalogue;

int main() {
    std::vector<std::string> requests = add_requests::Input(std::cin, read::LineWithNumber(std::cin));
    TransportCatalogue transport_catalogue;
    for (const auto stop : parse_requests::FirstIteration(requests)) {
        transport_catalogue.AddStop(stop);
    }

    const auto second_iter_parse = parse_requests::SecondIteration(requests);
    for (const auto [stop, neighbour_stops] : second_iter_parse.distances) {
        transport_catalogue.AddDistances(stop, neighbour_stops);
    }
    for (const auto [bus, stops] : second_iter_parse.bus) {
        Bus bus_to_add;
        bus_to_add.name = std::string(bus);
        for (auto stop: stops) {
            bus_to_add.stops.push_back(&transport_catalogue.FindStop(stop));
        }
        transport_catalogue.AddBus(bus_to_add);
    }

    const std::vector<std::string> output_requests = add_requests::Output(std::cin, read::LineWithNumber(std::cin));
    output::PrintBus(std::cout, transport_catalogue, output_requests);

    return 0;
}