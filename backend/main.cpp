#include "http_server/server.hpp"
#include "database_logic/animedb/animedb.hpp"

#include <iostream>
#include <vector>

int main(int argc, char* argv[])
{
    try
    {
        // if (argc != 6)
        // {
        //     std::cerr << "Usage: http_server_fast <address> <port> <doc_root> <num_workers> {spin|block}\n";
        //     std::cerr << "  For IPv4, try:\n";
        //     std::cerr << "    http_server_fast 0.0.0.0 80 . 100 block\n";
        //     std::cerr << "  For IPv6, try:\n";
        //     std::cerr << "    http_server_fast 0::0 80 . 100 block\n";
        //     return EXIT_FAILURE;
        // }

        // auto const address = boost::asio::ip::make_address(argv[1]);
        // unsigned short port = static_cast<unsigned short>(std::atoi(argv[2]));
        // std::string doc_root = argv[3];
        // int num_workers = 3;
        // bool spin = (std::strcmp(argv[5], "spin") == 0);

        auto animedb = AnimeDB();
        animedb.CreateDatabase();

        auto const address = boost::asio::ip::make_address("0.0.0.0");
        unsigned short port = static_cast<unsigned short>(8080);
        int num_workers = 3;
        bool spin = true;

        boost::asio::io_context ioc{1};
        tcp::acceptor acceptor{ioc, {address, port}};

        std::list<http_worker> workers;
        for (int i = 0; i < num_workers; ++i) {
            workers.emplace_back(acceptor);
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