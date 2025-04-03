#ifndef __USER_DATA_DB_H__
#define __USER_DATA_DB_H__

#include "base_mongodb.hpp"

#include <cstdint>
#include <mongocxx/collection.hpp>
#include <mongocxx/client.hpp>

#include <optional>
#include <string>

class UserDataDB: public BaseMongoDB{
public:
    UserDataDB(const std::string& db_name, const std::string& collection_name)
        : BaseMongoDB(db_name, collection_name) {}

    std::optional<std::string> GetUser(std::string user_id);

    std::optional<std::string> AddUser(const std::string& user_data_json);

    std::optional<int32_t> DeleteUser(std::string user_id);

    std::optional<int32_t> ChangeUserPassword(std::string user_id, const std::string& new_password_hash);

    std::optional<int32_t> ChangeUserLogin(std::string user_id,  const std::string& new_user_login);

    std::optional<int32_t> AddUserFavourite(std::string user_id, const std::string& anime_hash);

    std::optional<int32_t> RemoveUserFavourite(std::string user_id, const std::string& anime_hash);

    std::optional<bool> LoginUnique(const std::string& user_login);
    std::optional<bool> EmailUnique(const std::string& user_email);

    std::optional<std::string> UserExist(const std::string& user_login, const std::string& user_password_hash);
};

#endif