find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(MONGOCXX REQUIRED libmongocxx)

add_library(user_data_db STATIC user_data_db.hpp user_data_db.cpp)

target_link_libraries(user_data_db PRIVATE
    base_mongodb
    mongocxx
    bsoncxx
    OpenSSL::SSL
    OpenSSL::Crypto
)

target_include_directories(user_data_db PRIVATE
    ${MONGOCXX_INCLUDE_DIRS}
    ${DATABASE_LOGIC_DIRS}/base_mongodb
)