#include <fstream>
#include <iostream>
#include <string_view>

#include "json_reader.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "serialization.h"

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);


    if (mode == "make_base"sv) {

        catalogue::JSONReader json_reader(std::cin);
        catalogue::TransportCatalogue transport_catalogue = json_reader.BuildTransportCatalogue();
        const auto map_renderer = json_reader.GetRenderer();

        catalogue::TransportRouter transport_router{transport_catalogue};

        catalogue::DBHandler db_handler(transport_catalogue, map_renderer, transport_router);
        db_handler.SerializeToDB(json_reader.GetFilename());

    } else if (mode == "process_requests"sv) {

        catalogue::JSONReader json_reader(std::cin);

        transport_catalogue_serialize::Export pbf_deserialize = catalogue::Deserialize(json_reader.GetFilename());

        const auto transport_catalogue = catalogue::DeserializeTransportCatalogue(pbf_deserialize);
        const catalogue::renderer::MapRenderer map_renderer(catalogue::DeserializeRenderSettings(pbf_deserialize));
        catalogue::TransportRouter transport_router = DeserializeTransportRouter(pbf_deserialize, transport_catalogue);

        catalogue::RequestHandler request_handler(transport_catalogue, map_renderer, transport_router);
        json::Document out_doc = request_handler.MakeJSONDocument(json_reader.GetJSONRequests());
        //json::Print(out_doc, std::cout);
        std::ofstream out_txt("out.txt");
        json::Print(out_doc, out_txt);
    } else {
        PrintUsage();
        return 1;
    }
}