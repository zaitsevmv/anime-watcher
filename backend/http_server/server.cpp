#include "server.hpp"

#include <boost/beast/core/error.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/verb.hpp>

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
                std::cout << "New connection" << socket_.remote_endpoint() << std::endl;
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
            accept();
            break;
        case http::verb::post:
            accept();
            break;
        default:
            break;
    }
}
