#pragma once

#include <iostream>
#include <string_view>
#include <iomanip>
#include <optional>
#include <cassert>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json.h"
#include "json_builder.h"
#include "graph.h"
#include "transport_router.h"

using namespace std::string_literals;

namespace catalogue {
    class RequestHandler {
    public:
        RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer);
        RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer,
                       const TransportRouter& transport_router);

        // Возвращает информацию о маршруте (запрос Bus)
        TransportCatalogue::BusInfo GetBusInfo(const std::string_view& name) const;

        // Возвращает маршруты, проходящие через остановку
        std::set<std::string_view> GetStopInfo(const std::string_view& name) const;

        // Возвращает JSON документ с информацией по обработанным запросам
        json::Document MakeJSONDocument(const json::Document& json_requests);

        // Возвращает информацию о построенном маршруте
        catalogue::RouteInfo GetRouteInfo(const std::string_view& from, const std::string_view& to) const;

        const TransportRouter& GetTransportRouter() const;

    private:
        // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
        const TransportCatalogue& db_;
        const renderer::MapRenderer& renderer_;
        const TransportRouter& transport_router_;
    };

}