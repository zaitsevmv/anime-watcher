#ifndef __BASE_MONGO_DB_H__
#define __BASE_MONGO_DB_H__

#include <mongocxx/collection.hpp>
#include <mongocxx/client.hpp>

#include <optional>
#include <string>

class BaseMongoDB{
protected:
    struct Filter{
        Filter(std::string field, std::string value)
            :bson_filter(bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp(field, value))) 
        { }

        auto get() const {
            return bson_filter;
        }

    private:
        bsoncxx::v_noabi::document::value bson_filter;
    };

public:
    std::optional<int32_t> CreateDatabase(const std::string& db_name, const std::string& collection_name);

    std::optional<int32_t> LoadCollectionData(const std::string& path);

    void DropCollection();

    ~BaseMongoDB();

protected:
    std::optional<std::string> GetDocument(const Filter& filter);

    std::optional<int32_t> AddDocument(const std::string& document_json);

    std::optional<int32_t> DeleteDocument(const Filter& filter);

    std::optional<std::string> UpdateDocument(const Filter& filter, const std::string& new_data_json);

    mongocxx::v_noabi::collection collection;
    mongocxx::client client;
};

#endif