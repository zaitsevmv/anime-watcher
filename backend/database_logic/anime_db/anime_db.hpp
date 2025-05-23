#ifndef __ANIME_DB_H__
#define __ANIME_DB_H__

#include "base_mongodb.hpp"

#include <mongocxx/collection.hpp>
#include <mongocxx/client.hpp>

#include <optional>
#include <string>

class AnimeDB: public BaseMongoDB{
public:
    AnimeDB(const std::string& db_name, const std::string& collection_name)
        : BaseMongoDB(db_name, collection_name) {}

    std::optional<std::string> GetAnime(const std::string& anime_id);

    std::optional<std::string> AddAnime(const std::string& anime_data_json);

    std::optional<int32_t> DeleteAnime(const std::string& anime_id);

    std::optional<std::string> UpdateAnime(const std::string& anime_id, const std::string& anime_data_json);

    std::optional<int32_t> AddAnimeVideo(const std::string& anime_id, const std::string& video_id);

    std::optional<int32_t> RemoveAnimeVideo(const std::string& anime_id, const std::string& video_id);
};

#endif