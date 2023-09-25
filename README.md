# cpp-transport-catalogue
## Project overview / Общая информация

#### The "Transport Catalogue" project, which accepts requests in JSON format in two stages. 

First stage requests are used to create a database of bus stops and routes, and the generated database is being serialized. During the processing of the second stage requestss the database is being deserialized, and depending on the type of request, the user is given information about bus routes, stops, or the optimal route between two stops in terms of travel time, as well as a visualization of the route map. 

Output format: .json and .svg.

#### Проект "Транспортный справочник", принимающий запросы в формате JSON в две стадии. 

Запросы первой стадии служат для создания базы данных автобусных остановок и маршрутов, а сформированная база данных сериализуется. В ходе обработки второй стадии запросов база данных десериализуется, и в зависимости от типа запроса пользователю выдаётся информация об автобусных маршрутах, остановках или оптимальном с точки зрения затрачиваемого на поездку времени маршрута между двумя остановками, а также визуализация карты маршрутов. 

Формат выходных данных: .json и .svg.

## Requirements / Требования
C++17

##  Tech stack / Стек технологий

1. CMake 3.11.0
2. protobuf-3.21.9
