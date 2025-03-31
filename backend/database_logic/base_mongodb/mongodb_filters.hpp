#ifndef __BASIC_FILTERS_H__
#define __BASIC_FILTERS_H__

#include <bsoncxx/document/view.hpp>
#include <bsoncxx/types/bson_value/value.hpp>
#include <mongocxx/collection.hpp>
#include <mongocxx/client.hpp>

struct Filter{
    auto get() const {
        return bson_filter;
    }

protected:
    bsoncxx::v_noabi::document::view bson_filter;
};

struct SearchFilter: public Filter{
    SearchFilter(std::string&& field, bsoncxx::types::bson_value::value&& value){
        bson_filter = bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp(field, value));
    }
};

struct SetFieldFilter: public Filter{
    SetFieldFilter(std::string&& field, bsoncxx::types::bson_value::value&& value) { 
        bson_filter = bsoncxx::builder::basic::make_document(
            bsoncxx::builder::basic::kvp("$set",
                bsoncxx::builder::basic::make_document(
                    bsoncxx::builder::basic::kvp(field, value)
                )
            )
        );
    }
};

struct AddToSetFilter: public Filter{
    AddToSetFilter(std::string&& field, bsoncxx::types::bson_value::value&& value) { 
        bson_filter = bsoncxx::builder::basic::make_document(
            bsoncxx::builder::basic::kvp("$addToSet",
                bsoncxx::builder::basic::make_document(
                    bsoncxx::builder::basic::kvp(field, value)
                )
            )
        );
    }
};

struct RemoveFromSetFilter: public Filter{
    RemoveFromSetFilter(std::string&& field, bsoncxx::types::bson_value::value&& value) { 
        bson_filter = bsoncxx::builder::basic::make_document(
            bsoncxx::builder::basic::kvp("$pull",
                bsoncxx::builder::basic::make_document(
                    bsoncxx::builder::basic::kvp(field, value)
                )
            )
        );
    }
};

struct GreaterThanFilter: public Filter{
    GreaterThanFilter(std::string&& field, bsoncxx::types::bson_value::value&& value) { 
        bson_filter = bsoncxx::builder::basic::make_document(
            bsoncxx::builder::basic::kvp(field,
                bsoncxx::builder::basic::make_document(
                    bsoncxx::builder::basic::kvp("$gt", value)
                )
            )
        );
    }
};

#endif