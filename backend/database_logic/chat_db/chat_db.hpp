#ifndef __CHAT_DB_H__
#define __CHAT_DB_H__

#include "base_mongodb.hpp"

#include <mongocxx/collection.hpp>
#include <mongocxx/client.hpp>

#include <optional>
#include <string>
#include <chrono>

class ChatDB: private BaseMongoDB{
public:
    std::optional<uint64_t> AddMessage(const std::string& message_data);
    std::optional<int> DeleteMessage(const std::string& message_uid);
    std::optional<std::vector<std::string>> GetNewMessages(const std::chrono::system_clock::time_point& last_update);
};

#endif