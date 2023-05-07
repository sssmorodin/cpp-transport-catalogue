#include "json_reader.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

namespace catalogue {

        std::deque<Stop> JSONReader::FirstIteration(const json::Document& json_requests) {
            const auto& base_requests = json_requests.GetRoot().AsDict().at("base_requests"s);
            std::deque<Stop> stops;
            size_t i = 0;
            for (const auto& request : base_requests.AsArray()) {
                Stop current_stop;
                if("Stop"s == request.AsDict().at("type"s).AsString()) {
                    current_stop.name = request.AsDict().at("name"s).AsString();
                    current_stop.coordinates = {request.AsDict().at("latitude"s).AsDouble(),
                                                request.AsDict().at("longitude"s).AsDouble()};
                    current_stop.id_wait_start = i++;
                    current_stop.id_wait_end = i++;
                    stops.push_back(current_stop);
                }
            }
            return stops;
        }

    JSONReader::SecondIterationParse JSONReader::SecondIteration(const json::Document& json_requests) {
            const auto &base_requests = json_requests.GetRoot().AsDict().at("base_requests"s);

            SecondIterationParse out;
            for (const auto &request: base_requests.AsArray()) {
                if ("Bus"s == request.AsDict().at("type"s).AsString()) {
                    const auto &bus_name = request.AsDict().at("name"s).AsString();
                    const auto &bus_stops = request.AsDict().at("stops"s).AsArray();
                    for (const auto &bus_stop: bus_stops) {
                        out.bus[bus_name].push_back(bus_stop.AsString());
                    }
                    out.busname_to_roundtrip[bus_name] = request.AsDict().at("is_roundtrip"s).AsBool();
                }
                if ("Stop"s == request.AsDict().at("type"s).AsString()) {
                    std::string_view stop_name = request.AsDict().at("name"s).AsString();
                    const auto& road_distances = request.AsDict().at("road_distances"s);
                    auto distances = Distances(road_distances);
                    out.distances[stop_name] = distances;
                }
            }
            return out;
        }

        std::vector<std::pair<uint32_t, std::string_view>> JSONReader::Distances(const json::Node& node) {
            std::vector<std::pair<uint32_t, std::string_view>> out;
            for (const auto& [stop, distance] : node.AsDict()) {
                out.push_back({distance.AsInt(), stop});
            }
            return out;
        }
		
		svg::Color JSONReader::GetColor(const json::Node color) {
            svg::Color out;
            if (color.IsString()) {
                out = color.AsString();
            } else if (3 == color.AsArray().size()) {
                out = svg::Rgb(color.AsArray()[0].AsInt(),
                               color.AsArray()[1].AsInt(),
                               color.AsArray()[2].AsInt());
            } else {
                out = svg::Rgba(color.AsArray()[0].AsInt(),
                                color.AsArray()[1].AsInt(),
                                color.AsArray()[2].AsInt(),
                                color.AsArray()[3].AsDouble());
            }
            return out;
        }

        RenderSettings JSONReader::ReadRenderSettings() {
            RenderSettings out;
            const auto &render_settings = json_requests_.GetRoot().AsDict().at("render_settings"s);

            out.width = render_settings.AsDict().at("width"s).AsDouble();
            out.height = render_settings.AsDict().at("height"s).AsDouble();

            out.padding = render_settings.AsDict().at("padding"s).AsDouble();
            out.stop_radius = render_settings.AsDict().at("stop_radius"s).AsDouble();
            out.line_width = render_settings.AsDict().at("line_width"s).AsDouble();

            out.bus_label_font_size = render_settings.AsDict().at("bus_label_font_size"s).AsInt();
            for (const auto offset : render_settings.AsDict().at("bus_label_offset"s).AsArray()) {
                out.bus_label_offset.push_back(offset.AsDouble());
            }

            out.stop_label_font_size = render_settings.AsDict().at("stop_label_font_size"s).AsInt();
            for (const auto offset : render_settings.AsDict().at("stop_label_offset"s).AsArray()) {
                out.stop_label_offset.push_back(offset.AsDouble());
            }

            out.underlayer_color = GetColor(render_settings.AsDict().at("underlayer_color"s));

            out.underlayer_width = render_settings.AsDict().at("underlayer_width"s).AsDouble();
            for (const auto& color : render_settings.AsDict().at("color_palette"s).AsArray()) {
                out.color_palette.push_back(GetColor(color));
            }
            return out;
        }

    JSONReader::JSONReader(std::istream &input)
            : json_requests_(json::Load(input))
            , renderer_(ReadRenderSettings())
    {
        for (const auto& stop : FirstIteration(json_requests_)) {
            transport_catalogue_.AddStop(stop);
        }
        const auto second_iter_parse = SecondIteration(json_requests_);
        for (const auto& [stop, neighbour_stops] : second_iter_parse.distances) {
            transport_catalogue_.AddDistances(stop, neighbour_stops);
        }
        for (const auto& [bus, stops] : second_iter_parse.bus) {
            Bus bus_to_add;
            bus_to_add.name = std::string(bus);
            for (const auto& stop: stops) {
                bus_to_add.stops.push_back(&transport_catalogue_.FindStop(stop));
            }
            bus_to_add.is_roundtrip = second_iter_parse.busname_to_roundtrip.at(bus);
            transport_catalogue_.AddBus(bus_to_add);
        }
        transport_catalogue_.AddRoutingSettings(ReadRouteSettings(json_requests_));
    }

    // RoutingSettings routing_settings
    RoutingSettings JSONReader::ReadRouteSettings(const json::Document& json_requests) {
        RoutingSettings out;
        const auto &routing_settings = json_requests.GetRoot().AsDict().at("routing_settings"s);
        out.bus_wait_time = routing_settings.AsDict().at("bus_wait_time"s).AsInt();
        out.bus_velocity = routing_settings.AsDict().at("bus_velocity"s).AsDouble();
        return out;
    }

    const json::Document& JSONReader::GetJSONRequests() {
            return json_requests_;
    }
    const TransportCatalogue& JSONReader::GetTransportCatalogue() {
            return transport_catalogue_;
    }
    const renderer::MapRenderer& JSONReader::GetRenderer() {
        return renderer_;
    }


} // namespace catalogue