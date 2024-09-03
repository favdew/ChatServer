#ifndef CHAT_SERVER
#define CHAT_SERVER

#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <memory>
#include <set>
#include <sys/socket.h>
#include <thread>
#include <vector>
#include <boost/asio.hpp>
#include <application/Logger.h>
#include <application/ThreadQueue.h>
#include <application/ThreadedSet.hpp>
#include <network/chat_server_interface.h>
#include <network/chat_session.h>
#include <network/chat_room.h>

using boost::asio::ip::tcp;

class chat_server : public chat_server_interface
{
typedef std::shared_ptr<chat_room> room_ptr;

public:
    chat_server(boost::asio::io_context &io_context, short port);
    ~chat_server();
    std::size_t session_count();
    std::size_t room_count();
    std::shared_ptr<chat_session> get_session(std::string ip, unsigned short port);
    std::vector<std::shared_ptr<chat_session>> get_sessions();

    void destroy_session_in_session(std::shared_ptr<chat_session> session);
    void safety_exit();
public:
    bool exist_user_for_id(const std::string& id);
private:
    void do_accept();

    std::string get_connect_success_log(const tcp::socket& socket);

    tcp::acceptor acceptor_;                                                //Object accepting sockets
    std::thread cleaner_thread_;                                            //Thread removing closed or
                                                                            //unavailable session from set

    //ThreadedSet<std::shared_ptr<chat_session>, session_compare> sessions_;
    std::set<std::shared_ptr<chat_session>, session_compare> sessions_;     //Set saved sessions
    std::mutex sessions_mutex_;

    //ThreadedSet<room_ptr, chat_room_compare> room_set_;
    std::set<room_ptr, chat_room_compare> room_set_;                        //Set saved room
    std::mutex room_set_mutex_;

    std::size_t max_user_num_;                                              //Maximum of accepted user number at once
    std::size_t rpacket_num_;                                               //Number of read body packet

    const unsigned int timeout_milli = 10000;
};

#endif