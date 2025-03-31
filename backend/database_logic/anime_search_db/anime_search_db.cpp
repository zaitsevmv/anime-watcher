#include "anime_search_db.hpp"

#include <curl/curl.h>
#include <curl/easy.h>
#include <memory>


AnimeSearchDB::AnimeSearchDB()
    :curl(curl_easy_init()) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t newLength = size * nmemb;
    try {
        s->append((char*)contents, newLength);
    } catch (std::bad_alloc& e) {
        return 0;
    }
    return newLength;
}

std::optional<int32_t> AnimeSearchDB::AddAnime(const std::string& anime_hash, const std::string& anime_data_json) {
    std::string url = "http://localhost:9200/" + db_name + "/_doc/" + anime_hash;
    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, WriteCallback);
    std::string curl_buffer;
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &curl_buffer);

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, anime_data_json.c_str());

    auto result = curl_easy_perform(curl.get());
    curl_easy_cleanup(curl.get());
    if(result != CURLE_OK){
        return std::nullopt;
    }
    return 1;
}

std::optional<int32_t> AnimeSearchDB::DeleteAnime(const std::string& anime_hash) {
    std::string url = "http://localhost:9200/" + db_name + "/_doc/" + anime_hash;
    curl_easy_setopt(curl.get(), CURLOPT_CUSTOMREQUEST, "DELETE");
    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, WriteCallback);
    std::string curl_buffer;
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &curl_buffer);

    auto result = curl_easy_perform(curl.get());
    curl_easy_cleanup(curl.get());
    if(result != CURLE_OK) {
        return std::nullopt;
    }
    return 1;
}

std::optional<std::string> AnimeSearchDB::SearchAnime(const std::string& search_request) {
    std::string url = "http://localhost:9200/" + db_name + "/_search?q=" + anime_name_field + ":" + search_request;
    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, WriteCallback);
    std::string curl_buffer;
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &curl_buffer);

    auto result = curl_easy_perform(curl.get());
    curl_easy_cleanup(curl.get());
    if(result != CURLE_OK) {
        return std::nullopt;
    }
    return curl_buffer;
}

std::optional<int32_t> AnimeSearchDB::UpdateAnime(const std::string& anime_hash, const std::string& anime_data_json) {
    std::string url = "http://localhost:9200/" + db_name + "/_update/" + anime_hash;
    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, WriteCallback);
    std::string curl_buffer;
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &curl_buffer);

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, anime_data_json.c_str());

    auto result = curl_easy_perform(curl.get());
    curl_easy_cleanup(curl.get());
    if(result != CURLE_OK){
        return std::nullopt;
    }
    return 1;
}