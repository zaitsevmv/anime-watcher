#ifndef __BASIC_FILTERS_H__
#define __BASIC_FILTERS_H__

#include <bsoncxx/document/view.hpp>
#include <bsoncxx/types/bson_value/value.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/oid.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <mongocxx/collection.hpp>
#include <mongocxx/client.hpp>

struct Filter{
    Filter() : bson_filter(bsoncxx::builder::basic::make_document()) {}
    auto get() const {
        return bson_filter.view();
    }
protected:
    bsoncxx::document::value bson_filter;
};

struct SearchFilter: public Filter{   
    SearchFilter(std::string field, bsoncxx::types::bson_value::value value) {
        if(field == "_id"){
            try{
                bsoncxx::oid value_oid{value.view().get_string()};
                bson_filter = bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp(field, value_oid));
            } catch(...){
                bson_filter = bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp(field, value));
            }
        } else{
            bson_filter = bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp(field, value));
        }
    }
};

struct SetFieldFilter: public Filter{
    SetFieldFilter(std::string field, bsoncxx::types::bson_value::value value) { 
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
    AddToSetFilter(std::string field, bsoncxx::types::bson_value::value value) { 
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
    RemoveFromSetFilter(std::string field, bsoncxx::types::bson_value::value value) { 
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
    GreaterThanFilter(std::string field, bsoncxx::types::bson_value::value value) { 
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