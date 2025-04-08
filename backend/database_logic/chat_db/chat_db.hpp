#ifndef __CHAT_DB_H__
#define __CHAT_DB_H__

#include "base_mongodb.hpp"

#include <mongocxx/collection.hpp>
#include <mongocxx/client.hpp>

#include <optional>
#include <string>
#include <chrono>

class ChatDB: public BaseMongoDB{
public:
    ChatDB(const std::string& db_name, const std::string& collection_name)
        : BaseMongoDB(db_name, collection_name) {}

    std::optional<std::string> AddMessage(const std::string& message_data);

    std::optional<int32_t> DeleteMessage(const std::string& message_id);

    std::optional<int32_t> DeleteUserMessages(const std::string& user_id);
    
    std::optional<std::vector<std::string>> GetNewMessages(const int64_t last_update_ms);
};

#endif