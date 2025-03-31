cmake_minimum_required(VERSION 3.12)

project(anime-backend)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(database_logic)
add_subdirectory(http_server)

target_link_libraries(http_server PRIVATE ${Boost_LIBRARIES})

add_executable(main main.cpp)
target_link_libraries(main PRIVATE http_server anime_db)