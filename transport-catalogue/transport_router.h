#pragma once

#include <unordered_map>
#include <memory>

#include "router.h"
#include "transport_catalogue.h"

namespace catalogue {

    class TransportRouter {
    public:
        constexpr static const double CONVERT_KMPH_TO_MPMIN = 1000.0 / 60.0;

        TransportRouter(const TransportCatalogue& db);

        const catalogue::RouteInfo BuildRoute(std::string_view from, std::string_view to) const;

    private:
        const TransportCatalogue& db_;
        graph::DirectedWeightedGraph<double> graph_;
        std::unique_ptr<graph::Router<double>> router_;
        std::unordered_map<graph::EdgeId, RouteAct> edge_id_to_route_act_;
    };

} // namespace catalogue
