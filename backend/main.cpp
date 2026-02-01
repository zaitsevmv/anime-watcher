#include "anime_search_db/anime_search_db.hpp"
#include "base_mongodb.hpp"
#include "http_server/server.hpp"

#include <iostream>
#include <list>
#include <memory>
#include <vector>

int main(int argc, char* argv[])
{
    try
    {
        auto animedb = std::make_shared<AnimeDB>("anime_db", "anime");
        auto user_data_db = std::make_shared<UserDataDB>("anime_db", "users");
        auto chat_db = std::make_shared<ChatDB>("anime_db", "chat");
        auto anime_search_db = std::make_shared<AnimeSearchDB>("anime_db");

        auto const address = boost::asio::ip::make_address("192.168.0.10");
        unsigned short port = static_cast<unsigned short>(8080);
        int num_workers = 3;
        bool spin = true;

        boost::asio::io_context ioc{1};
        tcp::acceptor acceptor{ioc, {address, port}};

        std::list<http_worker> workers;
        for (int i = 0; i < num_workers; ++i) {
            workers.emplace_back(acceptor);
            auto& cur_worker = workers.back();

            cur_worker.set_anime_db(animedb);
            cur_worker.set_user_data_db(user_data_db);
            cur_worker.set_chat_db(chat_db);
            cur_worker.set_anime_search_db(anime_search_db);

            workers.back().start();
        }

        if (spin)
          for (;;) ioc.poll();
        else
          ioc.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}