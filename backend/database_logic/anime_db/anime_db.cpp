#include "animedb.hpp"

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/json.hpp>

std::optional<std::string> AnimeDB::GetAnime(const std::string& anime_hash) {
    return GetDocument({"hashed_name", anime_hash});
}

std::optional<uint64_t> AnimeDB::AddAnime(const std::string& anime_data_json) {
    return AddDocument(anime_data_json);
}

std::optional<int> AnimeDB::DeleteAnime(const std::string& anime_hash) {
    return DeleteDocument({"hashed_name", anime_hash});
}

std::optional<std::string> AnimeDB::UpdateAnime(const std::string& anime_hash, const std::string& anime_data_json) {
    return UpdateDocument({"hashed_name", anime_hash}, anime_data_json);
}

