#include "serialization.h"

namespace catalogue {

        DBHandler::DBHandler(const catalogue::TransportCatalogue &transport_catalogue,
                  const renderer::MapRenderer& renderer,
                  const TransportRouter& transport_router)
                : transport_catalogue_(transport_catalogue)
                , transport_router_(transport_router)
                , renderer_(renderer) {
        }

        transport_catalogue_serialize::TransportCatalogue DBHandler::SerializeTransportCatalogue() const {
            transport_catalogue_serialize::TransportCatalogue pbf_transport_catalogue;

            /// Stops serialiation
            std::vector<std::string_view> all_stops = transport_catalogue_.GetAllStopsNames();
            for (const std::string_view stopname : all_stops) {
                const Stop &stop = transport_catalogue_.FindStop(stopname);
                transport_catalogue_serialize::Stop pbf_stop;
                pbf_stop.set_name(stop.name);

                transport_catalogue_serialize::Coordinates pbf_coord;
                pbf_coord.set_lat(stop.coordinates.lat);
                pbf_coord.set_lng(stop.coordinates.lng);
                *pbf_stop.mutable_coordinates() = pbf_coord;

                pbf_stop.set_id_wait_start(stop.id_wait_start);
                pbf_stop.set_id_wait_end(stop.id_wait_end);

                *pbf_transport_catalogue.add_stops() = pbf_stop;
            }

            /// Distances serialiation
            for (const auto [stop_pair, distance] : transport_catalogue_.GetDistanceTable()) {
                transport_catalogue_serialize::Distance pbf_distance;
                auto it_l = std::find(all_stops.begin(), all_stops.end(), stop_pair.first->name);
                auto it_r = std::find(all_stops.begin(), all_stops.end(), stop_pair.second->name);
                pbf_distance.set_stop_id_l(it_l - all_stops.begin());
                pbf_distance.set_stop_id_r(it_r - all_stops.begin());
                pbf_distance.set_distance(distance);
                *pbf_transport_catalogue.add_distances() = pbf_distance;
            }

            /// Buses serialiation
            for (const std::string_view busname : transport_catalogue_.GetSortedBusNames()) {
                const Bus& bus = transport_catalogue_.FindBus(busname);
                transport_catalogue_serialize::Bus pbf_bus;
                pbf_bus.set_name(bus.name);

                for (Stop* bus_stop_ptr : bus.stops) {
                    auto it = std::find(all_stops.begin(), all_stops.end(), bus_stop_ptr->name);
                    if (it != all_stops.end()) {
                        pbf_bus.add_stop_ids(it - all_stops.begin());
                    }
                }
                pbf_bus.set_is_roundtrip(bus.is_roundtrip);
                *pbf_transport_catalogue.add_buses() = pbf_bus;
            }

            /// RoutingSettings serialization
            pbf_transport_catalogue.mutable_routing_settings()->set_bus_wait_time(transport_catalogue_.GetRoutingSettings().bus_wait_time);
            pbf_transport_catalogue.mutable_routing_settings()->set_bus_velocity(transport_catalogue_.GetRoutingSettings().bus_velocity);

            return pbf_transport_catalogue;
        }

