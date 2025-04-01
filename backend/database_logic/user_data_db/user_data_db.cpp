#include "user_data_db.hpp"
#include "mongodb_filters.hpp"

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/json.hpp>
#include <optional>

std::optional<std::string> UserDataDB::GetUser(int64_t user_id) {
    return GetDocument({"_id", user_id});
}

std::optional<int32_t> UserDataDB::AddUser(const std::string& user_data_json) {
    return AddDocument(user_data_json);
}

std::optional<int32_t> UserDataDB::DeleteUser(int64_t user_id) {
    return DeleteDocument({"_id", user_id});
}

std::optional<int32_t> UserDataDB::ChangeUserPassword(int64_t user_id, const std::string& new_password_hash) {
    auto result = collection.update_one(
        SearchFilter("_id", user_id).get(), 
        SetFieldFilter("password_hash", new_password_hash).get()
    );
    if(result){
        return (*result).modified_count();
    }
    return std::nullopt;
}

std::optional<int32_t> UserDataDB::ChangeUserLogin(int64_t user_id,  const std::string& new_user_login) {
    auto result = collection.update_one(
        SearchFilter("_id", user_id).get(), 
        SetFieldFilter("login", new_user_login).get()
    );
    if(result){
        return (*result).modified_count();
    }
    return std::nullopt;
}

std::optional<int32_t> UserDataDB::AddUserFavourite(int64_t user_id, const std::string& anime_hash) {
    auto result = collection.update_one(
        SearchFilter("_id", user_id).get(), 
        AddToSetFilter("favourite_anime", anime_hash).get()
    );
    if(result){
        return (*result).modified_count();
    }
    return std::nullopt;
}

std::optional<int32_t> UserDataDB::RemoveUserFavourite(int64_t user_id, const std::string& anime_hash) {
    auto result = collection.update_one(
        SearchFilter("_id", user_id).get(), 
        RemoveFromSetFilter("favourite_anime", anime_hash).get()
    );
    if(result){
        return (*result).modified_count();
    }
    return std::nullopt;
}
std::optional<bool> UserDataDB::LoginUnique(const std::string& user_login) {
    auto search_result = GetDocument(SearchFilter("login", user_login));
    return !search_result.has_value();
}

std::optional<bool> UserDataDB::UserExist(const std::string& user_login, const std::string& user_password_hash) {
    auto search_result = GetDocument(SearchFilter("login", user_login));
    if(!search_result.has_value()) return std::nullopt;
    auto bson_document = bsoncxx::from_json(*search_result);
    auto found_password_hash = bson_document["password_hash"];
    return (found_password_hash.get_string().value == user_password_hash);
}
