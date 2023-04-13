#include "request_handler.h"

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
    const auto& stat_requests = json_requests.GetRoot().AsDict().at("stat_requests"s);
    for (const auto& request : stat_requests.AsArray()) {
        json::Node dict_node;
        if("Stop"s == request.AsDict().at("type"s).AsString()) {
            if (db_.FindStop(request.AsDict().at("name"s).AsString()).name.empty()) {
                dict_node = json::Builder{}.StartDict()
                                               .Key("error_message"s).Value("not found"s)
                                               .Key("request_id"s).Value(request.AsDict().at("id"s).AsInt())
                                           .EndDict()
                                           .Build();
            } else {
                const auto buses = GetStopInfo(request.AsDict().at("name"s).AsString());
                // формируем массив автобусов, проходящих через данную остановку
                json::Array buses_node;
                buses_node.reserve(buses.size());
                for (const auto &bus: buses) {
                    buses_node.push_back(std::move(json::Node{std::string(bus)}));
                }
                dict_node = json::Builder{}.StartDict()
                                               .Key("buses"s).Value(buses_node)
                                               .Key("request_id"s).Value(request.AsDict().at("id"s).AsInt())
                                           .EndDict()
                                           .Build();
            }
        } else if ("Map"s == request.AsDict().at("type"s).AsString()) {
            std::ostringstream out_stream;
            renderer_.RenderMap(db_).Render(out_stream);
            dict_node = json::Builder{}.StartDict()
                                           .Key("map"s).Value(out_stream.str())
                                           .Key("request_id"s).Value(request.AsDict().at("id"s).AsInt())
                                       .EndDict()
                                       .Build();
        } else {
            // случай запроса Bus
            if (db_.FindBus(request.AsDict().at("name"s).AsString()).name.empty()) {
                dict_node = json::Builder{}.StartDict()
                                               .Key("error_message"s).Value("not found"s)
                                               .Key("request_id"s).Value(request.AsDict().at("id"s).AsInt())
                                           .EndDict()
                                           .Build();
            } else {
                const auto bus_info = GetBusInfo(request.AsDict().at("name"s).AsString());
                dict_node = json::Builder{}.StartDict()
                                               .Key("curvature"s).Value(bus_info.route_length / bus_info.distance)
                                               .Key("request_id"s).Value(request.AsDict().at("id"s).AsInt())
                                               .Key("route_length"s).Value(static_cast<int>(bus_info.route_length))
                                               .Key("stop_count"s).Value(static_cast<int>(bus_info.num_stops))
                                               .Key("unique_stop_count"s).Value(static_cast<int>(bus_info.unique_stops))
                                           .EndDict()
                                           .Build();
            }
        }
        out.push_back(dict_node);
    }
    json::Document out_doc(json::Node{out});
    return out_doc;
}

