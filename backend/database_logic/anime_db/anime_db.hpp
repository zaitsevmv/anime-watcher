#ifndef __ANIME_DB_H__
#define __ANIME_DB_H__

#include "base_mongodb.hpp"

#include <mongocxx/collection.hpp>
#include <mongocxx/client.hpp>

#include <optional>
#include <string>

class AnimeDB: private BaseMongoDB{
public:
    std::optional<std::string> GetAnime(const std::string& anime_hash);

    std::optional<uint64_t> AddAnime(const std::string& anime_data_json);

    std::optional<int> DeleteAnime(const std::string& anime_hash);

    std::optional<std::string> UpdateAnime(const std::string& anime_hash, const std::string& anime_data_json);
};

#endif