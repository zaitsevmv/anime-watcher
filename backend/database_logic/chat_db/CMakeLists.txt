find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(MONGOCXX REQUIRED libmongocxx)

add_library(chat_db STATIC chat_db.hpp chat_db.cpp)

target_link_libraries(chat_db PRIVATE
    base_mongodb
    mongocxx
    bsoncxx
    OpenSSL::SSL
    OpenSSL::Crypto
)

target_include_directories(chat_db PRIVATE
    ${MONGOCXX_INCLUDE_DIRS}
    ${DATABASE_LOGIC_DIRS}/base_mongodb
)