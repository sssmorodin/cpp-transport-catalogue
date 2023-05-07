#include "map_renderer.h"

namespace catalogue {
    inline const double EPSILON = 1e-6;
    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

// Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
        return {
                (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

    namespace renderer {

        const MapRenderer::ProjectionSettings MapRenderer::GetProjectionSettings() const {
            ProjectionSettings out;
            out.width = settings_.width;
            out.height = settings_.height;
            out.padding = settings_.padding;
            return out;
        }

        const svg::Color& MapRenderer::GetColor(int i) const {
            return settings_.color_palette[i];
        }

        svg::Polyline MapRenderer::DrawRoute(const Bus& bus, int counter,
                                const SphereProjector& sphere_projector) const {
            svg::Polyline out;
            for (const auto& bus_stop : bus.stops) {
                out.AddPoint(sphere_projector(bus_stop->coordinates));
            }

            if (!bus.is_roundtrip) {
                for (auto it = bus.stops.rbegin() + 1; it != bus.stops.rend(); ++it) {
                    out.AddPoint(sphere_projector((*it)->coordinates));
                }
            }

            out.SetStrokeColor(settings_.color_palette[counter % settings_.color_palette.size()])
                    .SetFillColor("none"s)
                    .SetStrokeWidth(settings_.line_width)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            return out;
        }
        std::vector<svg::Text> MapRenderer::DrawBusName(const Bus& bus, int counter,
                                           const SphereProjector& sphere_projector) const {
            std::vector<svg::Text> out;
            svg::Text route_start_underlayer;
            route_start_underlayer.SetPosition(sphere_projector(bus.stops.front()->coordinates))
                    .SetOffset({settings_.bus_label_offset.front(), settings_.bus_label_offset.back()})
                    .SetFontSize(settings_.bus_label_font_size)
                    .SetFontFamily("Verdana"s)
                    .SetFontWeight("bold"s)
                    .SetData(bus.name);
            svg::Text route_start_text = route_start_underlayer;

            route_start_underlayer.SetFillColor(settings_.underlayer_color)
                    .SetStrokeColor(settings_.underlayer_color)
                    .SetStrokeWidth(settings_.underlayer_width)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            route_start_text.SetFillColor(settings_.color_palette[counter % settings_.color_palette.size()]);
            out.push_back(route_start_underlayer);
            out.push_back(route_start_text);
            if (bus.stops.front() != bus.stops.back()) {
                route_start_text.SetPosition(sphere_projector(bus.stops.back()->coordinates));
                route_start_underlayer.SetPosition(sphere_projector(bus.stops.back()->coordinates));
                out.push_back(route_start_underlayer);
                out.push_back(route_start_text);
            }
            return out;
        }
        const svg::Circle MapRenderer::DrawStopSymbol(geo::Coordinates coordinates,
                                         const SphereProjector& sphere_projector) const {
            svg::Circle circle;
            circle.SetCenter(sphere_projector(coordinates))
                    .SetRadius(settings_.stop_radius)
                    .SetFillColor("white"s);
            return circle;
        }
        const std::pair<svg::Text, svg::Text> MapRenderer::DrawStopName(const Stop& stop,
                                                           const SphereProjector& sphere_projector) const {
            svg::Text route_start_underlayer;
            route_start_underlayer.SetPosition(sphere_projector(stop.coordinates))
                    .SetOffset({settings_.stop_label_offset.front(), settings_.stop_label_offset.back()})
                    .SetFontSize(settings_.stop_label_font_size)
                    .SetFontFamily("Verdana"s)
                    .SetData(stop.name);
            svg::Text route_start_text = route_start_underlayer;

            route_start_underlayer.SetFillColor(settings_.underlayer_color)
                    .SetStrokeColor(settings_.underlayer_color)
                    .SetStrokeWidth(settings_.underlayer_width)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            route_start_text.SetFillColor("black"s);
            return {route_start_underlayer, route_start_text};
        }

        svg::Document MapRenderer::RenderMap(const TransportCatalogue& db_) const {
            svg::Document document;
            //формируем вектор координат всех остановок
            const auto all_stops_coord = db_.GetAllStopsCoordinates();
            const auto buses = db_.GetSortedBusNames();

            // отрисовка линий маршрутов
            const SphereProjector sphere_projector(all_stops_coord.begin(), all_stops_coord.end(),
                                                   settings_.width, settings_.height,
                                                   settings_.padding);
            size_t counter = 0;
            for (const auto& bus_name : buses) {
                const auto bus = db_.FindBus(bus_name);
                svg::Polyline route_draw = DrawRoute(bus, counter, sphere_projector);
                document.Add(std::move(route_draw));
                ++counter;
            }
            // отрисовка названий маршрутов
            counter = 0;
            for (const auto& bus_name : buses) {
                const auto bus = db_.FindBus(bus_name);
                if (bus.stops.empty()) {
                    continue;
                }
                std::vector<svg::Text> route_name = DrawBusName(bus, counter, sphere_projector);
                for (const auto& figure : route_name) {
                    document.Add(std::move(figure));
                }
                ++counter;
            }

            // отрисовка круглых обозначний остановок
            const std::set<std::string_view> stops_names = db_.GetSortedOnRouteStopsNames();
            for (const auto& stop_name : stops_names) {
                svg::Circle circle = DrawStopSymbol(db_.FindStop(stop_name).coordinates, sphere_projector);
                document.Add(std::move(circle));
            }

            // отрисовка названий остановок
            for (const auto& stop_name : stops_names) {
                std::pair<svg::Text, svg::Text> stop = DrawStopName(db_.FindStop(stop_name), sphere_projector);
                document.Add(std::move(stop.first));
                document.Add(std::move(stop.second));
            }
            return document;
        }

    } // namespace renderer

} // namespace catalogue



