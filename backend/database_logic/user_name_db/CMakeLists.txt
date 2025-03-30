add_library(user_name_db STATIC user_name_db.hpp user_name_db.cpp)

target_include_directories(user_name_db PRIVATE ${REDIS_CPP_LIB_DIR})