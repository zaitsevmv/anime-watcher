#include "anime_db.hpp"

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/json.hpp>

std::optional<std::string> AnimeDB::GetAnime(const std::string& anime_id) {
    return GetDocument({"_id", anime_id});
}

std::optional<std::string> AnimeDB::AddAnime(const std::string& anime_data_json) {
    return AddDocument(anime_data_json);
}

std::optional<int32_t> AnimeDB::DeleteAnime(const std::string& anime_id) {
    return DeleteDocument({"_id", anime_id});
}

std::optional<std::string> AnimeDB::UpdateAnime(const std::string& anime_id, const std::string& anime_data_json) {
    return UpdateDocument({"_id", anime_id}, anime_data_json);
}

std::optional<int32_t> AnimeDB::AddAnimeVideo(const std::string& anime_id, const std::string& video_id) {
    auto result = collection.update_one(
        SearchFilter("_id", anime_id).get(), 
        AddToSetFilter("videos", video_id).get()
    );
    if(result){
        return (*result).modified_count();
    }
    return std::nullopt;
}

std::optional<int32_t> AnimeDB::RemoveAnimeVideo(const std::string& anime_id, const std::string& video_id) {
    auto result = collection.update_one(
        SearchFilter("_id", anime_id).get(), 
        RemoveFromSetFilter("videos", video_id).get()
    );
    if(result){
        return (*result).modified_count();
    }
    return std::nullopt;
}

