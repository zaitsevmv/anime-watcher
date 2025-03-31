find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)

add_library(anime_search_db STATIC anime_search_db.hpp anime_search_db.cpp)

target_link_libraries(anime_search_db PUBLIC
    OpenSSL::Crypto
    OpenSSL::SSL
    CURL::libcurl
)

target_include_directories(anime_search_db PUBLIC
    ${CURL_INCLUDE_DIRS}
)