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
        if (!bus.is_roundtrip) {
            for (auto it = FindBus(bus.name).stops.rbegin() + 1; it != FindBus(bus.name).stops.rend(); ++it) {
                const auto prev_it = it - 1;
                FindBus(bus.name).distance += ComputeDistance(FindStop((*prev_it)->name).coordinates,
                                                              FindStop((*it)->name).coordinates);
                FindBus(bus.name).route_length += distance_table_.count({*prev_it, *it})
                                                  ? distance_table_.at({*prev_it, *it})
                                                  : distance_table_.at({*it, *prev_it});
            }
        }
    }

//добавление остановки в базу,
    void TransportCatalogue::AddStop(const Stop& stop) {
        stops_.push_back(stop);
        stopname_to_stop_[stops_.back().name] = &stops_.back();
        stopname_to_buses_[stops_.back().name] = std::set<std::string_view>{};
    }

//поиск маршрута по имени
    Bus &TransportCatalogue::FindBus(const std::string_view bus_name) const {
        if (busname_to_bus_.count(bus_name)) {
            return *busname_to_bus_.at(bus_name);
        }
        static Bus bus;
        return bus;
    }

//поиск остановки по имени
    Stop &TransportCatalogue::FindStop(const std::string_view stop_name) const {
        if (stopname_to_stop_.count(stop_name)) {
            return *stopname_to_stop_.at(stop_name);
        }
        static Stop stop;
        return stop;
    }

    std::set<std::string_view> TransportCatalogue::GetStopInfo(const std::string_view name) const {
        if (stopname_to_buses_.count(name)) {
            return stopname_to_buses_.at(name);
        }
        return std::set<std::string_view>{};
    }

    void TransportCatalogue::AddDistances(std::string_view stop,
                                          const std::vector<std::pair<uint32_t, std::string_view>>& neighbour_stops) {
        for (auto neighbour_stop: neighbour_stops) {
            distance_table_[{&FindStop(stop), &FindStop(neighbour_stop.second)}] = neighbour_stop.first;
            if (!distance_table_[{&FindStop(neighbour_stop.second), &FindStop(stop)}]) {
                distance_table_[{&FindStop(neighbour_stop.second), &FindStop(stop)}] = neighbour_stop.first;
            }
        }
    }

    // возвращает координаты всех остановок, через которые проходит хотя бы один маршрут
	const std::vector<geo::Coordinates> TransportCatalogue::GetAllStopsCoordinates() const {
        std::vector<geo::Coordinates> out;
        out.reserve(stops_.size());
        for (const auto& stop : stops_) {
            if (!GetStopInfo(stop.name).empty()) {
                out.push_back(stop.coordinates);
            }
        }
        return out;
    }

    const std::set<std::string_view> TransportCatalogue::GetSortedBusNames() const {
        std::set<std::string_view> out;
        for (const auto& bus : buses_) {
            if (!bus.stops.empty()) {
                out.insert(bus.name);
            }
        }
        return out;
    }

    // возвращает отсортированные названия всех остановок, через которые проходит хотя бы один маршрут
    const std::set<std::string_view> TransportCatalogue::GetAllStopsNames() const {
        std::set<std::string_view> out;
        for (const auto& stop : stops_) {
            if (!GetStopInfo(stop.name).empty()) {
                out.insert(stop.name);
            }
        }
        return out;
    }

}