find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(MONGOCXX REQUIRED libmongocxx)

add_library(base_mongodb STATIC mongodb_filters.hpp base_mongodb.hpp base_mongodb.cpp)

target_link_libraries(base_mongodb PRIVATE 
    mongocxx 
    bsoncxx 
    OpenSSL::SSL 
    OpenSSL::Crypto
)

target_include_directories(base_mongodb PRIVATE ${MONGOCXX_INCLUDE_DIRS})