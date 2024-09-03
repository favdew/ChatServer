#ifndef CHAT_SERVER_INTERFACE
#define CHAT_SERVER_INTERFACE

#include <string>

class chat_server_interface
{
public:
    virtual ~chat_server_interface() {}
    virtual bool exist_user_for_id(const std::string& id) = 0;
};

#endif