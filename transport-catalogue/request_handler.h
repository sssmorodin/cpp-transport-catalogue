#pragma once

#include <iostream>
#include <string_view>
#include <iomanip>
#include <optional>
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json.h"
#include "json_builder.h"

using namespace std::string_literals;

namespace catalogue {
    class RequestHandler {
    public:
        RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer);

        // Возвращает информацию о маршруте (запрос Bus)
        TransportCatalogue::BusInfo GetBusInfo(const std::string_view& name);

        // Возвращает маршруты, проходящие через остановку
        std::set<std::string_view> GetStopInfo(const std::string_view& name) const;

        // Возвращает JSON документ с информацией по обработанным запросам
        json::Document MakeJSONDocument(const json::Document& json_requests);

    private:
        // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
        const TransportCatalogue& db_;
        const renderer::MapRenderer& renderer_;
    };

}