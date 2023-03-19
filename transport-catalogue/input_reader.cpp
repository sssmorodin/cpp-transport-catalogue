#include "input_reader.h"

namespace catalogue {

    namespace read {

        std::string Line(std::istream &input) {
            std::string s;
            getline(input, s);
            return s;
        }

        int LineWithNumber(std::istream &input) {
            int result;
            input >> result;
            read::Line(input);
            return result;
        }

    }

    namespace split {

        geo::Coordinates Coordinates(const std::string_view &text) {
            geo::Coordinates coordinates;
            auto coordinates_start = 0;
            auto coordinates_split = text.find(',');
            coordinates.lat = std::stod(
                    std::string(text.substr(coordinates_start, coordinates_split - coordinates_start)));
            coordinates_start = coordinates_split + 1;
            coordinates_split = text.find(',', coordinates_start);
            coordinates.lng = std::stod(
                    std::string(text.substr(coordinates_start, coordinates_split - coordinates_start)));
            return coordinates;
        }

        std::vector<std::string_view> StopsLine(const std::string_view &text) {
            std::vector<std::string_view> stops;
            int64_t pos = 0;
            const int64_t pos_end = text.npos;
            const char splitter = pos_end == text.find('>') ? '-' : '>';
            while (true) {
                int64_t split_mark = text.find(splitter, pos);
                stops.push_back(split_mark == pos_end ? text.substr(pos) : text.substr(pos, split_mark - pos));
                if (split_mark == pos_end) {
                    break;
                } else {
                    pos = split_mark + 1;
                }
            }
            std::vector<std::string_view> spaceless_stops;
            spaceless_stops.reserve(stops.size());
            for (const auto stop: stops) {
                const auto stop_start = stop.find_first_not_of(' ');
                const auto stop_end = stop.find_last_not_of(' ');
                spaceless_stops.push_back(stop.substr(stop_start, stop_end - stop_start + 1));
            }
            swap(stops, spaceless_stops);
            if ('-' == splitter) {
                std::vector<std::string_view> add_stops = stops;
                add_stops.reserve(2 * stops.size() - 1);
                for (auto it = stops.rbegin(); it != stops.rend(); ++it) {
                    if (stops.rbegin() == it) {
                        continue;
                    }
                    add_stops.push_back(*it);
                }
                swap(stops, add_stops);
            }
            return stops;
        }

// вызывается обработчиком запроса добавления остановки
// получает на вход часть запросаи, содержащую длины до соседних остановок
// возвращает вектор пар <расстояние, название> до соседних остановок
        std::vector<std::pair<uint32_t, std::string_view>> Distances(const std::string_view &text) {
            std::vector<std::pair<uint32_t, std::string_view>> out;
            int64_t part_start = 0;
            const int64_t pos_end = text.npos;
            while (true) {
                auto part_end = text.find(',', part_start);
                int64_t pos_m = text.find('m', part_start);
                int64_t pos_to = text.find("to", part_start);
                pos_to += 2;
                uint32_t distance = std::stoi(std::string(text.substr(part_start, pos_m - part_start)));
                auto stop_name_start = text.find_first_not_of(' ', pos_to);
                //auto stop_name_end   = text.find_last_not_of(' ', stop_name_start);
                std::string_view stop = text.substr(stop_name_start, part_end - stop_name_start);
                out.push_back({distance, stop});
                if (part_end == pos_end) {
                    break;
                } else {
                    part_start = part_end + 1;
                }
            }
            return out;
        }

    }

    namespace add_requests {

        std::vector<std::string> Input(std::istream &input, int num_requests) {
            std::vector<std::string> queue;
            for (int i = 0; i < num_requests; ++i) {
                std::string request;
                getline(input, request);
                queue.push_back(request);
            }
            return queue;
        }

    }

    namespace parse_requests {

        std::vector<Stop> FirstIteration(std::vector<std::string> &requests) {
            std::vector<Stop> out;
            for (const std::string_view request: requests) {
                if (request.substr(0, 4) == "Stop"s) {
                    std::string_view request_text = request.substr(5, request.size() - 5);
                    auto stop_name_pos_start = request_text.find_first_not_of(' ');
                    auto stop_name_pos_end = request_text.find(':');
                    Stop stop;
                    stop.name = request_text.substr(stop_name_pos_start, stop_name_pos_end - stop_name_pos_start);
                    geo::Coordinates coordinates = split::Coordinates(
                            request_text.substr(stop_name_pos_end + 1, request_text.back()));
                    stop.coordinates = coordinates;
                    out.push_back(stop);
                }
            }
            return out;
        }

        SecondIterationParse SecondIteration(std::vector<std::string> &requests) {
            SecondIterationParse out;
            //std::vector<std::pair<std::string_view, std::vector<std::string_view>>> out;
            for (const std::string_view request: requests) {
                if (request.substr(0, 3) == "Bus"s) {
                    std::string_view request_text = request.substr(4, request.size() - 4);
                    std::pair<std::string_view, std::vector<std::string_view>> bus;
                    auto bus_name_pos_start = request_text.find_first_not_of(' ');
                    auto bus_name_pos_end = request_text.find(':');
                    const auto bus_name = request_text.substr(bus_name_pos_start,
                                                              bus_name_pos_end - bus_name_pos_start);
                    const std::string_view stops = request_text.substr(bus_name_pos_end + 1,
                                                                       request_text.size() - bus_name_pos_end);
                    const auto bus_stops = split::StopsLine(stops);
                    out.bus[bus_name] = bus_stops;
                }
                if (request.substr(0, 4) == "Stop"s) {
                    std::string_view request_text = request.substr(5, request.size() - 5);
                    auto stop_name_pos_start = request_text.find_first_not_of(' ');
                    auto stop_name_pos_end = request_text.find(':');
                    std::string_view stop_name = request_text.substr(stop_name_pos_start,
                                                                     stop_name_pos_end - stop_name_pos_start);
                    auto pos = request_text.find(',');
                    pos = request_text.find(',', pos + 1);
                    if (request_text.npos == pos) {
                        continue;
                    }
                    std::string_view distances_sv = request_text.substr(pos + 1);
                    auto distances = split::Distances(distances_sv);
                    out.distances[stop_name] = distances;
                }
            }
            return out;
        }
    }

}