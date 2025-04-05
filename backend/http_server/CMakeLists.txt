add_library(http_server STATIC server.hpp fields_alloc.hpp server.cpp requests_handler.cpp)

find_package(Boost REQUIRED COMPONENTS json)

include_directories(${Boost_INCLUDE_DIRS})

target_link_libraries(http_server PUBLIC
    ${Boost_LIBRARIES}
    Boost::json
    anime_db
    anime_search_db
    chat_db
    user_data_db
)

target_include_directories(http_server PUBLIC
    ${DATABASE_LOGIC_DIRS}
)