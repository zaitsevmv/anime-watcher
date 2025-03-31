#ifndef __RESPONSE_SERVER_H__
#define __RESPONSE_SERVER_H__

#include "fields_alloc.hpp"
#include "anime_db/anime_db.hpp"
// #include "anime_search_db/anime_search_db.hpp"


#include <boost/asio/basic_waitable_timer.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/core/flat_static_buffer.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/fields.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/parser.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

#include <chrono>
#include <memory>
#include <optional>
#include <string>

namespace ip = boost::asio::ip;         // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio.hpp>
namespace http = boost::beast::http;    // from <boost/beast/http.hpp>

class http_worker{
public:
    http_worker(const http_worker&) = delete;
    http_worker& operator=(const http_worker&) = delete;

    http_worker(tcp::acceptor& acceptor);

    void start();

    void set_anime_db(AnimeDB& db);

private:
    using alloc_t = fields_alloc<char>;
    using request_body_t = http::string_body;

    void accept();
    void check_deadline();

    void read_request();
    void process_request(const http::request<request_body_t, http::basic_fields<alloc_t>>& req);

    void process_get_request(const http::request<request_body_t, http::basic_fields<alloc_t>>& req);
    void process_post_request(const http::request<request_body_t, http::basic_fields<alloc_t>>& req);

    tcp::acceptor& acceptor_;

    tcp::socket socket_{acceptor_.get_executor()};
    boost::beast::flat_static_buffer<8192> buffer_;
    boost::asio::basic_waitable_timer<std::chrono::steady_clock> request_deadline_{
        acceptor_.get_executor(), std::chrono::steady_clock::time_point::max()
    };

    std::optional<http::request_parser<request_body_t, alloc_t>> parser_;
    alloc_t alloc_{8192};

    std::shared_ptr<AnimeDB> anime_db;
};

#endif