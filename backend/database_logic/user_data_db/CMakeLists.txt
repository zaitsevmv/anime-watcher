add_library(user_data_db STATIC user_data_db.hpp user_data_db.cpp)

target_link_libraries(user_data_db PUBLIC
    base_mongodb
)

target_include_directories(user_data_db PUBLIC
    ${DATABASE_LOGIC_DIRS}/base_mongodb
)