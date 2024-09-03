#include <iostream>
#include <string>
#include <cstring>
#include <network/chat_server.h>

class ServerTerminal
{
public:
    ServerTerminal(chat_server& server);
    ~ServerTerminal();

    void order(std::string o);
private:
    chat_server& server_;
};