#include "chatdb.hpp"

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/json.hpp>

std::optional<uint64_t> ChatDB::AddMessage(const std::string& message_data) {
    return AddDocument(message_data);
}

std::optional<int> ChatDB::DeleteMessage(const std::string& message_uid) {
    return DeleteDocument({"uid", message_uid});
}

std::optional<std::string> ChatDB::GetNewMessages(const std::chrono::steady_clock::time_point& last_update) {
    
}

