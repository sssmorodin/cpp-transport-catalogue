#pragma once

#include <string_view>
#include <fstream>
#include <filesystem>
#include <variant>

#include <transport_catalogue.pb.h>
#include <map_renderer.pb.h>
#include <svg.pb.h>
#include <transport_router.pb.h>
#include <graph.pb.h>

#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

namespace catalogue {

    class DBHandler {
    public:
        DBHandler(const catalogue::TransportCatalogue &transport_catalogue,
                  const catalogue::renderer::MapRenderer& renderer,
                  const catalogue::TransportRouter& transport_router);

        transport_catalogue_serialize::TransportCatalogue SerializeTransportCatalogue() const;

        map_renderer_serialize::RenderSettings SerializeRenderSettings() const;

        router_serialize::TransportRouter SerializeTransportRouter() const ;

        void SerializeToDB(const std::string_view filename) const;

    private:
        const catalogue::TransportCatalogue &transport_catalogue_;
        const catalogue::renderer::MapRenderer& renderer_;
        const catalogue::TransportRouter& transport_router_;
    };

    TransportCatalogue DeserializeTransportCatalogue(transport_catalogue_serialize::Export pbf_serialized);

    catalogue::RenderSettings DeserializeRenderSettings(transport_catalogue_serialize::Export pbf_serialized);

    catalogue::TransportRouter DeserializeTransportRouter(transport_catalogue_serialize::Export pbf_serialized, const TransportCatalogue& db_);

    transport_catalogue_serialize::Export Deserialize(const std::string_view filename);

} // namespace catalogue