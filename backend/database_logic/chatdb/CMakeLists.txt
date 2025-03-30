find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(MONGOCXX REQUIRED libmongocxx)

add_library(chatdb STATIC animedb.hpp animedb.cpp)

target_link_libraries(chatdb PRIVATE
    base_mongodb
    mongocxx
    bsoncxx
    OpenSSL::SSL
    OpenSSL::Crypto
)

target_include_directories(chatdb PRIVATE
    ${MONGOCXX_INCLUDE_DIRS}
    ${DATABASE_LOGIC_DIRS}/base_mongodb
)