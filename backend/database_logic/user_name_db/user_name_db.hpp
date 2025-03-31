#ifndef __USER_NAME_DB__
#define __USER_NAME_DB__

#include <cstdint>
#include <istream>
#include <memory>
#include <optional>
#include <string>

class UserNameDB{
public:
    UserNameDB();

    std::optional<std::string> SetUserName(int64_t user_id, const std::string& user_name);
    std::optional<std::string> GetUserName(int64_t user_id);

private:
    std::shared_ptr<std::iostream> stream;
};

#endif