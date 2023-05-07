#pragma once

#include <string>
#include <unordered_map>
#include <deque>
#include <vector>
#include <algorithm>
#include <set>

#include "domain.h"
#include "geo.h"

namespace catalogue {

    class TransportCatalogue {

    public:
        struct BusInfo {
            std::string name;
            size_t num_stops;
            size_t unique_stops;
            double distance;
            uint64_t route_length;
        };

        struct StopInfo {
            std::string name;
            size_t num_stops;
        };

        struct PairPointerHasher {
            size_t operator()(const std::pair<Stop*, Stop*>& pointer_pair) const;
            std::hash<const void*> hasher_;
        };

        // добавление маршрута в базу
        void AddBus(const Bus& bus);

        // добавление остановки в базу
        void AddStop(const Stop& stop);

        // функция принимает название остановки и вектор пар дальность-название (до) соседней остановки
        void AddDistances(std::string_view stop, 
                          const std::vector<std::pair<uint32_t, std::string_view>>& neighbour_stops);

        void AddRoutingSettings(RoutingSettings routing_settings);

        // поиск маршрута по имени
        Bus& FindBus(const std::string_view bus_name) const;

        // поиск остановки по имени
        Stop& FindStop(const std::string_view stop_name) const;

        // получение информации об остановке
        std::set<std::string_view> GetStopInfo(const std::string_view name) const;
		
		const std::vector<geo::Coordinates> GetAllStopsCoordinates() const;

        // возвращает сет названий маршрутов, имеющих остановки
        const std::set<std::string_view> GetSortedBusNames() const;

        // возвращает отсортированные названия всех остановок, через которые проходит хотя бы один маршрут
        const std::set<std::string_view> GetSortedOnRouteStopsNames() const;

        const RoutingSettings& GetRoutingSettings() const;

        // get distance using distance_table_
        uint32_t GetDistance(Stop* from, Stop* to) const;

        const std::vector<std::string_view> GetAllStopsNames() const;

    private:
        std::deque<Stop> stops_;
        std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
        std::deque<Bus> buses_;
        std::unordered_map<std::string_view, Bus*> busname_to_bus_;
        std::unordered_map<std::string_view, std::set<std::string_view>> stopname_to_buses_;
        std::unordered_map<std::pair<Stop*, Stop*>, uint32_t, PairPointerHasher> distance_table_;
        RoutingSettings routing_settings_;
    };

}