        map_renderer_serialize::RenderSettings DBHandler::SerializeRenderSettings() const {
            map_renderer_serialize::RenderSettings pbf_render_settings;
            const RenderSettings& render_settings = renderer_.GetRenderSettings();

            pbf_render_settings.set_width(render_settings.width);
            pbf_render_settings.set_height(render_settings.height);
            pbf_render_settings.set_padding(render_settings.padding);
            pbf_render_settings.set_line_width(render_settings.line_width);
            pbf_render_settings.set_stop_radius(render_settings.stop_radius);
            pbf_render_settings.set_bus_label_font_size(render_settings.bus_label_font_size);
            for (const double bus_label_offset : render_settings.bus_label_offset) {
                pbf_render_settings.add_bus_label_offset(bus_label_offset);
            }
            pbf_render_settings.set_stop_label_font_size(render_settings.stop_label_font_size);
            for (const double stop_label_offset : render_settings.stop_label_offset) {
                pbf_render_settings.add_stop_label_offset(stop_label_offset);
            }

            if (const std::string* string_value = std::get_if<std::string>(&render_settings.underlayer_color)) {
                pbf_render_settings.mutable_underlayer_color()->set_name(*string_value);
            } else if (const svg::Rgb* rgb_value = std::get_if<svg::Rgb>(&render_settings.underlayer_color)) {
                Rgb rgb;
                rgb.set_red(rgb_value->red);
                rgb.set_green(rgb_value->green);
                rgb.set_blue(rgb_value->blue);
                *pbf_render_settings.mutable_underlayer_color()->mutable_rgb() = rgb;
            } else if (const svg::Rgba* rgba_value = std::get_if<svg::Rgba>(&render_settings.underlayer_color)) {
                Rgba rgba;
                rgba.set_red(rgba_value->red);
                rgba.set_green(rgba_value->green);
                rgba.set_blue(rgba_value->blue);
                rgba.set_opacity(rgba_value->opacity);
                *pbf_render_settings.mutable_underlayer_color()->mutable_rgba() = rgba;
            } else {
                pbf_render_settings.mutable_underlayer_color()->set_is_none(true);
            }

            pbf_render_settings.set_underlayer_width(render_settings.underlayer_width);

            for (svg::Color color_palette : render_settings.color_palette) {
                if (const std::string* string_value = std::get_if<std::string>(&color_palette)) {
                    pbf_render_settings.add_color_palette()->set_name(*string_value);
                } else if (const svg::Rgb* rgb_value = std::get_if<svg::Rgb>(&color_palette)) {
                    Rgb rgb;
                    rgb.set_red(rgb_value->red);
                    rgb.set_green(rgb_value->green);
                    rgb.set_blue(rgb_value->blue);
                    *pbf_render_settings.add_color_palette()->mutable_rgb() = rgb;
                } else if (const svg::Rgba* rgba_value = std::get_if<svg::Rgba>(&color_palette)) {
                    Rgba rgba;
                    rgba.set_red(rgba_value->red);
                    rgba.set_green(rgba_value->green);
                    rgba.set_blue(rgba_value->blue);
                    rgba.set_opacity(rgba_value->opacity);
                    *pbf_render_settings.add_color_palette()->mutable_rgba() = rgba;
                } else {
                    pbf_render_settings.add_color_palette()->set_is_none(true);
                }
            }
            return pbf_render_settings;
        }

        router_serialize::TransportRouter DBHandler::SerializeTransportRouter() const {
            router_serialize::TransportRouter transport_router;

            const graph::DirectedWeightedGraph<double>& graph = transport_router_.GetGraph();
            graph_serialize::Graph pbf_graph;

            for (size_t i = 0; i < graph.GetEdgeCount(); ++i) {
                graph_serialize::Edge pbf_edge;
                pbf_edge.set_from(graph.GetEdge(i).from);
                pbf_edge.set_to(graph.GetEdge(i).to);
                pbf_edge.set_weight(graph.GetEdge(i).weight);
                *pbf_graph.add_edges() = pbf_edge;
            }
            pbf_graph.set_vertex_count(graph.GetVertexCount());
            *transport_router.mutable_graph() = pbf_graph;

            const std::unordered_map<graph::EdgeId, RouteAct>& edge_id_to_route_act = transport_router_.GetEdgeIdToRouteAct();
            for (const auto& [id, route_act] : edge_id_to_route_act) {
                router_serialize::RouteAct pbf_route_act;
                if (RouteActType::WAIT == route_act.route_act_type) {
                    pbf_route_act.set_route_act_type(router_serialize::WAIT);
                } else {
                    pbf_route_act.set_route_act_type(router_serialize::BUS);
                }
                *pbf_route_act.mutable_name() = std::string(route_act.name);
                pbf_route_act.set_time(route_act.time);
                pbf_route_act.set_span_count(route_act.span_count);
                pbf_route_act.set_id(id);
                *transport_router.add_edge_id_to_route_act() = pbf_route_act;
            }

            return transport_router;
        }

