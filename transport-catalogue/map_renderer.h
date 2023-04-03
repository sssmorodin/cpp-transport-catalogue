#pragma once

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */
#include "geo.h"
#include "svg.h"
#include "domain.h"
#include "transport_catalogue.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <set>

namespace catalogue {

    using namespace std::string_literals;

    struct RenderSettings {
        double width;
        double height;

        double padding;

        double line_width;
        double stop_radius;

        uint32_t bus_label_font_size;
        std::vector<double> bus_label_offset;

        uint32_t stop_label_font_size;
        std::vector<double> stop_label_offset;

        svg::Color underlayer_color;
        double underlayer_width;

        std::vector<svg::Color> color_palette;
    };

    bool IsZero(double value);

    class SphereProjector {
    public:
        // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
        template<typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                        double max_width, double max_height, double padding);

        // Проецирует широту и долготу в координаты внутри SVG-изображения
        svg::Point operator()(geo::Coordinates coords) const;

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

    template<typename PointInputIt>
    SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                                     double max_width, double max_height, double padding)
        : padding_(padding)
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

namespace renderer {
    class MapRenderer {
    public:
        MapRenderer(const RenderSettings render_settings)
            : settings_(render_settings) {
        }
        struct ProjectionSettings {
            double width;
            double height;
            double padding;
        };

        const ProjectionSettings GetProjectionSettings() const;

        const svg::Color& GetColor(int i) const;

        svg::Polyline DrawRoute(const Bus& bus, int counter, const SphereProjector& sphere_projector) const;
        std::vector<svg::Text> DrawBusName(const Bus& bus, int counter, const SphereProjector& sphere_projector) const;
        const svg::Circle DrawStopSymbol(geo::Coordinates coordinates, const SphereProjector& sphere_projector) const;
        const std::pair<svg::Text, svg::Text> DrawStopName(const Stop& stop, const SphereProjector& sphere_projector) const;
        svg::Document RenderMap(const TransportCatalogue& db_) const;

    private:
        const RenderSettings settings_;
    };
}

} // namespace catalogue