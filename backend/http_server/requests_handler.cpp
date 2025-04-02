#include "server.hpp"

#include <boost/json/array.hpp>
#include <boost/json/object.hpp>
#include <boost/json/serialize.hpp>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <vector>

enum class req_targets: uint32_t{
    auth = 0x01,
    chat = 0x02,
    anime = 0x03,
    users = 0x05,

    reg = 0x0100,
    login = 0x0200,
    fav = 0x0300,
    details = 0x0400,
    search = 0x0500,
    send = 0x0600,
    msg = 0x0700

};

template<typename E>
constexpr auto toUType(E e) noexcept {
    return static_cast<std::underlying_type_t<E>>(e);
}

uint32_t convert_endpoint(const std::string_view& endpoint){
    std::vector<std::string> targets;
    std::string cur_target;
    for(auto c: endpoint){
        if(c != '/'){
            cur_target += c;
        } else{
            targets.push_back(cur_target);
            cur_target.clear();
        }
    }
    if(!cur_target.empty()){
        targets.push_back(cur_target);
    }
    uint32_t encoded = 0;
    for(const auto& tr: targets){
        if(tr == "auth"){
            encoded += toUType(req_targets::auth);
        }
        else if(tr == "chat"){
            encoded += toUType(req_targets::chat);
        }
        if(tr == "register"){
            encoded += toUType(req_targets::reg);
        }
        else if(tr == "login"){
            encoded += toUType(req_targets::login);
        }
    }
    return encoded;
}

//User data database

bool login_user(std::shared_ptr<UserDataDB> db, const std::string& login, const std::string& password_hash){
    auto uq = db->UserExist(login, password_hash);
    return (uq.has_value() && *uq);
}

bool register_user(std::shared_ptr<UserDataDB> db, const std::string& login, const std::string& password_hash){
    auto login_unique = db->LoginUnique(login);
    if(!login_unique.has_value() || !(*login_unique)){
        return false;
    }
    boost::json::object new_user_data;
    new_user_data["login"] = login;
    new_user_data["name"] = login;
    new_user_data["password_hash"] = password_hash;
    new_user_data["favourite_anime"] = boost::json::array();
    auto add_result = db->AddUser(boost::json::serialize(new_user_data));
    return (add_result.has_value() && ((*add_result) == 1));
}

//User name database

auto get_redis_user_name(std::shared_ptr<UserNameDB> db, const int64_t user_id){
    return db->GetUserName(user_id);
}

auto set_redis_user_name(std::shared_ptr<UserNameDB> db, const int64_t user_id, const std::string& user_name){
    return db->SetUserName(user_id, user_name);
}

//Anime Database

bool add_anime(std::shared_ptr<AnimeDB> db, const std::string& anime_data_json){
    auto result = db->AddAnime(anime_data_json);
    return (result.has_value() && (*result == 1));
}

bool delete_anime(std::shared_ptr<AnimeDB> db, const int64_t anime_id){
    auto result = db->DeleteAnime(anime_id);
    return (result.has_value() && (*result == 1));
}

auto get_anime(std::shared_ptr<AnimeDB> db, const int64_t anime_id){
    return db->GetAnime(anime_id);
}

auto update_anime(std::shared_ptr<AnimeDB> db, const int64_t anime_id,  const std::string& anime_data_json){
    return db->UpdateAnime(anime_id, anime_data_json);
}

//Anime search Database

auto index_anime(std::shared_ptr<AnimeSearchDB> db, const int64_t anime_id, const std::string& anime_data_json){
    return db->AddAnime(anime_id, anime_data_json);
}

auto delete_indexed_anime(std::shared_ptr<AnimeSearchDB> db, const int64_t anime_id){
    return db->DeleteAnime(anime_id);
} 

auto update_indexed_anime(std::shared_ptr<AnimeSearchDB> db, const int64_t anime_id, const std::string& anime_data_json){
    return db->UpdateAnime(anime_id, anime_data_json);
} 

auto search_indexed_anime(std::shared_ptr<AnimeSearchDB> db, const std::string& search_request){
    return db->SearchAnime(search_request);
}

//Chat Database

bool add_message(std::shared_ptr<ChatDB> db, const std::string& message_data_json){
    auto result = db->AddMessage(message_data_json);
    return (result.has_value() && (*result == 1));
}

bool delete_message(std::shared_ptr<ChatDB> db, const int64_t message_id){
    auto result = db->DeleteMessage(message_id);
    return (result.has_value() && (*result == 1));
}

void http_worker::process_get_request(const http::request<request_body_t, http::basic_fields<alloc_t>>& req) {
    std::cout << "Got GET request: " << req.base() << std::endl << req.body() << std::endl; 
}

void http_worker::process_post_request(const http::request<request_body_t, http::basic_fields<alloc_t>>& req) {
    std::cout << "Got POST request: " << req.base() << std::endl << req.body() << std::endl; 
    std::cout << "Target: " << req.base().target() << std::endl;
    auto endpoint = convert_endpoint(req.base().target());
    std::string body = req.body();
    switch (endpoint) {
        case (toUType(req_targets::auth) | toUType(req_targets::login)):
            {
                auto jv = boost::json::parse(body);
                if(jv.as_object().contains("login") && jv.as_object().contains("password_hash")){
                    auto res = login_user(
                        user_data_db_, 
                        jv.at("login").as_string().c_str(), 
                        jv.at("password_hash").as_string().c_str()
                    );
                    std::cout << "Login result: " << res << std::endl;
                }
            }
            break;
        case (toUType(req_targets::auth) | toUType(req_targets::reg)):
            {
                auto jv = boost::json::parse(body);
                if(jv.as_object().contains("login") && jv.as_object().contains("password_hash")){
                    auto res = register_user(
                        user_data_db_, 
                        jv.at("login").as_string().c_str(), 
                        jv.at("password_hash").as_string().c_str()
                    );
                    std::cout << "Register result: " << res << std::endl;
                }
            }
            break;
        default:
            break;
    }
}