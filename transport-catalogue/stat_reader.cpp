#include "stat_reader.h"

namespace catalogue {

    namespace parse_requests {

        std::string_view Output(const std::string_view request) {
            std::string_view out;
            std::string_view request_text = request.substr(4, request.size() - 4);
            auto bus_name_pos_start = request_text.find_first_not_of(' ');
            auto bus_name_pos_end = request_text.find_last_not_of(' ');
            out = request_text.substr(bus_name_pos_start, bus_name_pos_end - bus_name_pos_start + 1);
            return out;
        }
    }

    namespace add_requests {

        std::vector<std::string> Output(std::istream &input, int num_requests) {
            std::vector<std::string> queue;
            for (int i = 0; i < num_requests; ++i) {
                std::string request;
                getline(input, request);
                queue.push_back(request);
            }
            return queue;
        }

    }

    namespace output {

        void PrintBus(TransportCatalogue &catalogue, const std::vector<std::string> &requests) {
            for (const std::string_view request: requests) {
                const std::string_view request_text = parse_requests::Output(request);
                if (request.substr(0, 3) == "Bus"s) {
                    if (catalogue.FindBus(request_text).stops.size()) {
                        const TransportCatalogue::BusInfo current_bus = catalogue.GetBusInfo(request_text);
                        std::cout << "Bus "s << current_bus.name << ": "s
                                  << current_bus.num_stops << " stops on route, "
                                  << current_bus.unique_stops << " unique stops, " << std::setprecision(6)
                                  << current_bus.route_length << " route length, " << std::setprecision(6)
                                  << current_bus.route_length / current_bus.distance << " curvature"
                                  << std::endl;
                    } else {
                        std::cout << "Bus "s << request_text << ": "s
                                  << "not found"s << std::endl;
                    }
                } else {
                    //вывод по запросу по остановке
                    if (catalogue.FindStop(request_text).name.empty()) {
                        std::cout << "Stop "s << request_text << ": not found"s << std::endl;
                        continue;
                    }
                    const std::set<std::string_view> current_stop_buses = catalogue.GetStopInfo(request_text);
                    if (current_stop_buses.empty()) {
                        std::cout << "Stop "s << request_text << ": no buses"s << std::endl;
                    } else {
                        std::cout << "Stop "s << request_text << ": buses"s;
                        for (const auto bus: current_stop_buses) {
                            std::cout << ' ' << bus;
                        }
                        std::cout << std::endl;
                    }
                }
            }
        }
    }

}