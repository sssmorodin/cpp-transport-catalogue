#include "request_handler.h"

using namespace catalogue;

RequestHandler::RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer,
                               const TransportRouter& transport_router)
        : db_(db)
        , renderer_(renderer)
        , transport_router_(transport_router) {
}

TransportCatalogue::BusInfo RequestHandler::GetBusInfo(const std::string_view& name) const {
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

catalogue::RouteInfo RequestHandler::GetRouteInfo(const std::string_view& from, const std::string_view& to) const {
    return transport_router_.BuildRoute(from, to);
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
        } else if ("Bus"s == request.AsDict().at("type"s).AsString()) {
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
        } else if ("Route"s == request.AsDict().at("type"s).AsString()) {
            const auto route_info = GetRouteInfo(request.AsDict().at("from"s).AsString(),
                                                 request.AsDict().at("to"s).AsString());

            json::Dict temp_dict_node;
            temp_dict_node.insert({ "request_id"s, request.AsDict().at("id"s).AsInt() });

            if (route_info.not_found_flag) {
                temp_dict_node.insert({ "error_message"s, "not found"s });
            } else {
                temp_dict_node.insert({ "total_time"s, route_info.total_time });

                json::Array items;
                for (auto& item : route_info.items) {
                    json::Dict item_node;
                    switch (item->route_act_type) {
                        case RouteActType::WAIT:
                            item_node.insert({ "type"s, "Wait"s });
                            item_node.insert({ "stop_name"s, std::string(item->name) });
                            item_node.insert({ "time"s, item->time });
                            break;
                        case RouteActType::BUS:
                            item_node.insert({ "type"s, "Bus"s });
                            item_node.insert({ "bus"s, std::string(item->name) });
                            item_node.insert({ "span_count"s, static_cast<int>(item->span_count) });
                            item_node.insert({ "time"s, item->time });
                            break;
                    }
                    items.push_back(item_node);
                }

                temp_dict_node.insert({ "items"s, items });
            }
            dict_node = std::move(temp_dict_node);
        }
        out.push_back(dict_node);
    }
    json::Document out_doc(json::Node{out});
    return out_doc;
}

const TransportRouter& RequestHandler::GetTransportRouter() const {
    return transport_router_;
}