        void DBHandler::SerializeToDB(const std::string_view filename) const {
            std::filesystem::path file_path(filename);
            std::ofstream output(file_path, std::ios::binary);

            transport_catalogue_serialize::Export pbf_serialize;
            *pbf_serialize.mutable_transport_catalogue() = SerializeTransportCatalogue();
            *pbf_serialize.mutable_render_settings() = SerializeRenderSettings();
            *pbf_serialize.mutable_transport_router() = SerializeTransportRouter();

            pbf_serialize.SerializeToOstream(&output);
        }



    TransportCatalogue DeserializeTransportCatalogue(transport_catalogue_serialize::Export pbf_serialized) {

        transport_catalogue_serialize::TransportCatalogue pbf_transport_catalogue = pbf_serialized.transport_catalogue();
        TransportCatalogue transport_catalogue;

        for (int i = 0; i < pbf_transport_catalogue.stops_size(); ++i) {
            const transport_catalogue_serialize::Stop& pbf_stop = pbf_transport_catalogue.stops(i);
            Stop stop{pbf_stop.name(), {pbf_stop.coordinates().lat(), pbf_stop.coordinates().lng()},
                      pbf_stop.id_wait_start(), pbf_stop.id_wait_end()};
            transport_catalogue.AddStop(stop);
        }

        std::unordered_map<std::pair<Stop*, Stop*>, uint32_t, TransportCatalogue::PairPointerHasher> distance_table;
        for (int i = 0; i < pbf_transport_catalogue.distances_size(); ++i) {
            const auto& distance_row = pbf_transport_catalogue.distances(i);
            distance_table[{&transport_catalogue.FindStop(pbf_transport_catalogue.stops(distance_row.stop_id_l()).name()),
                            &transport_catalogue.FindStop(pbf_transport_catalogue.stops(distance_row.stop_id_r()).name())}]
                            = distance_row.distance();
        }
        transport_catalogue.AddDistanceTable(distance_table);

        const std::vector<std::string_view> all_stops = transport_catalogue.GetAllStopsNames();
        for (int i = 0; i < pbf_transport_catalogue.buses_size(); ++i) {
            const transport_catalogue_serialize::Bus& pbf_bus = pbf_transport_catalogue.buses(i);
            Bus bus;
            bus.name = pbf_bus.name();
            bus.stops.reserve(pbf_bus.stop_ids_size());
            for (auto stop_id : pbf_bus.stop_ids()) {
                bus.stops.push_back(&transport_catalogue.FindStop(pbf_transport_catalogue.stops(stop_id).name()));
            }
            bus.is_roundtrip = pbf_bus.is_roundtrip();
            transport_catalogue.AddBus(bus);
        }
        transport_catalogue.AddRoutingSettings({pbf_transport_catalogue.routing_settings().bus_wait_time(),
                                                pbf_transport_catalogue.routing_settings().bus_velocity()});
        return transport_catalogue;
    }

