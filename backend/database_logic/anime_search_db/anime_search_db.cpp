#include "anime_search_db.hpp"

#include <boost/json.hpp>
#include <boost/json/serialize.hpp>
#include <curl/curl.h>
#include <curl/easy.h>
#include <memory>
#include <iostream>

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t newLength = size * nmemb;
    try {
        s->append((char*)contents, newLength);
    } catch (std::bad_alloc& e) {
        return 0;
    }
    return newLength;
}

AnimeSearchDB::AnimeSearchDB(const std::string& name)
    :curl(curl_easy_init()),
     db_name(name) 
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
    std::string url = "localhost:9200/" + db_name + "?pretty";
    curl_easy_setopt(curl.get(),CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, WriteCallback);
    std::string curl_buffer;
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &curl_buffer);
    curl_easy_setopt(curl.get(), CURLOPT_CUSTOMREQUEST, "PUT");
    
    auto result = curl_easy_perform(curl.get());
    auto jv = boost::json::parse(curl_buffer);
    if(jv.as_object().contains("error")) {
        if(boost::json::serialize(
            jv.as_object().at("error")
            .as_object().at("root_cause").as_array().front()
            .as_object().at("type")) != "\"resource_already_exists_exception\"")
        {
            std::cout << "Elastic error" << std::endl;
        } else{
            std::cout << "Elastic exists" << std::endl;
        }
    }
    
}

std::optional<int32_t> AnimeSearchDB::AddAnime(const std::string& anime_data_json) {
    std::string url = "http://localhost:9200/" + db_name + "/_doc";
    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, WriteCallback);
    std::string curl_buffer;
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &curl_buffer);

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, anime_data_json.c_str());

    auto result = curl_easy_perform(curl.get());
    if(result != CURLE_OK){
        return std::nullopt;
    }
    auto jv = boost::json::parse(curl_buffer);
    if(jv.as_object().contains("error")){
        return std::nullopt;
    }
    return 1;
}

std::optional<int32_t> AnimeSearchDB::DeleteAnime(const std::string& anime_id) {
    std::string url = "http://localhost:9200/" + db_name + "/_doc/" + anime_id;
    curl_easy_setopt(curl.get(), CURLOPT_CUSTOMREQUEST, "DELETE");
    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, WriteCallback);
    std::string curl_buffer;
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &curl_buffer);
    curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, "");

    auto result = curl_easy_perform(curl.get());
    if(result != CURLE_OK) {
        return std::nullopt;
    }
    std::cout << curl_buffer << std::endl;
    auto jv = boost::json::parse(curl_buffer);
    if(jv.as_object().contains("error")){
        return std::nullopt;
    }
    return 1;
}

std::optional<std::string> AnimeSearchDB::SearchAnime(const std::string& search_request) {
    std::string url = "http://localhost:9200/" + db_name + "/_search?pretty";
    std::string json_data = "{\"query\":{\"match_phrase_prefix\":{\"title\":\"" + search_request + "\"}}, \"size\": 20}";

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, WriteCallback);
    std::string curl_buffer;
    curl_easy_setopt(curl.get(), CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, json_data.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &curl_buffer);

    auto result = curl_easy_perform(curl.get());
    if(result != CURLE_OK) {
        return std::nullopt;
    }
    auto jv = boost::json::parse(curl_buffer);
    if(jv.as_object().contains("error")){
        return std::nullopt;
    }
    auto hits_array_json = jv.as_object().at("hits").as_object().at("hits");
    std::cout << hits_array_json << std::endl;
    return boost::json::serialize(hits_array_json);
}

std::optional<std::string> AnimeSearchDB::SearchAnimeId(const std::string& search_request) {
    std::string url = "http://localhost:9200/" + db_name + "/_search?pretty";
    std::string json_data = "{\"query\":{\"match\":{\"anime_id\":\"" + search_request + "\"}}}";

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, WriteCallback);
    std::string curl_buffer;
    curl_easy_setopt(curl.get(), CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, json_data.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &curl_buffer);

    auto result = curl_easy_perform(curl.get());
    if(result != CURLE_OK) {
        return std::nullopt;
    }
    auto jv = boost::json::parse(curl_buffer);
    if(jv.as_object().contains("error")){
        return std::nullopt;
    }
    auto hits_array_json = jv.as_object().at("hits").as_object().at("hits");
    std::cout << hits_array_json << std::endl;
    return boost::json::serialize(hits_array_json);
}

std::optional<std::string> AnimeSearchDB::GetAllAnime() {
    std::string url = "http://localhost:9200/" + db_name + "/_search?pretty=true&q=*:*&size=10000";

    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, WriteCallback);
    std::string curl_buffer;
    curl_easy_setopt(curl.get(), CURLOPT_CUSTOMREQUEST, "GET");
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &curl_buffer);
    curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, "");

    auto result = curl_easy_perform(curl.get());
    std::cout << curl_buffer << std::endl;
    if (result != CURLE_OK) {
        return std::nullopt;
    }
    auto jv = boost::json::parse(curl_buffer);
    if (jv.as_object().contains("error")) {
        return std::nullopt;
    }
    auto hits_array_json = jv.as_object().at("hits").as_object().at("hits");
    std::cout << hits_array_json << std::endl;
    return boost::json::serialize(hits_array_json);
}

std::optional<int32_t> AnimeSearchDB::UpdateAnime(const std::string& anime_id, const std::string& anime_data_json) {
    std::string url = "http://localhost:9200/" + db_name + "/_update/" + anime_id;
    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, WriteCallback);
    std::string curl_buffer;
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &curl_buffer);

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, anime_data_json.c_str());

    auto result = curl_easy_perform(curl.get());
    if(result != CURLE_OK){
        return std::nullopt;
    }
    std::cout << curl_buffer << std::endl;
    auto jv = boost::json::parse(curl_buffer);
    if(jv.as_object().contains("error")){
        return std::nullopt;
    }
    return 1;
}