#include "user_name_db.hpp"

#include <redis-cpp/stream.h>
#include <redis-cpp/execute.h>

#include <iostream>

UserNameDB::UserNameDB(){
    stream = rediscpp::make_stream("localhost", "6379");
}

std::optional<std::string> UserNameDB::SetUserName(int64_t user_id, const std::string& user_name) {
    auto response = rediscpp::execute(*stream, "set", std::to_string(user_id), user_name, "ex", "60");
    std::cout << "Setting user name. Got response: " << response.as_string() << std::endl;
    return "";
}

std::optional<std::string> UserNameDB::GetUserName(int64_t user_id) {
    auto response = rediscpp::execute(*stream, "get", std::to_string(user_id));
    std::cout << "Getting user " << user_id << " name. Got response: " << response.as_string() << std::endl;
    return "";
}