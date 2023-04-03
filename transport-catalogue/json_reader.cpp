#include "json_reader.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

namespace catalogue {

    namespace parse_requests {

        std::deque<Stop> FirstIteration(const json::Document& json_requests) {
            const auto& base_requests = json_requests.GetRoot().AsMap().at("base_requests"s);
            std::deque<Stop> stops;
            for (const auto& request : base_requests.AsArray()) {
                Stop current_stop;
                if("Stop"s == request.AsMap().at("type"s)) {
                    current_stop.name = request.AsMap().at("name"s).AsString();
                    current_stop.coordinates = {request.AsMap().at("latitude"s).AsDouble(),
                                                request.AsMap().at("longitude"s).AsDouble()};
                    stops.push_back(current_stop);
                }
            }
            return stops;
        }

        SecondIterationParse SecondIteration(const json::Document& json_requests) {
            const auto &base_requests = json_requests.GetRoot().AsMap().at("base_requests"s);

            SecondIterationParse out;
            for (const auto &request: base_requests.AsArray()) {
                if ("Bus"s == request.AsMap().at("type"s)) {
                    //Bus current_bus;
                    const auto &bus_name = request.AsMap().at("name"s).AsString();
                    const auto &bus_stops = request.AsMap().at("stops"s).AsArray();
                    for (const auto &bus_stop: bus_stops) {
                        out.bus[bus_name].push_back(bus_stop.AsString());
                    }
                    out.busname_to_roundtrip[bus_name] = request.AsMap().at("is_roundtrip"s).AsBool();

                }
                if ("Stop"s == request.AsMap().at("type"s)) {
                    std::string_view stop_name = request.AsMap().at("name"s).AsString();
                    const auto& road_distances = request.AsMap().at("road_distances"s);
                    auto distances = Distances(road_distances);
                    out.distances[stop_name] = distances;
                }
            }
            return out;
        }

        std::vector<std::pair<uint32_t, std::string_view>> Distances(const json::Node& node) {
            std::vector<std::pair<uint32_t, std::string_view>> out;
            for (const auto& [stop, distance] : node.AsMap()) {
                out.push_back({distance.AsInt(), stop});
            }
            return out;
        }
		
		svg::Color GetColor(const json::Node color) {
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

        RenderSettings ReadRenderSettings(const json::Document& json_requests) {
            RenderSettings out;
            const auto &render_settings = json_requests.GetRoot().AsMap().at("render_settings"s);

            out.width = render_settings.AsMap().at("width"s).AsDouble();
            out.height = render_settings.AsMap().at("height"s).AsDouble();

            out.padding = render_settings.AsMap().at("padding"s).AsDouble();
            out.stop_radius = render_settings.AsMap().at("stop_radius"s).AsDouble();
            out.line_width = render_settings.AsMap().at("line_width"s).AsDouble();

            out.bus_label_font_size = render_settings.AsMap().at("bus_label_font_size"s).AsInt();
            for (const auto offset : render_settings.AsMap().at("bus_label_offset"s).AsArray()) {
                out.bus_label_offset.push_back(offset.AsDouble());
            }

            out.stop_label_font_size = render_settings.AsMap().at("stop_label_font_size"s).AsInt();
            for (const auto offset : render_settings.AsMap().at("stop_label_offset"s).AsArray()) {
                out.stop_label_offset.push_back(offset.AsDouble());
            }

            out.underlayer_color = GetColor(render_settings.AsMap().at("underlayer_color"s));

            out.underlayer_width = render_settings.AsMap().at("underlayer_width"s).AsDouble();
            for (const auto& color : render_settings.AsMap().at("color_palette"s).AsArray()) {
                out.color_palette.push_back(GetColor(color));
            }
            return out;
        }

    } // namespace parse_requests

} // namespace catalogue