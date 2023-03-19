#include "transport_catalogue.h"

namespace catalogue {

    size_t TransportCatalogue::PairPointerHasher::operator()(const std::pair<Stop *, Stop *> &pointer_pair) const {
        return hasher_(pointer_pair.first) + 37 * hasher_(pointer_pair.second);
    }

    void TransportCatalogue::AddBus(const Bus& bus) {
        buses_.push_back(bus);
        busname_to_bus_[buses_.back().name] = &buses_.back();
        for (auto stop: bus.stops) {
            stopname_to_buses_[stop->name].insert(buses_.back().name);
        }

        for (auto it = FindBus(bus.name).stops.begin() + 1; it != FindBus(bus.name).stops.end(); ++it) {
            const auto prev_it = it - 1;
            FindBus(bus.name).distance += ComputeDistance(FindStop((*prev_it)->name).coordinates,
                                                          FindStop((*it)->name).coordinates);
            FindBus(bus.name).route_length += distance_table_.count({*prev_it, *it})
                                            ? distance_table_.at({*prev_it, *it})
                                            : distance_table_.at({*it, *prev_it});
        }
    }

//добавление остановки в базу,
    void TransportCatalogue::AddStop(const Stop& stop) {
        stops_.push_back(stop);
        stopname_to_stop_[stops_.back().name] = &stops_.back();
        stopname_to_buses_[stops_.back().name] = std::set<std::string_view>{};
    }

//поиск маршрута по имени
    Bus &TransportCatalogue::FindBus(const std::string_view bus_name) {
        if (busname_to_bus_.count(bus_name)) {
            return *busname_to_bus_.at(bus_name);
        }
        static Bus bus;
        return bus;
    }

//поиск остановки по имени
    Stop &TransportCatalogue::FindStop(const std::string_view stop_name) {
        if (stopname_to_stop_.count(stop_name)) {
            return *stopname_to_stop_.at(stop_name);
        }
        static Stop stop;
        return stop;
    }

//получение информации о маршруте
    TransportCatalogue::BusInfo TransportCatalogue::GetBusInfo(const std::string_view name) {
        TransportCatalogue::BusInfo out;
        const auto bus = TransportCatalogue::FindBus(name);
        out.name = name;

        size_t num_stops = bus.stops.size();
        out.num_stops = num_stops;

        std::set<Stop *> unique;
        std::for_each(bus.stops.begin(), bus.stops.end(), [&unique](const auto &stop) {
            unique.insert(stop);
        });
        out.unique_stops = unique.size();

        out.distance = bus.distance;
        out.route_length = bus.route_length;

        return out;
    }

    std::set<std::string_view> TransportCatalogue::GetStopInfo(const std::string_view name) {
        if (stopname_to_buses_.count(name)) {
            return stopname_to_buses_.at(name);
        }
        return std::set<std::string_view>{};
    }

    void TransportCatalogue::AddDistances(std::string_view stop,
                                          std::vector<std::pair<uint32_t, std::string_view>> neighbour_stops) {
        for (auto neighbour_stop: neighbour_stops) {
            distance_table_[{&FindStop(stop), &FindStop(neighbour_stop.second)}] = neighbour_stop.first;
        }
    }

}