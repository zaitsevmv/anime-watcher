#ifndef __SEARCH_DB__
#define __SEARCH_DB__

#include <algorithm>
#include <curl/curl.h>

#include <cstdint>
#include <curl/easy.h>
#include <memory>
#include <optional>
#include <string>


class AnimeSearchDB{
public:
    AnimeSearchDB(const std::string& name);

    std::optional<int32_t> AddAnime(const std::string& anime_data_json);

    std::optional<int32_t> DeleteAnime(int64_t anime_id);

    std::optional<std::string> SearchAnime(const std::string& search_request);

    std::optional<int32_t> UpdateAnime(int64_t anime_id, const std::string& anime_data_json);
private:
    struct CURL_deleter{
        void operator()(CURL* pCURL) const {
            curl_easy_cleanup(pCURL);
            curl_global_cleanup();
        }
    };
    std::unique_ptr<CURL, CURL_deleter> curl;

    std::string db_name;
    std::string anime_name_field = "anime_name";
};

#endif