# cpp-transport-catalogue
The "Transport Catalogue" project, which accepts requests in JSON format in two stages. 

Queries of the first stage are used to create a database (DB) of bus stops and routes, and the generated database is serialized. During the processing of the second stage of queries, the database is deserialized, and depending on the type of query, the user is given information about bus routes, stops, or the optimal route between two stops in terms of travel time, as well as a visualization of the route map. 

Output format: .json and .svg.

Проект "Транспортный справочник", принимающий запросы в формате JSON в две стадии. 

Запросы первой стадии служат для создания базы данных (БД) автобусных остановок и маршрутов, а сформированная база данных сериализуется. В ходе обработки второй стадии запросов БД десериализуется, и в зависимости от типа запроса пользователю выдаётся информация об автобусных маршрутах, остановках или оптимальном с точки зрения затрачиваемого на поездку времени маршрута между двумя остановками, а также визуализация карты маршрутов. 

Формат выходных данных: .json и .svg.
