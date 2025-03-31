find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)

add_library(search_db STATIC search_db.hpp search_db.cpp)

target_link_libraries(search_db PRIVATE
    OpenSSL::Crypto
    OpenSSL::SSL
    CURL::libcurl
)

target_include_directories(search_db PRIVATE
    ${CURL_INCLUDE_DIRS}
)