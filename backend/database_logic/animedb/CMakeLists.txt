find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(MONGOCXX REQUIRED libmongocxx)

add_library(animedb animedb.hpp animedb.cpp)

target_link_libraries(animedb PRIVATE mongocxx bsoncxx OpenSSL::SSL OpenSSL::Crypto)
target_include_directories(animedb PRIVATE ${MONGOCXX_INCLUDE_DIRS})