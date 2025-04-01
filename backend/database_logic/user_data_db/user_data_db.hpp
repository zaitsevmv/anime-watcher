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

    std::optional<std::string> GetUser(int64_t user_id);

    std::optional<int32_t> AddUser(const std::string& user_data_json);

    std::optional<int32_t> DeleteUser(int64_t user_id);

    std::optional<int32_t> ChangeUserPassword(int64_t user_id, const std::string& new_password_hash);

    std::optional<int32_t> ChangeUserLogin(int64_t user_id,  const std::string& new_user_login);

    std::optional<int32_t> AddUserFavourite(int64_t user_id, const std::string& anime_hash);

    std::optional<int32_t> RemoveUserFavourite(int64_t user_id, const std::string& anime_hash);

    std::optional<bool> LoginUnique(const std::string& user_login);

    std::optional<bool> UserExist(const std::string& user_login, const std::string& user_password_hash);
};

#endif