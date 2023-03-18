#pragma once

#include <iostream>
#include <string_view>
#include <iomanip>
#include "transport_catalogue.h"

using namespace std::string_literals;

namespace catalogue {

    namespace add_requests {
        std::vector<std::string> Output(std::istream &input, int num_requests);
    }

    namespace output {
        void PrintBus(TransportCatalogue &catalogue, const std::vector<std::string> &requests);
    }

    namespace parse_requests {
        std::string_view Output(const std::string &requests);
    }
}