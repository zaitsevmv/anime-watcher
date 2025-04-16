#include "chat_db/chat_db.hpp"
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
    brief = 0x0800,
    name = 0x0900,
    last_video = 0x0a00,
    status = 0x0b00,
    update = 0x0c00,
    banned = 0x0d00,
    del = 0x0e00,
    ban = 0x0f00,
    video = 0x1000,

    after = 0x010000,
    query = 0x020000,
    anime_id = 0x030000,
    user_id = 0x040000,
    add = 0x050000,
    remove = 0x060000,
    set = 0x070000,
    all = 0x080000

};

template<typename E>
constexpr auto toUType(E e) noexcept {
    return static_cast<std::underlying_type_t<E>>(e);
}

std::string decode_url_string(const std::string& str) {
    std::ostringstream decoded;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '%' && i + 2 < str.length()) {
            int hexValue;
            std::istringstream iss(str.substr(i + 1, 2));
            if (iss >> std::hex >> hexValue)
                decoded << static_cast<char>(hexValue);
            else
                decoded << '%';
            i += 2;
        } else if (str[i] == '+') {
            decoded << ' ';
        } else {
            decoded << str[i];
        }
    }
    return decoded.str();
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
            encoded ^= toUType(req_targets::auth);
        else if(tr == "users") 
            encoded ^= toUType(req_targets::users);
        else if(tr == "anime") 
            encoded ^= toUType(req_targets::anime);
        else if(tr == "search") 
            encoded ^= toUType(req_targets::search);
        else if(tr == "banned") 
            encoded ^= toUType(req_targets::banned);
        else if(tr == "chat") 
            encoded ^= toUType(req_targets::chat);
        else if(tr == "video") 
            encoded ^= toUType(req_targets::video);
        else if(tr == "name") 
            encoded ^= toUType(req_targets::name);
        else if(tr == "register") 
            encoded ^= toUType(req_targets::reg);
        else if(tr == "login") 
            encoded ^= toUType(req_targets::login);
        else if(tr == "register") 
            encoded ^= toUType(req_targets::reg);
        else if(tr == "ban") 
            encoded ^= toUType(req_targets::ban);
        else if(tr == "delete") 
            encoded ^= toUType(req_targets::del);
        else if(tr == "favourites") 
            encoded ^= toUType(req_targets::fav); 
        else if(tr == "update") 
            encoded ^= toUType(req_targets::update); 
        else if(tr == "send") 
            encoded ^= toUType(req_targets::send); 
        else if(tr == "messages") 
            encoded ^= toUType(req_targets::msg); 
        else if(tr == "status") 
            encoded ^= toUType(req_targets::status); 
        else if(tr == "details")
            encoded ^= toUType(req_targets::details);
        else if(tr == "last_video")
            encoded ^= toUType(req_targets::last_video);
        else if(tr == "add")
            encoded ^= toUType(req_targets::add);
        else if(tr == "remove")
            encoded ^= toUType(req_targets::remove);
        else if(tr == "brief")
            encoded ^= toUType(req_targets::brief);
        else if (tr == "all")
            encoded ^= toUType(req_targets::all);
        else if(tr == "set") 
            encoded ^= toUType(req_targets::set);
    }
    if(targets.back().find('?') < targets.back().size()){
        auto lhs = targets.back().substr(0, targets.back().find('?'));
        auto rhs = targets.back().substr(lhs.size()+1u);
        auto com = rhs.substr(0, rhs.find('='));
        auto val = rhs.substr(com.size()+1u);

        if(lhs == "messages")
            encoded ^= toUType(req_targets::msg);
        else if(lhs == "search")
            encoded ^= toUType(req_targets::search);
        else if(lhs == "users")
            encoded ^= toUType(req_targets::users);
        else if(lhs == "details")
            encoded ^= toUType(req_targets::details);
        else if(lhs == "brief")
            encoded ^= toUType(req_targets::brief);
        else if(lhs == "favourites")
            encoded ^= toUType(req_targets::fav);
        else if(lhs == "name") 
            encoded ^= toUType(req_targets::name);
        else if(lhs == "last_video")
            encoded ^= toUType(req_targets::last_video);
        else if(lhs == "remove")
            encoded ^= toUType(req_targets::remove);
        else if(lhs == "status") 
            encoded ^= toUType(req_targets::status); 
        else if(lhs == "banned") 
            encoded ^= toUType(req_targets::banned);
        else if (lhs == "all")
            encoded ^= toUType(req_targets::all);

        if(com == "after")
            encoded ^= toUType(req_targets::after);
        else if(com == "query")
            encoded ^= toUType(req_targets::query);
        else if(com == "anime_id")
            encoded ^= toUType(req_targets::anime_id);
        else if(com == "user_id")
            encoded ^= toUType(req_targets::user_id);
        else if(com == "all")
            encoded ^= toUType(req_targets::all);

        auto decoded = decode_url_string(val);
        result.data.emplace(decoded);
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

auto user_password_and_id(std::shared_ptr<UserDataDB> db, const UserDataForm& form){
    return db->GetPasswordAndId(form.login);
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

std::optional<std::string> get_user(std::shared_ptr<UserDataDB> db, const std::string& user_id){
    std::cout << user_id << std::endl;
    return db->GetUser(user_id);
}

bool add_user_favourite(std::shared_ptr<UserDataDB> db, const std::string& user_id, const std::string& anime_id){
    auto res = db->AddUserFavourite(user_id, anime_id);
    return (res.has_value() && *res == 1);
}

bool remove_user_favourite(std::shared_ptr<UserDataDB> db, const std::string& user_id, const std::string& anime_id){
    auto res = db->RemoveUserFavourite(user_id, anime_id);
    return (res.has_value() && *res == 1);
}

bool set_ban_flag_user(std::shared_ptr<UserDataDB> user_db, std::shared_ptr<ChatDB> chat_db, const std::string& user_id, const bool flag){
    auto ban_res = user_db->FlagBanUser(user_id, flag);
    if(!ban_res.has_value()) return false;
    if(flag){
        auto res = chat_db->DeleteUserMessages(user_id);
        return res.has_value();
    }
    return true;
}

std::optional<std::string> get_user_favourites(std::shared_ptr<UserDataDB> db, const std::string& user_id){
    auto user_data = db->GetUser(user_id);
    if(!user_data.has_value()) return std::nullopt;
    auto jv = boost::json::parse(*user_data).as_object();
    if(!jv.contains("favourite_anime")) return std::nullopt;
    return boost::json::serialize(jv.at("favourite_anime"));
}

std::optional<std::string> get_user_last_video(std::shared_ptr<UserDataDB> db, const std::string& user_id){
    auto user_data = db->GetUser(user_id);
    if(!user_data.has_value()) return std::nullopt;
    auto jv = boost::json::parse(*user_data).as_object();
    if(!jv.contains("last_video")) return std::nullopt;
    return jv.at("last_video").as_string().c_str();
}

std::optional<std::string> get_user_name(std::shared_ptr<UserDataDB> db, const std::string& user_id){
    auto user_data = db->GetUser(user_id);
    if(!user_data.has_value()) return std::nullopt;
    auto jv = boost::json::parse(*user_data).as_object();
    if(!jv.contains("name")) return std::nullopt;
    return jv.at("name").as_string().c_str();
}

bool set_user_name(std::shared_ptr<UserDataDB> db, const std::string& user_id, const std::string& user_name){
    auto res = db->ChangeUserName(user_id, user_name);
    return (res.has_value() && *res == 1);
}

bool set_user_last_video(std::shared_ptr<UserDataDB> db, const std::string& user_id, const std::string& video_id){
    auto res = db->ChangeLastVideo(user_id, video_id);
    return (res.has_value() && *res == 1);
}

std::optional<bool> get_user_banned(std::shared_ptr<UserDataDB> db, const std::string& user_id){
    auto user_data = db->GetUser(user_id);
    if(!user_data.has_value()) return std::nullopt;
    auto jv = boost::json::parse(*user_data).as_object();
    if(!jv.contains("banned")) return false;
    return jv.at("banned").as_bool();
}

std::optional<int64_t> get_user_status(std::shared_ptr<UserDataDB> db, const std::string& user_id){
    auto user_data = db->GetUser(user_id);
    if(!user_data.has_value()) return std::nullopt;
    auto jv = boost::json::parse(*user_data).as_object();
    if(!jv.contains("status")) return std::nullopt;
    return jv.at("status").as_int64();
}

//Anime Database

auto add_anime(std::shared_ptr<AnimeDB> db, const std::string& anime_data_json){
    return db->AddAnime(anime_data_json);
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

bool add_anime_video(std::shared_ptr<AnimeDB> db, const std::string& anime_id, const std::string& video_id){
    auto res = db->AddAnimeVideo(anime_id, video_id);
    return (res.has_value() && *res == 1);
}

bool remove_anime_video(std::shared_ptr<AnimeDB> db, const std::string& anime_id, const std::string& video_id){
    auto res = db->RemoveAnimeVideo(anime_id, video_id);
    return (res.has_value() && *res == 1);
}

//Anime search Database

auto index_anime(std::shared_ptr<AnimeSearchDB> db, const std::string& anime_data_json){
    return db->AddAnime(anime_data_json);
}

auto delete_indexed_anime(std::shared_ptr<AnimeSearchDB> db, const std::string& anime_id){
    return db->DeleteAnime(anime_id);
} 

auto update_indexed_anime(std::shared_ptr<AnimeSearchDB> db, const std::string& anime_id, const std::string& anime_data_json){
    return db->UpdateAnime(anime_id, anime_data_json);
} 

auto search_indexed_anime(std::shared_ptr<AnimeSearchDB> db, const std::string& search_request){
    return db->SearchAnime(search_request);
}

auto get_all_indexed_anime(std::shared_ptr<AnimeSearchDB> db){
    return db->GetAllAnime();
}

std::optional<std::string> get_index_id(std::shared_ptr<AnimeSearchDB> db, const std::string& anime_id){
    auto result = db->SearchAnimeId(anime_id);
    if(result.has_value()){
        auto jv = boost::json::parse(*result).as_array().front();
        auto index_id = jv.as_object().at("_id").as_string();
        return index_id.c_str();
    }   
    return std::nullopt;
}


//Chat Database

bool add_message(std::shared_ptr<ChatDB> db, const std::string& message_data_json){
    auto result = db->AddMessage(message_data_json);
    return result.has_value();
}

bool delete_message(std::shared_ptr<ChatDB> db, const std::string& message_id){
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
        case (toUType(req_targets::users) | toUType(req_targets::details) | toUType(req_targets::user_id)):
            {
                if(endpoint.data.has_value()){
                    auto res = get_user(
                        user_data_db_, 
                        *endpoint.data
                    );
                    boost::json::object response_object;
                    response_object["success"] = res.has_value();
                    if(res){
                        response_object["user"] = *res;
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
                if(endpoint.data.has_value()){
                    auto res = search_indexed_anime(
                        anime_search_db_, 
                        *endpoint.data
                    );
                    boost::json::object response_object;
                    response_object["success"] = res.has_value();
                    if(res){
                        boost::json::array anime_ids_json(boost::json::make_shared_resource<boost::json::monotonic_resource>());
                        auto jv = boost::json::parse(*res).as_array();
                        for(const auto& anim: jv){
                            auto src = anim.as_object().at("_source");
                            auto anim_id = src.as_object().at("anime_id").as_string();
                            anime_ids_json.emplace_back(anim_id);
                        }
                        response_object["anime_ids"] = anime_ids_json;
                    }
                    send_json_response(http::status::ok, boost::json::serialize(response_object));
                } else{
                    send_text_response(http::status::bad_request, "Bad request");
                }
            }
            break;
        case (toUType(req_targets::anime) | toUType(req_targets::search) | toUType(req_targets::all)):
            {
                auto res = get_all_indexed_anime(
                    anime_search_db_
                );
                boost::json::object response_object;
                response_object["success"] = res.has_value();
                std::cout << "Get all anime result: " << res.has_value() << std::endl; 
                if(res){
                    boost::json::array anime_ids_json(boost::json::make_shared_resource<boost::json::monotonic_resource>());
                    auto jv = boost::json::parse(*res).as_array();
                    for(const auto& anim: jv){
                        auto src = anim.as_object().at("_source");
                        auto anim_id = src.as_object().at("anime_id").as_string();
                        anime_ids_json.emplace_back(anim_id);
                    }
                    response_object["anime_ids"] = anime_ids_json;
                }
                send_json_response(http::status::ok, boost::json::serialize(response_object));
            }
            break;
        case (toUType(req_targets::anime) | toUType(req_targets::details) | toUType(req_targets::anime_id)):
            {
                if(endpoint.data.has_value()){
                    auto res = get_anime(
                        anime_db_, 
                        *endpoint.data
                    );
                    boost::json::object response_object;
                    response_object["success"] = res.has_value();
                    if(res){
                        response_object["anime"] = *res;
                    }
                    send_json_response(http::status::ok, boost::json::serialize(response_object));
                } else{
                    send_text_response(http::status::bad_request, "Bad request");
                }
            }
            break;
        case (toUType(req_targets::users) | toUType(req_targets::fav) | toUType(req_targets::user_id)):
            {
                if(endpoint.data.has_value()){
                    auto res = get_user_favourites(
                        user_data_db_, 
                        *endpoint.data
                    );
                    std::cout << "Fav get result: " << res.has_value() << std::endl;
                    boost::json::object response_object;
                    response_object["success"] = res.has_value();
                    if(res){
                        response_object["anime_ids"] = *res;
                    }
                    send_json_response(http::status::ok, boost::json::serialize(response_object));
                } else{
                    send_text_response(http::status::bad_request, "Bad request");
                }
            }
            break;
        case (toUType(req_targets::users) | toUType(req_targets::last_video) | toUType(req_targets::user_id)):
            {
                if(endpoint.data.has_value()){
                    auto res = get_user_last_video(
                        user_data_db_, 
                        *endpoint.data
                    );
                    boost::json::object response_object;
                    response_object["success"] = res.has_value();
                    if(res){
                        response_object["last_video"] = *res;
                    }
                    send_json_response(http::status::ok, boost::json::serialize(response_object));
                } else{
                    send_text_response(http::status::bad_request, "Bad request");
                }
            }
            break;
        case (toUType(req_targets::users) | toUType(req_targets::status) | toUType(req_targets::user_id)):
            {
                if(endpoint.data.has_value()){
                    auto res = get_user_status(
                        user_data_db_, 
                        *endpoint.data
                    );
                    boost::json::object response_object;
                    response_object["success"] = res.has_value();
                    if(res){
                        response_object["status"] = *res;
                    }
                    send_json_response(http::status::ok, boost::json::serialize(response_object));
                } else{
                    send_text_response(http::status::bad_request, "Bad request");
                }
            }
            break;
        case (toUType(req_targets::users) | toUType(req_targets::name) | toUType(req_targets::user_id)):
            {
                if(endpoint.data.has_value()){
                    auto res = get_user_name(
                        user_data_db_, 
                        *endpoint.data
                    );
                    boost::json::object response_object;
                    response_object["success"] = res.has_value();
                    if(res){
                        response_object["name"] = *res;
                    }
                    std::cout << response_object << std::endl;
                    send_json_response(http::status::ok, boost::json::serialize(response_object));
                } else{
                    send_text_response(http::status::bad_request, "Bad request");
                }
            }
            break;
        case (toUType(req_targets::users) | toUType(req_targets::banned) | toUType(req_targets::user_id)):
            {
                if(endpoint.data.has_value()){
                    auto res = get_user_banned(
                        user_data_db_, 
                        *endpoint.data
                    );
                    boost::json::object response_object;
                    response_object["success"] = res.has_value();
                    if(res){
                        response_object["banned"] = *res;
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
                if(jv.as_object().contains("login")){
                    UserDataForm login_form;
                    login_form.login = jv.at("login").as_string().c_str();

                    auto res = user_password_and_id(
                        user_data_db_, 
                        login_form
                    );
                    boost::json::object response_object;
                    response_object["success"] = res.has_value();
                    if(res){
                        response_object["hashed_password"] = (*res).first;
                        response_object["user_id"] = (*res).second;
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
        case (toUType(req_targets::anime) | toUType(req_targets::add)):
            {
                auto jv = boost::json::parse(body);
                if(jv.as_object().contains("title") 
                    && jv.as_object().contains("description") 
                    && jv.as_object().contains("videos")
                    && jv.as_object().contains("episodes"))
                {
                    boost::json::object anime_data;
                    anime_data["title"] = jv.as_object().at("title"); 
                    anime_data["description"] = jv.as_object().at("description"); 
                    anime_data["videos"] = jv.as_object().at("videos"); 
                    anime_data["episodes"] = jv.as_object().at("episodes"); 
                    auto res = add_anime(
                        anime_db_, 
                        boost::json::serialize(anime_data)
                    );
                    std::cout << "Anime add result: " << res.has_value() << std::endl;
                    boost::json::object response_object;
                    if(res){
                        boost::json::object anime_brief;
                        anime_brief["title"] = jv.as_object().at("title");
                        anime_brief["anime_id"] = *res;
                        auto idx_result = index_anime(
                            anime_search_db_, 
                            boost::json::serialize(anime_brief)
                        );
                        response_object["success"] = idx_result.has_value();
                        response_object["db_id"] = *res;
                    } else{
                        response_object["success"] = false;
                    }
                    send_json_response(http::status::ok, boost::json::serialize(response_object));
                } else{
                    send_text_response(http::status::bad_request, "Bad request");
                }
            }
            break;
        case (toUType(req_targets::users) | toUType(req_targets::ban)):
            {
                auto jv = boost::json::parse(body);
                if(jv.as_object().contains("user_id") 
                    && jv.as_object().contains("flag"))
                {
                    auto res = set_ban_flag_user(
                        user_data_db_, 
                        chat_db_,
                        jv.at("user_id").as_string().c_str(),
                        jv.at("flag").as_bool()
                    );
                    std::cout << "User flag ban result: " << res << std::endl;
                    boost::json::object response_object;
                    response_object["success"] = res;
                    send_json_response(http::status::ok, boost::json::serialize(response_object));
                } else{
                    send_text_response(http::status::bad_request, "Bad request");
                }
            }
            break;
        case (toUType(req_targets::users) | toUType(req_targets::fav) | toUType(req_targets::add)):
            {
                auto jv = boost::json::parse(body);
                if(jv.as_object().contains("user_id") 
                    && jv.as_object().contains("anime_id"))
                {
                    auto res = add_user_favourite(
                        user_data_db_, 
                        jv.at("user_id").as_string().c_str(),
                        jv.at("anime_id").as_string().c_str()
                    );
                    std::cout << "Favorite add result: " << res << std::endl;
                    boost::json::object response_object;
                    response_object["success"] = res;
                    send_json_response(http::status::ok, boost::json::serialize(response_object));
                } else{
                    send_text_response(http::status::bad_request, "Bad request");
                }
            }
            break;
        case (toUType(req_targets::users) | toUType(req_targets::name) | toUType(req_targets::set)):
            {
                auto jv = boost::json::parse(body);
                if(jv.as_object().contains("user_id") 
                    && jv.as_object().contains("name"))
                {
                    auto res = set_user_name(
                        user_data_db_, 
                        jv.at("user_id").as_string().c_str(),
                        jv.at("name").as_string().c_str()
                    );
                    std::cout << "User name change result: " << res << std::endl;
                    boost::json::object response_object;
                    response_object["success"] = res;
                    send_json_response(http::status::ok, boost::json::serialize(response_object));
                } else{
                    send_text_response(http::status::bad_request, "Bad request");
                }
            }
            break;
        case (toUType(req_targets::users) | toUType(req_targets::last_video) | toUType(req_targets::set)):
            {
                auto jv = boost::json::parse(body);
                if(jv.as_object().contains("user_id") 
                    && jv.as_object().contains("video_id"))
                {
                    auto res = set_user_last_video(
                        user_data_db_, 
                        jv.at("user_id").as_string().c_str(),
                        jv.at("video_id").as_string().c_str()
                    );
                    std::cout << "Last video set result: " << res << std::endl;
                    boost::json::object response_object;
                    response_object["success"] = res;
                    send_json_response(http::status::ok, boost::json::serialize(response_object));
                } else{
                    send_text_response(http::status::bad_request, "Bad request");
                }
            }
            break;
        case (toUType(req_targets::users) | toUType(req_targets::fav) | toUType(req_targets::remove)):
            {
                auto jv = boost::json::parse(body);
                if(jv.as_object().contains("user_id") 
                    && jv.as_object().contains("anime_id"))
                {
                    auto res = remove_user_favourite(
                        user_data_db_, 
                        jv.at("user_id").as_string().c_str(),
                        jv.at("anime_id").as_string().c_str()
                    );
                    std::cout << "Favorite remove result: " << res << std::endl;
                    boost::json::object response_object;
                    response_object["success"] = res;
                    send_json_response(http::status::ok, boost::json::serialize(response_object));
                } else{
                    send_text_response(http::status::bad_request, "Bad request");
                }
            }
            break;
        case (toUType(req_targets::anime) | toUType(req_targets::video) | toUType(req_targets::add)):
            {
                auto jv = boost::json::parse(body);
                if(jv.as_object().contains("anime_id") 
                    && jv.as_object().contains("video_id"))
                {
                    auto res = add_anime_video(
                        anime_db_, 
                        jv.at("anime_id").as_string().c_str(),
                        jv.at("video_id").as_string().c_str()
                    );
                    std::cout << "Video add result: " << res << std::endl;
                    boost::json::object response_object;
                    response_object["success"] = res;
                    send_json_response(http::status::ok, boost::json::serialize(response_object));
                } else{
                    send_text_response(http::status::bad_request, "Bad request");
                }
            }
            break;
        case (toUType(req_targets::anime) | toUType(req_targets::video) | toUType(req_targets::remove)):
            {
                std::cout << "here" << std::endl;
                auto jv = boost::json::parse(body);
                if(jv.as_object().contains("anime_id") 
                    && jv.as_object().contains("video_id"))
                {
                    auto res = remove_anime_video(
                        anime_db_, 
                        jv.at("anime_id").as_string().c_str(),
                        jv.at("video_id").as_string().c_str()
                    );
                    std::cout << "Video remove result: " << res << std::endl;
                    boost::json::object response_object;
                    response_object["success"] = res;
                    send_json_response(http::status::ok, boost::json::serialize(response_object));
                } else{
                    send_text_response(http::status::bad_request, "Bad request");
                }
            }
            break;
        case (toUType(req_targets::anime) | toUType(req_targets::update)):
            {
                auto jv = boost::json::parse(body);
                if(jv.as_object().contains("id")
                    && jv.as_object().contains("title") 
                    && jv.as_object().contains("description") 
                    && jv.as_object().contains("videos")
                    && jv.as_object().contains("episodes"))
                {
                    auto anime_id = jv.as_object().at("id").try_as_string();
                    if(!anime_id.has_value()){
                        send_text_response(http::status::bad_request, "Bad request");
                    }
                    boost::json::object anime_data;
                    anime_data["title"] = jv.as_object().at("title"); 
                    anime_data["description"] = jv.as_object().at("description"); 
                    anime_data["videos"] = jv.as_object().at("videos"); 
                    anime_data["episodes"] = jv.as_object().at("episodes"); 
                    auto res = update_anime(
                        anime_db_, 
                        anime_id->c_str(),
                        boost::json::serialize(anime_data)
                    );
                    std::cout << "Anime update result: " << res.has_value() << std::endl;
                    boost::json::object response_object;
                    if(res){
                        boost::json::object anime_brief;
                        anime_brief["title"] = jv.as_object().at("title");
                        anime_brief["anime_id"] = *anime_id;
                        auto idx_id = get_index_id(anime_search_db_, anime_id->c_str());
                        if(idx_id){
                            boost::json::object idx_data;
                            idx_data["doc"] = anime_brief;
                            std::cout << idx_data << std::endl;
                            auto idx_result = update_indexed_anime(
                                anime_search_db_, 
                                *idx_id,
                                boost::json::serialize(idx_data)
                            );
                            std::cout << "Index update result: " << idx_result.has_value() << std::endl;
                            response_object["success"] = idx_result.has_value();
                        } else{
                            response_object["success"] = false;
                        }
                    } else{
                        response_object["success"] = false;
                    }
                    send_json_response(http::status::ok, boost::json::serialize(response_object));
                } else{
                    send_text_response(http::status::bad_request, "Bad request");
                }
            }
            break;
        case (toUType(req_targets::anime) | toUType(req_targets::del)):
            {
                auto jv = boost::json::parse(body);
                if(jv.as_object().contains("id"))
                {
                    auto anime_id = jv.as_object().at("id").try_as_string();
                    if(!anime_id.has_value()){
                        send_text_response(http::status::bad_request, "Bad request");
                    }
                    auto res = delete_anime(
                        anime_db_, 
                        anime_id->c_str()
                    );
                    std::cout << "Anime delete result: " << res << std::endl;
                    boost::json::object response_object;
                    if(res){
                        auto idx_id = get_index_id(anime_search_db_, anime_id->c_str());
                        if(idx_id){
                            auto idx_result = delete_indexed_anime(
                                anime_search_db_, 
                                *idx_id
                            );
                            std::cout << "Index delete result: " << idx_result.has_value() << std::endl;
                            response_object["success"] = idx_result.has_value();
                        } else{
                            response_object["success"] = false;
                        }
                    } else{
                        response_object["success"] = false;
                    }
                    send_json_response(http::status::ok, boost::json::serialize(response_object));
                } else{
                    send_text_response(http::status::bad_request, "Bad request");
                }
            }
            break;
        case (toUType(req_targets::chat) | toUType(req_targets::msg) | toUType(req_targets::remove)):
            {
                auto jv = boost::json::parse(body);
                if(jv.as_object().contains("message_id"))
                {
                    auto message_id = jv.as_object().at("message_id").as_string();
                    auto res = delete_message(
                        chat_db_, 
                        message_id.c_str()
                    );
                    std::cout << "Message delete result: " << res << std::endl;
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