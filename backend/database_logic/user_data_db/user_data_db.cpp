#include "user_data_db.hpp"
#include "mongodb_filters.hpp"

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <cstdint>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/json.hpp>
#include <optional>

std::optional<std::string> UserDataDB::GetUser(const std::string& user_id) {
    return GetDocument(SearchFilter("_id", user_id));
}

std::optional<std::string> UserDataDB::AddUser(const std::string& user_data_json) {
    return AddDocument(user_data_json);
}

std::optional<int32_t> UserDataDB::FlagBanUser(const std::string& user_id, bool flag) {
    auto result = collection.update_one(
        SearchFilter("_id", user_id).get(), 
        SetFieldFilter("banned", flag).get()
    );
    if(result){
        return (*result).modified_count();
    }
    return std::nullopt;
}

std::optional<int32_t> UserDataDB::DeleteUser(const std::string& user_id) {
    return DeleteDocument({"_id", user_id});
}

std::optional<int32_t> UserDataDB::ChangeUserPassword(const std::string& user_id, const std::string& new_password_hash) {
    auto result = collection.update_one(
        SearchFilter("_id", user_id).get(), 
        SetFieldFilter("password_hash", new_password_hash).get()
    );
    if(result){
        return (*result).modified_count();
    }
    return std::nullopt;
}

std::optional<int32_t> UserDataDB::ChangeUserLogin(const std::string& user_id,  const std::string& new_user_login) {
    auto result = collection.update_one(
        SearchFilter("_id", user_id).get(), 
        SetFieldFilter("login", new_user_login).get()
    );
    if(result){
        return (*result).modified_count();
    }
    return std::nullopt;
}

std::optional<int32_t> UserDataDB::ChangeUserName(const std::string& user_id,  const std::string& new_user_name) {
    auto result = collection.update_one(
        SearchFilter("_id", user_id).get(), 
        SetFieldFilter("name", new_user_name).get()
    );
    if(result){
        return (*result).modified_count();
    }
    return std::nullopt;
}

std::optional<int32_t> UserDataDB::ChangeLastVideo(const std::string& user_id,  const std::string& last_video_json) {
    auto result = collection.update_one(
        SearchFilter("_id", user_id).get(), 
        SetFieldFilter("last_video", bsoncxx::from_json(last_video_json).view()).get()
    );
    if(result){
        return (*result).modified_count();
    }
    return std::nullopt;
}

std::optional<int32_t> UserDataDB::AddVideoToHistory(const std::string& user_id, const std::string& anime_id, const std::string& last_video_json) {
    auto result = collection.update_one(
        SearchFilter("_id", user_id).get(), 
        SetFieldFilter(std::string("history." + anime_id), bsoncxx::from_json(last_video_json).view()).get()
    );
    if(result){
        return (*result).modified_count();
    }
    return std::nullopt;
}

std::optional<int32_t> UserDataDB::AddUserFavourite(const std::string& user_id, const std::string& anime_id) {
    auto result = collection.update_one(
        SearchFilter("_id", user_id).get(), 
        AddToSetFilter("favourite_anime", anime_id).get()
    );
    if(result){
        return (*result).modified_count();
    }
    return std::nullopt;
}

std::optional<int32_t> UserDataDB::RemoveUserFavourite(const std::string& user_id, const std::string& anime_id) {
    auto result = collection.update_one(
        SearchFilter("_id", user_id).get(), 
        RemoveFromSetFilter("favourite_anime", anime_id).get()
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

std::optional<bool> UserDataDB::EmailUnique(const std::string& user_email) {
    auto search_result = GetDocument(SearchFilter("email", user_email));
    return !search_result.has_value();
}

std::optional<std::pair<std::string, std::string>> UserDataDB::GetPasswordAndId(const std::string& user_login) {
    auto search_result = GetDocument(SearchFilter("login", user_login));
    if(!search_result.has_value()) return std::nullopt;
    auto bson_document = bsoncxx::from_json(*search_result);
    auto found_password_hash = bson_document["password_hash"].get_string();
    std::string user_id = bson_document["_id"].get_oid().value.to_string();
    return std::make_pair(std::string(found_password_hash.value), user_id);
}

std::optional<std::string> UserDataDB::UserExist(const std::string& user_login, const std::string& user_password_hash) {
    auto search_result = GetDocument(SearchFilter("login", user_login));
    if(!search_result.has_value()) return std::nullopt;
    auto bson_document = bsoncxx::from_json(*search_result);
    auto found_password_hash = bson_document["password_hash"];
    if(found_password_hash.get_string().value == user_password_hash){
        std::string user_id = bson_document["_id"].get_oid().value.to_string();
        return user_id;
    }
    return std::nullopt;
}
