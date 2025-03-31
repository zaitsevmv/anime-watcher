add_library(anime_db STATIC anime_db.hpp anime_db.cpp)

target_link_libraries(anime_db PUBLIC
    base_mongodb
)

target_include_directories(anime_db PUBLIC
    ${DATABASE_LOGIC_DIRS}/base_mongodb
)