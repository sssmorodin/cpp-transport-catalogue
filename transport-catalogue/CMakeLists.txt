cmake_minimum_required(VERSION 3.11)
project(s15_final)

set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalogue.proto map_renderer.proto
                                            svg.proto graph.proto transport_router.proto)

set(TRANSPORT_CATALOGUE_FILES json_builder.cpp
        router.h
        json_reader.h
        serialization.h
        transport_router.h
        map_renderer.h
        map_renderer.cpp
        json_builder.h
        svg.h
        domain.h
        ranges.h
        svg.cpp
        json.cpp
        request_handler.cpp
        geo.h
        transport_router.cpp
        serialization.cpp
        request_handler.h
        transport_catalogue.h
        domain.cpp
        graph.h
        geo.cpp
        json_reader.cpp
        transport_catalogue.cpp
        json.h
        main.cpp
        transport_catalogue.proto
        map_renderer.proto
        svg.proto)

add_executable(transport_catalogue ${PROTO_SRCS} ${PROTO_HDRS} ${TRANSPORT_CATALOGUE_FILES})
target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "libprotobuf.a" "libprotobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "libprotoc.a" "libprotocd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)