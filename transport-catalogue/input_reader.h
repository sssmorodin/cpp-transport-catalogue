#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <istream>
#include <sstream>
#include <algorithm>

#include "geo.h"

using namespace std::string_literals;

namespace catalogue {

    struct DatabaseRequest {
        std::string type;
        std::string text;
    };

    struct Stop {
        std::string name;
        geo::Coordinates coordinates;
    };

    struct Bus {
        std::string name;
        std::vector<Stop*> stops;
    };

    struct SecondIterationParse {
        std::unordered_map<std::string_view, std::vector<std::string_view>> bus;
        std::unordered_map<std::string_view, std::vector<std::pair<uint32_t, std::string_view>>> distances;
    };

    namespace parse_requests {
        std::vector<Stop> FirstIteration(std::vector<std::string> &requests);
        SecondIterationParse SecondIteration(std::vector<std::string> &requests);
    }

    namespace add_requests {
        std::vector<std::string> Input(std::istream &input, int num_requests);
    }

    namespace read {
        std::string Line();
        int LineWithNumber();
    }

}