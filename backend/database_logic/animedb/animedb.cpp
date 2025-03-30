#include "animedb.hpp"

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/json.hpp>

std::optional<std::string> AnimeDB::GetAnime(const std::string& anime_name) {
    return GetDocument({"hashed_name", std::to_string(h(anime_name))});
}

std::optional<uint64_t> AnimeDB::AddAnime(const std::string& anime_data_json) {
    return AddDocument(anime_data_json);
}

std::optional<int> AnimeDB::DeleteAnime(const std::string& anime_name) {
    std::hash<std::string> h;
    return DeleteDocument({"hashed_name", std::to_string(h(anime_name))});
}

std::optional<std::string> AnimeDB::UpdateAnime(const std::string& anime_name, const std::string& anime_data_json) {
    return UpdateDocument({"hashed_name", std::to_string(h(anime_name))}, anime_data_json);
}

