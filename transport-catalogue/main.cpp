#include <iostream>
#include <vector>
#include "input_reader.h"
#include "transport_catalogue.h"
#include "stat_reader.h"

using namespace catalogue;

int main() {
    //int num_requests = ReadLineWithNumber();
    std::vector<std::string> requests = add_requests::Input(std::cin, read::LineWithNumber());
    TransportCatalogue trans_cat;
    for (const auto stop : parse_requests::FirstIteration(requests)) {
        trans_cat.AddStop(std::move(stop));
    }

    const auto second_iter_parse = parse_requests::SecondIteration(requests);
    for (const auto [bus, stops] : second_iter_parse.bus) {
        Bus bus_to_add;
        bus_to_add.name = std::string(bus);
        for (auto stop: stops) {
            bus_to_add.stops.push_back(&trans_cat.FindStop(stop));
        }
        trans_cat.AddBus(std::move(bus_to_add));
    }
    for (const auto [stop, neighbour_stops] : second_iter_parse.distances) {
        trans_cat.AddDistances(stop, neighbour_stops);
    }

    const std::vector<std::string> output_requests = add_requests::Output(std::cin, read::LineWithNumber());
    output::PrintBus(trans_cat, output_requests);

    return 0;
}