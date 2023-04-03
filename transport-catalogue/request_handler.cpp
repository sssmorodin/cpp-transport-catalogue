#include "request_handler.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */
using namespace catalogue;

RequestHandler::RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer)
    : db_(db)
    , renderer_(renderer) {
}

TransportCatalogue::BusInfo RequestHandler::GetBusInfo(const std::string_view& name) {
    TransportCatalogue::BusInfo out;
    const auto bus = db_.FindBus(name);
    out.name = name;

    size_t num_stops = bus.is_roundtrip ? bus.stops.size() : (2 * bus.stops.size() - 1);
    out.num_stops = num_stops;

    std::set<Stop *> unique;
    std::for_each(bus.stops.begin(), bus.stops.end(), [&unique](const auto &stop) {
        unique.insert(stop);
    });
    out.unique_stops = unique.size();

    out.distance = bus.distance;
    out.route_length = bus.route_length;

    return out;
}

std::set<std::string_view> RequestHandler::GetStopInfo(const std::string_view& name) const {
        return db_.GetStopInfo(name);
}

json::Document RequestHandler::MakeJSONDocument(const json::Document& json_requests) {
    json::Array out;
    const auto& stat_requests = json_requests.GetRoot().AsMap().at("stat_requests"s);
    for (const auto& request : stat_requests.AsArray()) {
        if("Stop"s == request.AsMap().at("type"s).AsString()) {
            json::Node dict_node;
            if (db_.FindStop(request.AsMap().at("name"s).AsString()).name.empty()) {
                dict_node = json::Node{json::Dict{{"error_message"s, "not found"s},
                                                  {"request_id"s,    request.AsMap().at("id"s).AsInt()}}};
            } else {
                const auto buses = GetStopInfo(request.AsMap().at("name"s).AsString());
                // формируем массив автобусов, проходящих через данную остановку
                json::Array buses_node;
                buses_node.reserve(buses.size());
                for (const auto &bus: buses) {
                    buses_node.push_back(std::move(json::Node{std::string(bus)}));
                }
                dict_node = json::Node{json::Dict{{"buses"s,      buses_node},
                                                  {"request_id"s, request.AsMap().at("id"s).AsInt()}}};
            }
            out.push_back(dict_node);
        } else if ("Map"s == request.AsMap().at("type"s).AsString()) {
            std::ostringstream out_stream;
            renderer_.RenderMap(db_).Render(out_stream);
            json::Node dict_node = json::Dict{{"map"s, json::Node(out_stream.str())},
                                              {"request_id"s, request.AsMap().at("id"s).AsInt()}};
            out.push_back(dict_node);
        } else {
            // случай запроса Bus
            json::Node dict_node;
            if (db_.FindBus(request.AsMap().at("name"s).AsString()).name.empty()) {
                dict_node = json::Node{json::Dict{ {"error_message"s, "not found"s},
                                                   {"request_id"s, request.AsMap().at("id"s).AsInt()} }};
            } else {
                const auto bus_info = GetBusInfo(request.AsMap().at("name"s).AsString());
                dict_node = json::Node{json::Dict{{"curvature"s, bus_info.route_length / bus_info.distance},
                                                  {"request_id"s, request.AsMap().at("id"s).AsInt()},
                                                  {"route_length"s, static_cast<int>(bus_info.route_length)},
                                                  {"stop_count"s, static_cast<int>(bus_info.num_stops)},
                                                  {"unique_stop_count"s, static_cast<int>(bus_info.unique_stops)}
                }};
            }
            out.push_back(dict_node);
        }
    }
    json::Document out_doc(json::Node{out});
    return out_doc;
}

