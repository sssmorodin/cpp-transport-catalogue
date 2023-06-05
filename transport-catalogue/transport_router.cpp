#include "transport_router.h"

namespace catalogue {

    TransportRouter::TransportRouter(const TransportCatalogue& db)
            : db_(db)
            , graph_(graph::DirectedWeightedGraph<double>(2 * db.GetAllStopsNames().size())) {
        const RoutingSettings& routing_settings = db_.GetRoutingSettings();
        // two graph vertex used for each Stop
        for (const auto stop_name : db_.GetAllStopsNames()) {
            const Stop& stop = db_.FindStop(stop_name);
            const auto edge_id = graph_.AddEdge({ stop.id_wait_start, stop.id_wait_end, static_cast<double>(routing_settings.bus_wait_time)});
            edge_id_to_route_act_[edge_id] = RouteAct{RouteActType::WAIT, stop_name, static_cast<double>(routing_settings.bus_wait_time), 0};
        }
        graph::EdgeId edge_id;
        for (const auto bus_name : db_.GetSortedBusNames()) {
            const Bus& bus = db_.FindBus(bus_name);
            const auto& num_of_stops = bus.stops.size();
            for (size_t i = 0; i < num_of_stops - 1; ++i) {
                double weight = 0.0;
                double back_weight = 0.0;
                for (size_t j = i + 1; j < num_of_stops; ++j) {
                    weight += db_.GetDistance(bus.stops[j - 1], bus.stops[j]) / (static_cast<double>(routing_settings.bus_velocity) * CONVERT_KMPH_TO_MPMIN);
                    edge_id = graph_.AddEdge({bus.stops[i]->id_wait_end, bus.stops[j]->id_wait_start, weight});
                    edge_id_to_route_act_[edge_id] = RouteAct{RouteActType::BUS, bus_name, weight, j - i};
                    if (!bus.is_roundtrip) {
                        back_weight += db_.GetDistance(bus.stops[j], bus.stops[j - 1]) / (static_cast<double>(routing_settings.bus_velocity) * CONVERT_KMPH_TO_MPMIN);
                        edge_id = graph_.AddEdge({bus.stops[j]->id_wait_end, bus.stops[i]->id_wait_start, back_weight});
                        edge_id_to_route_act_[edge_id] = RouteAct{RouteActType::BUS, bus_name, back_weight, j - i};
                    }
                }
            }
        }
        router_ = std::make_unique<graph::Router<double>>(graph_);
    }

    TransportRouter::TransportRouter(const TransportCatalogue& db, graph::DirectedWeightedGraph<double>&& graph,
                                     const std::unordered_map<graph::EdgeId, RouteAct>&& edge_id_to_route_act)
            : db_(db)
            , graph_(std::move(graph))
            , edge_id_to_route_act_(std::move(edge_id_to_route_act)) {
        router_ = std::make_unique<graph::Router<double>>(graph_);
    }


    const catalogue::RouteInfo TransportRouter::BuildRoute(std::string_view from, std::string_view to) const {
        catalogue::RouteInfo out;
        auto route_info = router_->BuildRoute(db_.FindStop(from).id_wait_start, db_.FindStop(to).id_wait_start);
        if (route_info.has_value()) {
            out.total_time = route_info.value().weight;
            const auto& edges = route_info.value().edges;
            out.items.reserve(edges.size());
            for (const auto& edge : edges) {
                out.items.push_back(&edge_id_to_route_act_.at(edge));
            }
        } else {
            out.not_found_flag = true;
        }
        return out;
    }

    const graph::Router<double>& TransportRouter::GetRouter() const {
        return *router_.get();
    }

    const graph::DirectedWeightedGraph<double>& TransportRouter::GetGraph() const {
        return graph_;
    }

    const std::unordered_map<graph::EdgeId, RouteAct>& TransportRouter::GetEdgeIdToRouteAct() const {
        return edge_id_to_route_act_;
    }
} // namespace catalogue
