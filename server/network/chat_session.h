#ifndef CHATSESSION
#define CHATSESSION

#include <iostream>
#include <fstream>
#include <cstring>
#include <queue>
#include <memory>
#include <mutex>
#include <random>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <application/ThreadQueue.h>
#include <application/ThreadedSet.hpp>
#include <application/crypto_proc.h>
#include <network/chat_server_interface.h>
#include <network/chat_message.hpp>
#include <network/chat_participant.h>
#include <network/chat_room.h>
#include <db/account_proc.h>

using boost::asio::ip::tcp;

const std::size_t max_buf_size = 10240;
const std::size_t header_size = 16;
const long long max_room_size = 1000;

//Structure of Keep Alive information
struct kal_info
{
public:
    int is_kal;
    int idle;
    int interval;
    int maxpkt;
};

//Structure of task written
struct wtask
{
public:
    wtask(int _task_type, char * _task_data);
    ~wtask();
    void clear_data();

    int task_type;
    char *task_data;

    enum ttype {
        TTYPE_HEADER = 1,
        TTYPE_BODY
    };
};

//User Session
class chat_session : public chat_participant,
    public std::enable_shared_from_this<chat_session>
{
typedef std::shared_ptr<chat_room> room_ptr;

public:
    chat_session(tcp::socket socket, ThreadedSet<room_ptr, chat_room_compare>& room_set, std::mutex& room_set_mutex);
    chat_session(tcp::socket socket, std::set<room_ptr, chat_room_compare>& room_set, std::mutex& room_set_mutex);
    ~chat_session();

    void start();
    void close();
    void write(const char* msg, std::size_t length);
    const std::string ip();
    const std::string ip() const;
    const unsigned short port();
    const unsigned short port() const;
    const std::string get_stored_ip();
    const std::string get_stored_ip() const;
    const unsigned short get_stored_port();
    const unsigned short get_stored_port() const;
    void get_read_header_buf(char* dest);
    void get_read_buf(char* dest);

    bool is_socket_open();
    bool is_available();
    bool is_run();

    void set_chat_server_interfafce(chat_server_interface *interface);

    std::size_t get_wq_size();

    std::string id();
    std::string id() const;

    bool operator<(const chat_session &session);
    bool operator>(const chat_session &session);
    bool operator==(const chat_session &session);

    bool operator<(const chat_participant& participant);
    bool operator>(const chat_participant& participant);
    bool operator==(const chat_participant& participant);

    void delivery(const char* header_data, const char* data);
    void remove_from_room(bool clear_room = true);
    kal_info get_kal_info();
    int get_syncnt();

    std::size_t received_packet_num_ = 0;

    bool is_reading_ = false;
protected:
    void read_header_packet_proc(boost::system::error_code ec, std::size_t length);
    void read_packet_proc(boost::system::error_code ec, std::size_t length);
    void write_header_packet_proc(boost::system::error_code ec, std::size_t length);
    void write_packet_proc(boost::system::error_code ec, std::size_t length);

    void do_read_header();
    void do_read(std::size_t body_length);
    void do_write_header(const char *str, bool im = false);
    void do_write(const char *str, std::size_t length);
    void do_close();
    
private:
    void store_ip(const std::string &ip);
    void store_port(const unsigned short port);

    void clear_wtask_queue();

    long long alloc_room_number();
    
    tcp::socket socket_;
    std::string stored_ip_;
    unsigned short stored_port_;
    std::queue<wtask> wtask_queue_;
    //ThreadedSet<room_ptr, chat_room_compare>& room_set_;
    std::set<room_ptr, chat_room_compare>& room_set_;
    std::mutex &room_set_mutex_;
    room_ptr current_room_;
    std::mutex current_room_mutex_;
    chat_server_interface *chat_server_interface_;

    crypto_proc cproc_;
    unsigned char aes_key_[CryptoPP::AES::DEFAULT_KEYLENGTH];

    char read_header_buffer_[header_size];
    char read_buffer_[max_buf_size];

    chat_message_header header_;

    std::string id_;
    std::string nickname_;
    bool is_run_ = false;
    
    bool is_change_passwd_granted = false;
};

struct session_compare
{
    typedef std::shared_ptr<chat_session> session_ptr;
    bool operator()(const session_ptr& left, const session_ptr& right) const;
};

#endif