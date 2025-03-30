find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(MONGOCXX REQUIRED libmongocxx)

add_library(anime_db STATIC anime_db.hpp anime_db.cpp)

target_link_libraries(anime_db PRIVATE
    base_mongodb
    mongocxx
    bsoncxx
    OpenSSL::SSL
    OpenSSL::Crypto
)

target_include_directories(anime_db PRIVATE
    ${MONGOCXX_INCLUDE_DIRS}
    ${DATABASE_LOGIC_DIRS}/base_mongodb
)