#ifndef __USER_DATA_DB_H__
#define __USER_DATA_DB_H__

#include "base_mongodb.hpp"

#include <mongocxx/collection.hpp>
#include <mongocxx/client.hpp>

#include <optional>
#include <string>

class UserDataDB: private BaseMongoDB{
public:
    std::optional<std::string> GetUser(uint64_t user_id);

    std::optional<uint64_t> AddUser(const std::string& user_data_json);

    std::optional<int32_t> DeleteUser(uint64_t user_id);

    std::optional<std::string> ChangeUserPassword(uint64_t user_id);

    std::optional<std::string> ChangeUserLogin(uint64_t user_id);

    std::optional<std::string> AddUserFavourite(uint64_t user_id, const std::string& anime_hash);

    std::optional<std::string> RemoveUserFavourite(uint64_t user_id, const std::string& anime_hash);
};

#endif