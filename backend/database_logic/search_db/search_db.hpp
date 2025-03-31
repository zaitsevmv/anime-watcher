#ifndef __SEARCH_DB__
#define __SEARCH_DB__

#include <curl/curl.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

class AnimeSearchDB{
public:
    ~AnimeSearchDB();

    void Start();

    std::optional<int32_t> AddAnime(const std::string& anime_hash, const std::string& anime_data_json);

    std::optional<int32_t> DeleteAnime(const std::string& anime_hash);

    std::optional<std::string> SearchAnime(const std::string& search_request);

    std::optional<int32_t> UpdateAnime(const std::string& anime_hash, const std::string& anime_data_json);
private:
    std::unique_ptr<CURL> curl;

    std::string db_name = "anime_names";
    std::string anime_name_field = "anime_name";
};

#endif