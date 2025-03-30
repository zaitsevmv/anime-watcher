add_library(http_server STATIC server.hpp fields_alloc.hpp server.cpp)

find_package(Boost REQUIRED)
message("${BOOST_VERSION}")
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(http_server LINK_PUBLIC ${Boost_LIBRARIES})
