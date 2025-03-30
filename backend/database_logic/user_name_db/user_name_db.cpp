#include "user_name_db.hpp"

#include <redis-cpp/stream.h>
#include <redis-cpp/execute.h>

#include <iostream>

UserNameDB::UserNameDB(){
    stream = rediscpp::make_stream("localhost", "6379");
}

std::optional<std::string> UserNameDB::SetUserName(uint64_t user_id, const std::string& user_name) {
    auto response = rediscpp::execute(*stream, "set", user_id, user_name, "ex", "60");
    std::cout << "Setting user name. Got response: " << response.as_string() << std::endl;
}

std::optional<std::string> UserNameDB::GetUserName(uint64_t user_id) {
    auto response = rediscpp::execute(*stream, "get", user_id);
    std::cout << "Getting user " << user_id << " name. Got response: " << response.as_string() << std::endl;
}