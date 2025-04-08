#include "base_mongodb.hpp"
#include "mongodb_filters.hpp"

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <cstdint>
#include <cstdio>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/json.hpp>

#include <fstream>
#include <optional>
#include <filesystem>

BaseMongoDB::BaseMongoDB(const std::string& db_name, const std::string& collection_name)
    : client(mongocxx::uri("mongodb://admin:secret@localhost:27017"))
{
    if(client[db_name].has_collection(collection_name)){
        collection = client[db_name][collection_name];
    } else{
        collection = client[db_name].create_collection(collection_name);
    }
}

std::optional<int32_t> BaseMongoDB::LoadCollectionData(const std::string& path) {
    std::ifstream input_document(path);
    if(!input_document) return std::nullopt;
    auto input_size = std::filesystem::file_size(path);
    std::string data(input_size, '\0');
    input_document.read(data.data(), data.size());
    auto bson_document = bsoncxx::from_json(data);
    input_document.close();
    // auto result = collection.insert_many(bson_document.view());
    // if(result){
    //     return result->inserted_count();
    // }
    return std::nullopt;
}

void BaseMongoDB::DropCollection() {
    collection.drop();
}

std::optional<std::string> BaseMongoDB::GetDocument(const SearchFilter& filter) {
    auto search_result = collection.find_one(filter.get());
    if(search_result){
        return bsoncxx::to_json(*search_result);
    }
    return std::nullopt;
}

std::optional<std::string> BaseMongoDB::AddDocument(const std::string& document_json) {
    auto bson_document = bsoncxx::from_json(document_json);
    auto add_result = collection.insert_one(bson_document.view());
    if(add_result){
        return add_result->inserted_id().get_oid().value.to_string();  // returns the amount of added documents
    }
    return std::nullopt;
}

std::optional<int32_t> BaseMongoDB::DeleteDocument(const SearchFilter& filter) {
    auto deletion_result = collection.delete_one(filter.get());
    if(deletion_result){
        return deletion_result->deleted_count();
    }
    return std::nullopt;
}

std::optional<int32_t> BaseMongoDB::DeleteAll(const SearchFilter& filter) {
    auto deletion_result = collection.delete_many(filter.get());
    if(deletion_result){
        return deletion_result->deleted_count();
    }
    return std::nullopt;
}

std::optional<std::string> BaseMongoDB::UpdateDocument(const SearchFilter& filter, const std::string& new_data_json) {
    auto bson_document = bsoncxx::from_json(new_data_json);
    auto update_result = collection.find_one_and_update(filter.get(), bson_document.view());
    if(update_result){
        return bsoncxx::to_json(*update_result);
    }
    return std::nullopt;
}