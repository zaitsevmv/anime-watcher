add_library(chat_db STATIC chat_db.hpp chat_db.cpp)

target_link_libraries(chat_db PUBLIC
    base_mongodb
    mongocxx
)

target_include_directories(chat_db PUBLIC
    ${DATABASE_LOGIC_DIRS}/base_mongodb
)