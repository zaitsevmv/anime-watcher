#include "server.hpp"

void http_worker::process_get_request(const http::request<request_body_t, http::basic_fields<alloc_t>>& req) {
    std::cout << "Got GET request: " << req.body() << std::endl; 
}

void http_worker::process_post_request(const http::request<request_body_t, http::basic_fields<alloc_t>>& req) {
    std::cout << "Got POST request: " << req.body() << std::endl; 
}