    RenderSettings DeserializeRenderSettings(transport_catalogue_serialize::Export pbf_serialized) {
        RenderSettings render_settings;
        map_renderer_serialize::RenderSettings pbf_render_settings = pbf_serialized.render_settings();

        render_settings.width = pbf_render_settings.width();
        render_settings.height = pbf_render_settings.height();
        render_settings.padding = pbf_render_settings.padding();
        render_settings.line_width = pbf_render_settings.line_width();
        render_settings.stop_radius = pbf_render_settings.stop_radius();
        render_settings.bus_label_font_size = pbf_render_settings.bus_label_font_size();

        for (double bus_label_offset_element : pbf_render_settings.bus_label_offset()) {
            render_settings.bus_label_offset.push_back(bus_label_offset_element);
        }

        render_settings.stop_label_font_size = pbf_render_settings.stop_label_font_size();

        for (double stop_label_offset_element : pbf_render_settings.stop_label_offset()) {
            render_settings.stop_label_offset.push_back(stop_label_offset_element);
        }

        if (pbf_render_settings.underlayer_color().has_rgb()) {
            render_settings.underlayer_color = svg::Rgb(pbf_render_settings.underlayer_color().rgb().red(),
                                                        pbf_render_settings.underlayer_color().rgb().green(),
                                                        pbf_render_settings.underlayer_color().rgb().blue());
        } else if (pbf_render_settings.underlayer_color().has_rgba()) {
            render_settings.underlayer_color = svg::Rgba(pbf_render_settings.underlayer_color().rgba().red(),
                                                         pbf_render_settings.underlayer_color().rgba().green(),
                                                         pbf_render_settings.underlayer_color().rgba().blue(),
                                                         pbf_render_settings.underlayer_color().rgba().opacity());
        } else if (pbf_render_settings.underlayer_color().is_none()) {
            render_settings.underlayer_color = svg::NoneColor;
        } else {
            render_settings.underlayer_color = pbf_render_settings.underlayer_color().name();
        }

        render_settings.underlayer_width = pbf_render_settings.underlayer_width();

        for (Color color : pbf_render_settings.color_palette()) {
            if (color.has_rgb()) {
                render_settings.color_palette.push_back(svg::Rgb(color.rgb().red(),
                                                                 color.rgb().green(),
                                                                 color.rgb().blue()));
            } else if (color.has_rgba()) {
                render_settings.color_palette.push_back(svg::Rgba(color.rgba().red(),
                                                                  color.rgba().green(),
                                                                  color.rgba().blue(),
                                                                  color.rgba().opacity()));
            } else if (color.is_none()) {
                render_settings.color_palette.push_back(svg::NoneColor);
            } else {
                render_settings.color_palette.push_back(color.name());
            }
        }
        return render_settings;
    }

    TransportRouter DeserializeTransportRouter(transport_catalogue_serialize::Export pbf_serialized, const TransportCatalogue& db_) {


        /// routes internal data
        //router_serialize::RoutesInternalData pbf_routes_internal_data;
        //graph::Router<double> router;

        /// edge_id_to_route_act
        std::unordered_map<graph::EdgeId, RouteAct> edge_id_to_route_act;
        for (const auto& pbf_edge_id_to_route_act : pbf_serialized.transport_router().edge_id_to_route_act()) {
            RouteAct route_act;
            if (router_serialize::WAIT == pbf_edge_id_to_route_act.route_act_type()) {
                route_act.route_act_type = RouteActType::WAIT;
                route_act.name = db_.FindStop(pbf_edge_id_to_route_act.name()).name;
            } else {
                route_act.route_act_type = RouteActType::BUS;
                route_act.name = db_.FindBus(pbf_edge_id_to_route_act.name()).name;
            }

            route_act.time = pbf_edge_id_to_route_act.time();
            route_act.span_count = pbf_edge_id_to_route_act.span_count();

            edge_id_to_route_act[pbf_edge_id_to_route_act.id()] = route_act;
        }

        /// graph
        graph_serialize::Graph pbf_graph = pbf_serialized.transport_router().graph();
        graph::DirectedWeightedGraph<double> graph(pbf_graph.vertex_count());
        for (size_t i = 0; i < pbf_graph.edges_size(); ++i) {
            graph.AddEdge({pbf_graph.edges(i).from(), pbf_graph.edges(i).to(), pbf_graph.edges(i).weight()});
        }
        TransportRouter transport_router(db_, std::move(graph), std::move(edge_id_to_route_act));

        return transport_router;
    }

    transport_catalogue_serialize::Export Deserialize(const std::string_view filename) {
        std::filesystem::path file_path(filename);
        std::ifstream input(file_path, std::ios::binary);

        transport_catalogue_serialize::Export pbf_serialized;
        pbf_serialized.ParseFromIstream(&input);
        return pbf_serialized;
    }

} // namespace catalogue