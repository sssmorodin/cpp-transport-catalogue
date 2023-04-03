#include <iostream>
#include <vector>
#include "json_reader.h"
#include "request_handler.h"
#include "map_renderer.h"

using namespace catalogue;

int main() {
    JSONReader json_reader(std::cin);
    const auto transport_catalogue = json_reader.GetTransportCatalogue();
    const auto map_renderer = json_reader.GetRenderer();
    RequestHandler request_handler(transport_catalogue, map_renderer);

    json::Document out_doc = request_handler.MakeJSONDocument(json_reader.GetJSONRequests());
    json::Print(out_doc, std::cout);

    return 0;
}

