#include "server.hpp"

#include <boost/json/array.hpp>
#include <boost/json/object.hpp>
#include <boost/json/serialize.hpp>
#include <cstdint>
#include <memory>
#include <optional>
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
    msg = 0x0700,
    name = 0x0800,

    after = 0x010000,
    query = 0x020000

};

template<typename E>
constexpr auto toUType(E e) noexcept {
    return static_cast<std::underlying_type_t<E>>(e);
}

struct EndpointStruct{
    uint32_t target;
    std::optional<std::string> data;
};

EndpointStruct convert_endpoint(const std::string_view& endpoint){
    EndpointStruct result;
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
        if(tr == "auth") 
            encoded += toUType(req_targets::auth);
        else if(tr == "anime") 
            encoded += toUType(req_targets::anime);
        else if(tr == "search") 
            encoded += toUType(req_targets::search);
        else if(tr == "chat") 
            encoded += toUType(req_targets::chat);
        else if(tr == "register") 
            encoded += toUType(req_targets::reg);
        else if(tr == "login") 
            encoded += toUType(req_targets::login);
        else if(tr == "register") 
            encoded += toUType(req_targets::reg);
        else if(tr == "send") 
            encoded += toUType(req_targets::send); 
        else if(tr == "messages") 
            encoded += toUType(req_targets::msg); 
    }
    if(targets.back().find('?') < targets.back().size()){
        auto lhs = targets.back().substr(0, targets.back().find('?'));
        auto rhs = targets.back().substr(lhs.size()+1u);
        auto com = rhs.substr(0, rhs.find('='));
        auto val = rhs.substr(com.size()+1u);

        if(lhs == "messages")
            encoded += toUType(req_targets::msg);
        else if(lhs == "search")
            encoded += toUType(req_targets::search);
        else if(lhs == "users")
            encoded += toUType(req_targets::users);

        if(com == "after")
            encoded += toUType(req_targets::after);
        else if(com == "query")
            encoded += toUType(req_targets::query);
        else if(com == "name")
            encoded += toUType(req_targets::name);

        result.data.emplace(val);
    }
    result.target = encoded;
    return result;
}

//User data database

struct UserDataForm{
    std::string login;
    std::string name;
    std::string email;
    std::string password_hash;
};

std::optional<std::string> login_user(std::shared_ptr<UserDataDB> db, const UserDataForm& form){
    return db->UserExist(form.login, form.password_hash);
}

std::optional<std::string> register_user(std::shared_ptr<UserDataDB> db, const UserDataForm& form){
    auto login_unique = db->LoginUnique(form.login);
    if(!login_unique.has_value() || !(*login_unique)){
        return std::nullopt;
    }
    auto email_unique = db->EmailUnique(form.email);
    if(!email_unique.has_value() || !(*email_unique)){
        return std::nullopt;
    }
    boost::json::object new_user_data;
    new_user_data["login"] = form.login;
    new_user_data["name"] = form.login;
    new_user_data["email"] = form.email;
    new_user_data["password_hash"] = form.password_hash;
    new_user_data["favourite_anime"] = boost::json::array();
    auto add_result = db->AddUser(boost::json::serialize(new_user_data));
    return add_result;
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
    return result.has_value();
}

bool delete_anime(std::shared_ptr<AnimeDB> db, const std::string& anime_id){
    auto result = db->DeleteAnime(anime_id);
    return (result.has_value() && (*result == 1));
}

auto get_anime(std::shared_ptr<AnimeDB> db, const std::string& anime_id){
    return db->GetAnime(anime_id);
}

auto update_anime(std::shared_ptr<AnimeDB> db, const std::string& anime_id,  const std::string& anime_data_json){
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
    return result.has_value();
}

bool delete_message(std::shared_ptr<ChatDB> db, const int64_t message_id){
    auto result = db->DeleteMessage(message_id);
    return (result.has_value() && (*result == 1));
}

auto get_messages(std::shared_ptr<ChatDB> db, const int64_t last_update_ms){
    return db->GetNewMessages(last_update_ms);
}

//

