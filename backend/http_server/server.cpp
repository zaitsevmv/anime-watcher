#include "server.hpp"

#include <boost/beast/core/error.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/verb.hpp>

#include <boost/beast/http/write.hpp>
#include <chrono>
#include <cstddef>
#include <tuple>
#include <utility>
#include <iostream>

using namespace std::chrono_literals;

http_worker::http_worker(tcp::acceptor& acceptor) :
    acceptor_(acceptor)
{}

void http_worker::start() {
    accept();
    check_deadline();
}

void http_worker::set_anime_db(std::shared_ptr<AnimeDB> db) {
    anime_db_ = std::move(db);
}

void http_worker::set_user_data_db(std::shared_ptr<UserDataDB> db) {
    user_data_db_ = std::move(db);
}

void http_worker::set_anime_search_db(std::shared_ptr<AnimeSearchDB> db) {
    anime_search_db_ = std::move(db);
}

void http_worker::set_chat_db(std::shared_ptr<ChatDB> db) {
    chat_db_ = std::move(db);
}

void http_worker::accept() {
    boost::beast::error_code ec;
    socket_.close(ec);
    buffer_.consume(buffer_.size());

    acceptor_.async_accept(
        socket_,
        [this](boost::beast::error_code ec){
            if(ec){
                accept();
            } else{
                request_deadline_.expires_after(60s);
                read_request();
            }
        }
    );
}

void http_worker::check_deadline() {
    if(request_deadline_.expiry() < std::chrono::steady_clock::now()){
        boost::beast::error_code ec;
        socket_.close(ec);

        request_deadline_.expires_at(std::chrono::steady_clock::time_point::max());
    }
    request_deadline_.async_wait(
        [this](boost::beast::error_code){
            check_deadline();
        }
    );
}

void http_worker::read_request() {
    parser_.emplace(std::piecewise_construct, std::make_tuple(), std::make_tuple(alloc_));
    http::async_read(
        socket_, 
        buffer_,
        parser_->get(),
        [this](boost::beast::error_code ec, std::size_t){
            if(ec){
                accept();
            } else{
                process_request(parser_->get());
            }
        }
    );
}

void http_worker::process_request(const http::request<request_body_t, http::basic_fields<alloc_t>>& req) {
    switch (req.method()) {
        case http::verb::get:
            process_get_request(req);
            accept();
            break;
        case http::verb::post:
            process_post_request(req);
            accept();
            break;
        default:
            break;
    }
}

void http_worker::send_json_response(http::status status, const std::string& message) {
    string_response_.emplace(std::piecewise_construct, std::make_tuple(), std::make_tuple(alloc_));

    string_response_->result(status);
    string_response_->keep_alive(false);
    string_response_->set(http::field::server, "Beast");
    string_response_->set(http::field::content_type, "application/json");  
    string_response_->body() = message;
    string_response_->prepare_payload();

    string_serializer_.emplace(*string_response_);

    http::async_write(
        socket_, 
        *string_serializer_, 
        [this](boost::beast::error_code ec, std::size_t){
            socket_.shutdown(tcp::socket::shutdown_send, ec);
            string_serializer_.reset();
            string_response_.reset();
            accept();
        }
    );
}

void http_worker::send_text_response(http::status status, const std::string& message) {
    string_response_.emplace(std::piecewise_construct, std::make_tuple(), std::make_tuple(alloc_));

    string_response_->result(status);
    string_response_->keep_alive(false);
    string_response_->set(http::field::server, "Beast");
    string_response_->set(http::field::content_type, "text/plain");  
    string_response_->body() = message;
    string_response_->prepare_payload();

    string_serializer_.emplace(*string_response_);

    http::async_write(
        socket_, 
        *string_serializer_, 
        [this](boost::beast::error_code ec, std::size_t){
            socket_.shutdown(tcp::socket::shutdown_send, ec);
            string_serializer_.reset();
            string_response_.reset();
            accept();
        }
    );
}
