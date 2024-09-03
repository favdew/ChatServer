#include <iostream>
#include <fstream>
#include <memory>
#include <queue>
#include <string>
#include <thread>
#include <application/ServerTerminal.h>
#include <boost/asio.hpp>
#include <network/chat_server.h>
#include <network/chat_session.h>

using boost::asio::ip::tcp;

int main()
{
    boost::asio::io_context io_context;
    chat_server server(io_context, 12345);

    std::thread order_thread([&server]()
    {
        ServerTerminal terminal(server);
        std::string order;
        while(std::getline(std::cin, order))
        {
            terminal.order(order);
        }
    });
    
    io_context.run();

    return 0;
}