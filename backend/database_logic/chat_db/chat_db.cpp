#include "chat_db.hpp"

#include "mongodb_filters.hpp"
#include <bsoncxx/oid.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/json.hpp>
#include <chrono>

std::optional<int32_t> ChatDB::AddMessage(const std::string& message_data) {
    return AddDocument(message_data);
}

std::optional<int32_t> ChatDB::DeleteMessage(const int64_t message_id) {
    return DeleteDocument(SearchFilter("_id", message_id));
}

std::optional<std::vector<std::string>> ChatDB::GetNewMessages(const int64_t last_update_ms) {
    auto search_result = collection.find(GreaterThanFilter("timestamp_ms", last_update_ms).get()); 
    std::vector<std::string> result;
    result.reserve(128);
    for(const auto& message: search_result){
        result.push_back(bsoncxx::to_json(message));
    }
    return result;
}

