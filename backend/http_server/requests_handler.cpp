#include "server.hpp"

#include <cstdint>
#include <memory>
#include <type_traits>
#include <vector>

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
            encoded += (0x01);
        }
        else if(tr == "chat"){
            encoded += (0x02);
        }
        if(tr == "register"){
            encoded += (0x0100);
        }
        else if(tr == "login"){
            encoded += (0x0200);
        }
    }
    return encoded;
}

enum class req_targets: uint32_t{
    auth = 0x01,
    chat = 0x02,

    reg = 0x0100,
    login = 0x0200

};

template<typename E>
constexpr auto toUType(E e) noexcept {
    return static_cast<std::underlying_type_t<E>>(e);
}

bool login_user(std::shared_ptr<UserDataDB> db, const std::string& login, const std::string& password_hash){
    auto uq = db->UserExist(login, password_hash);
    return (uq.has_value() && *uq);
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
                        user_data_db, 
                        jv.at("login").as_string().c_str(), 
                        jv.at("password_hash").as_string().c_str()
                    );
                }
            }
            break;

        default:
            break;
    }
}