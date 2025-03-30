set(DATABASE_LOGIC_DIRS ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(base_mongodb)

add_subdirectory(anime_db)      # mongodb
add_subdirectory(chat_db)       # mongodb
add_subdirectory(search_db)     # elastic search
add_subdirectory(user_name_db)  # redis
add_subdirectory(user_data_db)  # mongodb