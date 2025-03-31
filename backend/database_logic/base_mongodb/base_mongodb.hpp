#ifndef __BASE_MONGO_DB_H__
#define __BASE_MONGO_DB_H__

#include "mongodb_filters.hpp"

#include <bsoncxx/document/view.hpp>
#include <bsoncxx/types/bson_value/value.hpp>
#include <mongocxx/collection.hpp>
#include <mongocxx/client.hpp>

#include <optional>
#include <string>

class BaseMongoDB{
public:
    std::optional<int32_t> CreateDatabase(const std::string& db_name, const std::string& collection_name);

    std::optional<int32_t> LoadCollectionData(const std::string& path);

    void DropCollection();
    
protected:
    std::optional<std::string> GetDocument(const SearchFilter& filter);

    std::optional<int32_t> AddDocument(const std::string& document_json);

    std::optional<int32_t> DeleteDocument(const SearchFilter& filter);

    std::optional<std::string> UpdateDocument(const SearchFilter& filter, const std::string& new_data_json);

    mongocxx::v_noabi::collection collection;
    mongocxx::client client;
};

#endif