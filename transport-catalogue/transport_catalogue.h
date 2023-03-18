#pragma once

#include <string>
#include <unordered_map>
#include <deque>
#include <vector>
#include <algorithm>
#include <set>

#include "input_reader.h"
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

        // добавление маршрута в базу,
        void AddBus(Bus bus);

        //добавление остановки в базу,
        void AddStop(Stop stop);

        void AddDistances(std::string_view stop, std::vector<std::pair<uint32_t, std::string_view>> neighbour_stops);

        //поиск маршрута по имени,
        Bus& FindBus(const std::string_view bus_name);

        //поиск остановки по имени
        Stop& FindStop(const std::string_view stop_name);

        //получение информации о маршруте
        BusInfo GetBusInfo(const std::string_view name);

        std::set<std::string_view> GetStopInfo(const std::string_view name);

    private:
        std::deque<Stop> stops_;
        std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
        std::deque<Bus> buses_;
        std::unordered_map<std::string_view, Bus*> busname_to_bus_;
        std::unordered_map<std::string_view, std::set<std::string_view>> stopname_to_buses_;
        std::unordered_map<std::pair<Stop*, Stop*>, uint32_t, PairPointerHasher> distance_table_;
    };

}