void http_worker::process_get_request(const http::request<request_body_t, http::basic_fields<alloc_t>>& req) {
    std::cout << "Target: " << req.base().target() << std::endl;
    auto endpoint = convert_endpoint(req.base().target());
    std::string body = req.body();
    switch (endpoint.target) {
        case (toUType(req_targets::users) | toUType(req_targets::name)):
            {
                if(endpoint.data.has_value()){
                    auto res = get_redis_user_name(
                        user_name_db_, 
                        stoll(*endpoint.data)
                    );
                    boost::json::object response_object;
                    response_object["success"] = res.has_value();
                    if(res){
                        boost::json::array messages_json(boost::json::make_shared_resource<boost::json::monotonic_resource>());
                        for(const auto& s: *res){
                            messages_json.emplace_back(s);
                        }
                        response_object["messages"] = messages_json;
                    }
                    send_json_response(http::status::ok, boost::json::serialize(response_object));
                } else{
                    send_text_response(http::status::bad_request, "Bad request");
                }
            }
            break;
        case (toUType(req_targets::chat) | toUType(req_targets::msg) | toUType(req_targets::after)):
            {
                if(endpoint.data.has_value()){
                    if(*endpoint.data == "undefined"){
                        send_text_response(http::status::bad_request, "Bad request");
                        break;
                    }
                    auto res = get_messages(
                        chat_db_, 
                        stoll(*endpoint.data)
                    );
                    boost::json::object response_object;
                    response_object["success"] = res.has_value();
                    if(res){
                        boost::json::array messages_json(boost::json::make_shared_resource<boost::json::monotonic_resource>());
                        for(const auto& s: *res){
                            messages_json.emplace_back(s);
                        }
                        response_object["messages"] = messages_json;
                    }
                    send_json_response(http::status::ok, boost::json::serialize(response_object));
                } else{
                    send_text_response(http::status::bad_request, "Bad request");
                }
            }
            break;
        case (toUType(req_targets::anime) | toUType(req_targets::search) | toUType(req_targets::query)):
            {
                std::cout << "here" << std::endl;
                if(endpoint.data.has_value()){
                    auto res = search_indexed_anime(
                        anime_search_db_, 
                        *endpoint.data
                    );
                    std::cout << "Anime search result: " << res.has_value() << std::endl;
                    boost::json::object response_object;
                    response_object["success"] = res.has_value();
                    if(res){
                        boost::json::array anime_ids_json(boost::json::make_shared_resource<boost::json::monotonic_resource>());
                        auto jv = boost::json::parse(*res).as_array();
                        for(const auto& anim: jv){
                            auto src = anim.as_object().at("_source");
                            auto anim_id = boost::json::serialize(src.as_object().at("anime_id"));
                            anime_ids_json.emplace_back(anim_id);
                        }
                        response_object["anime_ids"] = anime_ids_json;
                    }
                    std::cout << response_object << std::endl;
                    send_json_response(http::status::ok, boost::json::serialize(response_object));
                } else{
                    send_text_response(http::status::bad_request, "Bad request");
                }
            }
            break;
        default:
            break;
    }
}

void http_worker::process_post_request(const http::request<request_body_t, http::basic_fields<alloc_t>>& req) {
    std::cout << "Got POST request: " << req.base() << std::endl << req.body() << std::endl; 
    std::cout << "Target: " << req.base().target() << std::endl;
    auto endpoint = convert_endpoint(req.base().target());
    std::string body = req.body();
    switch (endpoint.target) {
        case (toUType(req_targets::auth) | toUType(req_targets::login)):
            {
                auto jv = boost::json::parse(body);
                if(jv.as_object().contains("login") && jv.as_object().contains("password_hash")){
                    UserDataForm login_form;
                    login_form.login = jv.at("login").as_string().c_str();
                    login_form.password_hash = jv.at("password_hash").as_string().c_str();

                    auto res = login_user(
                        user_data_db_, 
                        login_form
                    );
                    std::cout << "Login result: " << res.has_value() << std::endl;
                    boost::json::object response_object;
                    response_object["success"] = res.has_value();
                    if(res){
                        response_object["user_id"] = *res;
                    }
                    send_json_response(http::status::ok, boost::json::serialize(response_object));
                } else{
                    send_text_response(http::status::bad_request, "Bad request");
                }
            }
            break;
        case (toUType(req_targets::auth) | toUType(req_targets::reg)):
            {
                auto jv = boost::json::parse(body);
                if(jv.as_object().contains("login") && jv.as_object().contains("password_hash") && jv.as_object().contains("email")){
                    UserDataForm reg_form;
                    reg_form.login = jv.at("login").as_string().c_str();
                    reg_form.password_hash = jv.at("password_hash").as_string().c_str();
                    reg_form.email = jv.at("email").as_string().c_str();
                    auto res = register_user(
                        user_data_db_, 
                        reg_form
                    );
                    std::cout << "Register result: " << res.has_value() << std::endl;
                    boost::json::object response_object;
                    response_object["success"] = res.has_value();
                    if(res){
                        response_object["user_id"] = *res;
                    }
                    send_json_response(http::status::ok, boost::json::serialize(response_object));
                } else{
                    send_text_response(http::status::bad_request, "Bad request");
                }
            }
            break;
        case (toUType(req_targets::chat) | toUType(req_targets::send)):
            {
                auto jv = boost::json::parse(body);
                if(jv.as_object().contains("user_id") 
                    && jv.as_object().contains("message") 
                    && jv.as_object().contains("timestamp_ms"))
                {
                    auto res = add_message(
                        chat_db_, 
                        body
                    );
                    std::cout << "Message sending result: " << res << std::endl;
                    boost::json::object response_object;
                    response_object["success"] = res;
                    send_json_response(http::status::ok, boost::json::serialize(response_object));
                } else{
                    send_text_response(http::status::bad_request, "Bad request");
                }
            }
            break;
        default:
            break;
    }
}