#include "chat_db.hpp"
#include "mongodb_filters.hpp"

#include <bsoncxx/oid.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <chrono>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/json.hpp>

std::optional<uint64_t> ChatDB::AddMessage(const std::string& message_data) {
    return AddDocument(message_data);
}

std::optional<int> ChatDB::DeleteMessage(const std::string& message_uid) {
    return DeleteDocument({"uid", message_uid});
}

std::optional<std::vector<std::string>> ChatDB::GetNewMessages(const std::chrono::system_clock::time_point& last_update) {
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(last_update.time_since_epoch());
    auto earliest_id = bsoncxx::oid(std::to_string(timestamp.count()));
    auto search_result = collection.find(GreaterThanFilter("_id", earliest_id).get()); 
    std::vector<std::string> result;
    result.reserve(128);
    for(const auto& message: search_result){
        result.push_back(bsoncxx::to_json(message));
    }
    return result;
}

