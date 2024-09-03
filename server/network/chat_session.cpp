#include "chat_session.h"
#include "chat_message.hpp"

wtask::wtask(int _task_type, char * _task_data)
        : task_type(_task_type), task_data(_task_data)
{
}

wtask::~wtask()
{
}

void wtask::clear_data()
{
    delete[] task_data;
}

chat_session::chat_session(tcp::socket socket, std::set<room_ptr, chat_room_compare>& room_set, std::mutex& room_set_mutex)
    : socket_(std::move(socket)), room_set_(room_set), id_(""), nickname_(""), room_set_mutex_(room_set_mutex),
    chat_server_interface_(nullptr)
{
    current_room_ = nullptr;
    store_ip(ip());
    store_port(port());
}

chat_session::~chat_session()
{
}

void chat_session::start()
{
    is_run_ = true;
    do_read_header();
}

void chat_session::close()
{
    do_close();
}

void chat_session::write(const char* msg, std::size_t length)
{
    do_write(msg, length);
}

const std::string chat_session::ip() const
{
    boost::system::error_code ec;
    auto ep = socket_.remote_endpoint(ec);
    if(ec) return "";
    return ep.address().to_string();
}

const std::string chat_session::ip()
{
    boost::system::error_code ec;
    auto ep = socket_.remote_endpoint(ec);
    if(ec) return "";
    return ep.address().to_string();
}

const unsigned short chat_session::port() const
{
    boost::system::error_code ec;
    auto ep = socket_.remote_endpoint(ec);
    if(ec) return 0;
    return ep.port();
}

const unsigned short chat_session::port()
{
    boost::system::error_code ec;
    auto ep = socket_.remote_endpoint(ec);
    if(ec) return 0;
    return ep.port();
}

const std::string chat_session::get_stored_ip()
{
    return stored_ip_;
}

const std::string chat_session::get_stored_ip() const
{
    return stored_ip_;
}

const unsigned short chat_session::get_stored_port()
{
    return stored_port_;
}

const unsigned short chat_session::get_stored_port() const
{
    return stored_port_;
}

void chat_session::get_read_header_buf(char* dest)
{
    std::memset(dest, 0, header_size);
    std::memcpy(dest, read_header_buffer_, header_size);
}

void chat_session::get_read_buf(char* dest)
{
    std::memset(dest, 0, max_buf_size);
    std::memcpy(dest, read_buffer_, max_buf_size);
}

bool chat_session::is_socket_open()
{
    return socket_.is_open();
}

bool chat_session::is_available()
{
    if(this->ip() == "" || this->port() == 0) return false;
    return true;
}

bool chat_session::is_run()
{
    return is_run_;
}

void chat_session::set_chat_server_interfafce(chat_server_interface *interface)
{
    chat_server_interface_ = interface;
}

std::size_t chat_session::get_wq_size()
{
    return wtask_queue_.size();
}

std::string chat_session::id()
{
    return id_;
}

std::string chat_session::id() const
{
    return id_;
}

bool chat_session::operator<(const chat_session &session)
{
    int cmp = std::strcmp(this->ip().c_str(),session.ip().c_str());
    if(cmp < 0) return true;
    else if(cmp == 0)
    {
        if(this->port() < session.port()) return true;
        else false;
    }
    else false;
}

bool chat_session::operator>(const chat_session &session)
{
    int cmp = std::strcmp(this->ip().c_str(),session.ip().c_str());
    if(cmp > 0) return true;
    else if(cmp == 0)
    {
        if(this->port() > session.port()) return true;
        else false;
    }
    else return false;
}

bool chat_session::operator==(const chat_session &session)
{
    if(std::strcmp(this->ip().c_str(),session.ip().c_str())) return false;
    if(this->port() != session.port()) return false;
    return true;
}

bool chat_session::operator<(const chat_participant& participant)
{
    return id_ < participant.id();
}

bool chat_session::operator>(const chat_participant& participant)
{
    return id_ > participant.id();
}

bool chat_session::operator==(const chat_participant& participant)
{
    return id_ == participant.id();
}

void chat_session::delivery(const char* header_data, const char* data)
{
    //Message will be transfered here
    chat_message_header header(header_data);
    do_write_header(header_data);
    do_write(data, header.body_size);
}

void chat_session::remove_from_room(bool clear_room)
{
    current_room_mutex_.lock();

    if(current_room_ != nullptr)
    {
        auto self(shared_from_this());
        
        current_room_->quit(self);
        
        current_room_->part_set_mutex_.lock();
        if(current_room_->get_participants_num() <= 0)
        {
            current_room_->part_set_mutex_.unlock();
            room_set_mutex_.lock();
            auto it = room_set_.find(current_room_);
            if(it != room_set_.end())
                room_set_.erase(it);
            room_set_mutex_.unlock();
        }
        else
        {
            auto it = current_room_->get_participants().begin();
            if(it != current_room_->get_participants().end())
                current_room_->set_owner_id((*it)->id());
            current_room_->part_set_mutex_.unlock();

            //Notify exiting this session to other clients in room
            talk_data ntf_td(id_, nickname_, talk_data::TDF_EXIT, "");
            cm_send_room_message ntf_message(ntf_td);
            chat_message_header ntf_header(ntf_message.size(),
                chat_message_header::MT_SENDMESSAGE);
            
            auto nhbuf = ntf_header.serialization();
            auto nbuf = ntf_message.serialization();

            current_room_->delivery(nhbuf, nbuf);

            delete[] nhbuf;
            delete[] nbuf;
        }
        
        if(clear_room)
            current_room_ = nullptr;
    }
    
    current_room_mutex_.unlock();
}

kal_info chat_session::get_kal_info()
{
    kal_info ret;

    int is_kal;
    socklen_t sz_is_kal;
    getsockopt(socket_.native_handle(), SOL_SOCKET, SO_KEEPALIVE, &is_kal, &sz_is_kal);

    int idle;
    socklen_t sz_idle;
    getsockopt(socket_.native_handle(), IPPROTO_TCP, TCP_KEEPIDLE, &idle, &sz_idle);

    int interval;
    socklen_t sz_interval;
    getsockopt(socket_.native_handle(), IPPROTO_TCP, TCP_KEEPINTVL, &interval, &sz_interval);

    int maxpkt;
    socklen_t sz_maxpkt;
    getsockopt(socket_.native_handle(), IPPROTO_TCP, TCP_KEEPCNT, &maxpkt, &sz_maxpkt);
    
    ret.is_kal = is_kal;
    ret.idle = idle;
    ret.interval = interval;
    ret.maxpkt = maxpkt;

    return ret;
}

int chat_session::get_syncnt()
{
    int syncnt;
    socklen_t sz_syncnt;
    timeval timeout;

    getsockopt(socket_.native_handle(), IPPROTO_TCP, TCP_SYNCNT, &syncnt, &sz_syncnt);

    return syncnt;
}

void chat_session::read_header_packet_proc(boost::system::error_code ec, std::size_t length)
{
    if(!ec)
    {
        if(length < header_size)
        {
            is_reading_ = false;
            remove_from_room();
            do_close();
            return;
        }
        char* rh_buf = new char[header_size];
        get_read_header_buf(rh_buf); 
        header_ = convert_to_header(rh_buf);
        
        delete [] rh_buf;

        if( header_.is_valid() && header_.message_type != 0)
        {
            do_read(header_.body_size);
            return;
        }
        else
        {
            std::cout << "packet header is invalid\n";
            std::cout << "body_size=" << header_.body_size << ", message_type="
                << header_.message_type << "\n";
            is_reading_ = false;
            remove_from_room();
            do_close();
            return;
        }

    }
    else
    {
        is_reading_ = false;
        remove_from_room();
        do_close();

        std::cerr << "read_header_packet_proc error: " <<
            std::endl << ec.message() << std::endl;
        std::cerr << "port:" << port() << std::endl;

        return;
    }
}

void chat_session::read_packet_proc(boost::system::error_code ec, std::size_t length)
{
    if(!ec)
    {
        received_packet_num_++;

        char* r_buf = new char[max_buf_size];
        
        get_read_buf(r_buf);
        switch(header_.message_type)
        {
            case chat_message_header::MT_CREATEROOM:
            {
                cm_room_message msg(r_buf);

                long long new_room_number = alloc_room_number();

                if(new_room_number == -1)
                {
                    cm_room_message response("", "", new_room_number);
                    chat_message_header res_header(response.size(),
                        chat_message_header::MT_CREATEROOM);

                    auto res_hbuf = res_header.serialization();
                    auto res_buf = response.serialization();

                    do_write_header(res_hbuf);
                    do_write(res_buf, response.size());
                }

                else
                {
                    auto self(shared_from_this());
                    room_ptr new_room_ptr =
                        std::make_shared<chat_room>(new_room_number, msg.room_name,
                        msg.id, self);
                    room_set_mutex_.lock();
                    room_set_.emplace(new_room_ptr);
                    room_set_mutex_.unlock();
                    
                    current_room_mutex_.lock();
                    current_room_ = new_room_ptr;

                    //After completion of creating a room, respond to client
                    //with information of room
                    cm_room_message response(new_room_ptr->owner_id().c_str(),
                        new_room_ptr->room_name().c_str(),
                        new_room_ptr->room_number());
                    current_room_->part_set_mutex_.lock();
                    response.set_room_participants_id(new_room_ptr->get_participants());
                    current_room_->part_set_mutex_.unlock();
                    current_room_mutex_.unlock();

                    chat_message_header res_header(response.size(), chat_message_header::MT_CREATEROOM);
                    auto res_hbuf = res_header.serialization();
                    
                    char* res_buf = response.serialization();
                    do_write_header(res_hbuf);
                    do_write(res_buf, response.size());

                    delete[] res_hbuf;
                    delete[] res_buf;

                    talk_data td(id_, nickname_, talk_data::TDF_CREATE,
                        std::to_string(new_room_number));
                    cm_send_room_message ntf_message(td);
                    chat_message_header ntf_header(ntf_message.size(),
                        chat_message_header::MT_SENDMESSAGE);

                    auto nhbuf = ntf_header.serialization();
                    auto nbuf = ntf_message.serialization();

                    current_room_mutex_.lock();
                    current_room_->delivery(nhbuf, nbuf);
                    current_room_mutex_.unlock();

                    do_write_header(nhbuf);
                    do_write(nbuf, ntf_header.body_size);

                    delete[] nhbuf;
                    delete[] nbuf;
                }
                break;
            }
            case chat_message_header::MT_CREATEKEY:
            {
                cm_create_key_message message(r_buf);

                std::string rsa_public_key(message.data, message.key_length);

                cproc_.LoadPublicKey(rsa_public_key);

                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<int> dis(0, 255);

                for(int i = 0; i < sizeof(aes_key_); i++)
                {
                    aes_key_[i] = dis(gen);
                }

                std::memset(aes_key_, 1, sizeof(aes_key_));

                cproc_.SetAESKey(aes_key_);

                unsigned char *enc_aes_key;
                std::size_t enc_size = 6144;
                std::size_t enc_size2 = enc_size;
                enc_aes_key = new unsigned char[enc_size];

                cproc_.EncryptWithRSA(aes_key_, enc_aes_key, enc_size);

                cm_create_key_message res_message(
                    (const char *)enc_aes_key, enc_size);
                chat_message_header res_header(res_message.size(),
                    chat_message_header::MT_CREATEKEY);

                auto res_hbuf = res_header.serialization();
                auto res_buf = res_message.serialization();

                do_write_header(res_hbuf);
                do_write(res_buf, res_header.body_size);

                delete[] enc_aes_key;
                delete[] res_hbuf;
                delete[] res_buf;

                break;
            }
            case chat_message_header::MT_LOGIN:
            {
               unsigned char iv[CryptoPP::AES::DEFAULT_BLOCKSIZE];

                std::memcpy(iv, r_buf + (header_.body_size
                    - CryptoPP::AES::DEFAULT_BLOCKSIZE),
                        CryptoPP::AES::DEFAULT_BLOCKSIZE);

                char pure_body[header_.body_size - CryptoPP::AES::DEFAULT_BLOCKSIZE];
                std::memcpy(pure_body, r_buf, header_.body_size - CryptoPP::AES::DEFAULT_BLOCKSIZE);

                std::string enc_body_data(pure_body,
                    header_.body_size - CryptoPP::AES::DEFAULT_BLOCKSIZE);
                std::string body_data;

                cproc_.DecryptWithAES(enc_body_data, body_data, iv);

                cm_login_message message(body_data.c_str());

                /*****************************************************
                 * Statement of Searching received information is right
                *****************************************************/

                std::string enc_passwd;
                std::string passwd(message.password);
                cproc_.ToHash(passwd, enc_passwd);

                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<int> dis(0, 255);
                unsigned char res_iv[CryptoPP::AES::DEFAULT_BLOCKSIZE];

                for (int i = 0; i < sizeof(iv); i++)
                {
                    res_iv[i] = dis(gen);
                }

                //When server has searched right information
                if(!account_proc::instance().login(message.id, enc_passwd.c_str()))
                {
                    //When the id exists already have been logined
                    if(chat_server_interface_->exist_user_for_id(message.id))
                    {
                        cm_login_message res_message(CODE_ALREADY_LOGINED, "");
                        res_message.set_iv(res_iv);
                        auto res_buf = res_message.serialization_except_iv();

                        std::string str_buf(res_buf, res_message.size() - res_message.iv_size());
                        std::string buf_output;

                        cproc_.EncryptWithAES(str_buf, buf_output, res_message.iv);

                        std::size_t final_size = buf_output.size() + res_message.iv_size();
                        char* wbuf = new char[final_size];
                        std::memcpy(wbuf, buf_output.c_str(), buf_output.size());
                        std::memcpy(wbuf + buf_output.size(), res_message.iv, res_message.iv_size());

                        chat_message_header res_header(final_size,
                            chat_message_header::MT_LOGIN);

                        auto res_hbuf = res_header.serialization();

                        do_write_header(res_hbuf);
                        do_write(wbuf, res_header.body_size);

                        delete[] res_buf;
                        delete[] wbuf;
                        delete[] res_hbuf;
                    }
                    else
                    {
                        id_ = message.id;

                        //Temporary setting to nickname
                        nickname_ = "nick" + std::string(message.id);

                        cm_login_message res_message(id_.c_str(), "");

                        res_message.set_iv(res_iv);
                        auto res_buf = res_message.serialization_except_iv();

                        std::string str_buf(res_buf, res_message.size() - res_message.iv_size());
                        std::string buf_output;

                        cproc_.EncryptWithAES(str_buf, buf_output, res_message.iv);

                        std::size_t final_size = buf_output.size() + res_message.iv_size();
                        char* wbuf = new char[final_size];
                        std::memcpy(wbuf, buf_output.c_str(), buf_output.size());
                        std::memcpy(wbuf + buf_output.size(), res_message.iv, res_message.iv_size());

                        chat_message_header res_header(final_size,
                            chat_message_header::MT_LOGIN);

                        auto res_hbuf = res_header.serialization();

                        do_write_header(res_hbuf);
                        do_write(wbuf, res_header.body_size);

                        delete[] res_buf;
                        delete[] wbuf;
                        delete[] res_hbuf;
                    } 
                }

                //When server has searched no right information
                else
                {
                    cm_login_message res_message;

                    res_message.set_iv(res_iv);
                    auto res_buf = res_message.serialization_except_iv();

                    std::string str_buf(res_buf, res_message.size() - res_message.iv_size());
                    std::string buf_output;

                    cproc_.EncryptWithAES(str_buf, buf_output, res_message.iv);

                    std::size_t final_size = buf_output.size() + res_message.iv_size();
                    char* wbuf = new char[final_size];
                    std::memcpy(wbuf, buf_output.c_str(), buf_output.size());
	                std::memcpy(wbuf + buf_output.size(), res_message.iv, res_message.iv_size());

                    chat_message_header res_header(final_size,
                        chat_message_header::MT_LOGIN);

                    auto res_hbuf = res_header.serialization();

                    do_write_header(res_hbuf);
                    do_write(wbuf, res_header.body_size);

                    delete[] res_buf;
                    delete[] wbuf;
                    delete[] res_hbuf;
                }
                
                break;
            }
            case chat_message_header::MT_SENDMESSAGE:
            {
                current_room_mutex_.lock();
                if(current_room_ != nullptr)
                {
                    cm_send_room_message message(r_buf);
                    
                    talk_data td(id_, nickname_, talk_data::TDF_MESSAGE,
                        message.td.content());
                    cm_send_room_message res_message(td);

                    chat_message_header res_header(message.size(),
                        chat_message_header::MT_SENDMESSAGE);

                    auto res_hbuf = res_header.serialization();
                    auto res_buf = res_message.serialization();
                    
                    current_room_->delivery(res_hbuf, res_buf);

                    delete[] res_hbuf;
                    delete[] res_buf;
                }
                
                current_room_mutex_.unlock();

                break;
            }

            case chat_message_header::MT_EXITROOM:
            {
                cm_exit_room_message message(r_buf);

                current_room_mutex_.lock();
                if(message.id == id_ && current_room_ != nullptr
                    && message.room_name == current_room_->room_name())
                {
                    auto self(shared_from_this());

                    current_room_->quit(self);

                    if(current_room_->get_participants_num() <= 0)
                    {
                        room_set_mutex_.lock();
                        auto it = room_set_.find(current_room_);
                        if(it != room_set_.end())
                        {
                            room_set_.erase(it);
                        }
                        room_set_mutex_.unlock();
                    }
                    else
                    {
                        current_room_->part_set_mutex_.lock();
                        auto it = current_room_->get_participants().begin();
                        if(it != current_room_->get_participants().end())
                        {
                            current_room_->set_owner_id((*it)->id());
                        }
                        current_room_->part_set_mutex_.unlock();

                        //Notify exiting this session to other clients in room
                        talk_data ntf_td(id_, nickname_, talk_data::TDF_EXIT, "");
                        cm_send_room_message ntf_message(ntf_td);
                        chat_message_header ntf_header(ntf_message.size(),
                            chat_message_header::MT_SENDMESSAGE);
                        
                        auto nhbuf = ntf_header.serialization();
                        auto nbuf = ntf_message.serialization();

                        current_room_->delivery(nhbuf, nbuf);
                        
                        delete[] nhbuf;
                        delete[] nbuf;
                    }
                    
                    current_room_ = nullptr;

                    chat_message_header res_header(message.size(),
                        chat_message_header::MT_EXITROOM);
                    auto rhbuf = res_header.serialization();
                    auto rbuf = message.serialization();
                    do_write_header(rhbuf);
                    do_write(rbuf, res_header.body_size);

                    delete[] rhbuf;
                    delete[] rbuf;
                }

                else
                {
                    if(current_room_ != nullptr)
                    {
                        std::cout << "Exit room: room name is different [" <<
                            current_room_->room_name() << "], [" <<
                            message.room_name << "]\n";
                    }
                    cm_exit_room_message res_message("", "");
                    chat_message_header res_header(res_message.size(),
                        chat_message_header::MT_EXITROOM);

                    auto rhbuf = res_header.serialization();
                    auto rbuf = res_message.serialization();
                    do_write_header(rhbuf);
                    do_write(rbuf, res_header.body_size);

                    delete[] rhbuf;
                    delete[] rbuf;
                }
                
                current_room_mutex_.unlock();
                break;
            }

            case chat_message_header::MT_JOINROOM:
            {
                cm_join_room_message message(r_buf);

                current_room_mutex_.lock();
                room_set_mutex_.lock();
                auto it = room_set_.begin();
                for(; it != room_set_.end(); it++)
                {
                    if( (*it)->room_number() == message.room_number)
                    {
                        current_room_ = (*it);
                        break;
                    }
                }

                //When searched no room with received room number
                if(it == room_set_.end())
                {
                    room_set_mutex_.unlock();

                    current_room_ = nullptr;

                    cm_join_room_message res_message;
                    chat_message_header res_header(res_message.size(),
                        chat_message_header::MT_JOINROOM);

                    auto rhbuf = res_header.serialization();
                    auto rbuf = res_message.serialization();

                    do_write_header(rhbuf);
                    do_write(rbuf, res_header.body_size);

                    delete[] rhbuf;
                    delete[] rbuf;
                }
                //When searched room with received room number
                else
                {
                    room_set_mutex_.unlock();
                    auto self(shared_from_this());

                    current_room_->join(self);
                    cm_join_room_message res_message(message.id, message.room_number,
                        current_room_->room_name().c_str());

                    chat_message_header res_header(res_message.size(),
                        chat_message_header::MT_JOINROOM);

                    auto rhbuf = res_header.serialization();
                    auto rbuf = res_message.serialization();

                    do_write_header(rhbuf);
                    do_write(rbuf, res_header.body_size);

                    delete[] rhbuf;
                    delete[] rbuf;

                    //Notify joining to session of the room
                    talk_data td(id_, nickname_, talk_data::TDF_JOIN, "");
                    cm_send_room_message ntf_message(td);
                    chat_message_header ntf_header(ntf_message.size(),
                        chat_message_header::MT_SENDMESSAGE);
                    auto nhbuf = ntf_header.serialization();
                    auto nbuf = ntf_message.serialization();

                    current_room_->delivery(nhbuf, nbuf);

                    delete[] nhbuf;
                    delete[] nbuf;
                }
                current_room_mutex_.unlock();
                break;
            }
            case chat_message_header::MT_REGISTER:
            {
                unsigned char iv[CryptoPP::AES::DEFAULT_BLOCKSIZE];

                std::memcpy(iv, r_buf + (header_.body_size
                    - CryptoPP::AES::DEFAULT_BLOCKSIZE),
                        CryptoPP::AES::DEFAULT_BLOCKSIZE);

                char pure_body[header_.body_size - CryptoPP::AES::DEFAULT_BLOCKSIZE];
                std::memcpy(pure_body, r_buf, header_.body_size - CryptoPP::AES::DEFAULT_BLOCKSIZE);

                std::string enc_body_data(pure_body,
                    header_.body_size - CryptoPP::AES::DEFAULT_BLOCKSIZE);
                std::string body_data;

                cproc_.DecryptWithAES(enc_body_data, body_data, iv);

                cm_register_message message(body_data.c_str());

                //check if received id exists
                std::string requested_id(message.id);

                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<int> dis(0, 255);
                unsigned char res_iv[CryptoPP::AES::DEFAULT_BLOCKSIZE];

                for (int i = 0; i < sizeof(iv); i++)
                {
                    res_iv[i] = dis(gen);
                }

                //When reqeusted id already exists
                if(account_proc::instance().id_exist(requested_id))
                {
                    cm_register_message res_message;

                    res_message.set_iv(res_iv);
                    auto res_buf = res_message.serialization_except_iv();

                    std::string str_buf(res_buf, res_message.size() - res_message.iv_size());
                    std::string buf_output;

                    cproc_.EncryptWithAES(str_buf, buf_output, res_message.iv);

                    std::size_t final_size = buf_output.size() + res_message.iv_size();
                    char* wbuf = new char[final_size];
                    std::memcpy(wbuf, buf_output.c_str(), buf_output.size());
	                std::memcpy(wbuf + buf_output.size(), res_message.iv, res_message.iv_size());

                    chat_message_header res_header(final_size, 
                        chat_message_header::MT_REGISTER);

                    auto res_hbuf = res_header.serialization();

                    do_write_header(res_hbuf);
                    do_write(wbuf, res_header.body_size);

                    delete[] res_buf;
                    delete[] wbuf;
                    delete[] res_hbuf;
                }
                else
                {
                    std::string requested_passwd(message.password);
                    std::string enc_passwd;
                    cproc_.ToHash(requested_passwd, enc_passwd);
                    account_proc::instance().register_user(message.id, enc_passwd,
                        message.name, message.email);

                    cm_register_message res_message(message.id, "", message.name, message.email);

                    res_message.set_iv(res_iv);
                    auto res_buf = res_message.serialization_except_iv();

                    std::string str_buf(res_buf, res_message.size() - res_message.iv_size());
                    std::string buf_output;

                    cproc_.EncryptWithAES(str_buf, buf_output, res_message.iv);

                    std::size_t final_size = buf_output.size() + res_message.iv_size();
                    char *wbuf = new char[final_size];
                    std::memcpy(wbuf, buf_output.c_str(), buf_output.size());
                    std::memcpy(wbuf + buf_output.size(), res_message.iv, res_message.iv_size());

                    chat_message_header res_header(final_size, 
                        chat_message_header::MT_REGISTER);

                    auto res_hbuf = res_header.serialization();

                    do_write_header(res_hbuf);
                    do_write(wbuf, res_header.body_size);

                    delete[] res_buf;
                    delete[] wbuf;
                    delete[] res_hbuf;
                }
                
                break;
            }
            case chat_message_header::MT_CONFIRMUINFO:
            {
                unsigned char iv[CryptoPP::AES::DEFAULT_BLOCKSIZE];

                std::memcpy(iv, r_buf + (header_.body_size
                    - CryptoPP::AES::DEFAULT_BLOCKSIZE),
                        CryptoPP::AES::DEFAULT_BLOCKSIZE);

                char pure_body[header_.body_size - CryptoPP::AES::DEFAULT_BLOCKSIZE];
                std::memcpy(pure_body, r_buf, header_.body_size - CryptoPP::AES::DEFAULT_BLOCKSIZE);

                std::string enc_body_data(pure_body,
                    header_.body_size - CryptoPP::AES::DEFAULT_BLOCKSIZE);
                std::string body_data;

                cproc_.DecryptWithAES(enc_body_data, body_data, iv);

                cm_confirm_uinfo_message message(body_data.c_str());

                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<int> dis(0, 255);
                unsigned char res_iv[CryptoPP::AES::DEFAULT_BLOCKSIZE];

                for (int i = 0; i < sizeof(iv); i++)
                {
                    res_iv[i] = dis(gen);
                }

                std::string requested_id(message.id);
                std::string requested_name(message.name);
                std::string requested_email(message.email);

                //When reqeusted information exists
                if(account_proc::instance().exist_uinfo(requested_id, requested_name, requested_email))
                {
                    cm_confirm_uinfo_message res_message(message.id, message.name, message.email);

                    res_message.set_iv(res_iv);
                    auto res_buf = res_message.serialization_except_iv();

                    std::string str_buf(res_buf, res_message.size() - res_message.iv_size());
                    std::string buf_output;

                    cproc_.EncryptWithAES(str_buf, buf_output, res_message.iv);

                    std::size_t final_size = buf_output.size() + res_message.iv_size();
                    char* wbuf = new char[final_size];
                    std::memcpy(wbuf, buf_output.c_str(), buf_output.size());
	                std::memcpy(wbuf + buf_output.size(), res_message.iv, res_message.iv_size());

                    chat_message_header res_header(final_size, 
                        chat_message_header::MT_CONFIRMUINFO);

                    auto res_hbuf = res_header.serialization();

                    do_write_header(res_hbuf);
                    do_write(wbuf, res_header.body_size);

                    delete[] res_buf;
                    delete[] wbuf;
                    delete[] res_hbuf;

                    is_change_passwd_granted = true;
                }
                else
                {
                    cm_confirm_uinfo_message res_message;

                    res_message.set_iv(res_iv);
                    auto res_buf = res_message.serialization_except_iv();

                    std::string str_buf(res_buf, res_message.size() - res_message.iv_size());
                    std::string buf_output;

                    cproc_.EncryptWithAES(str_buf, buf_output, res_message.iv);

                    std::size_t final_size = buf_output.size() + res_message.iv_size();
                    char *wbuf = new char[final_size];
                    std::memcpy(wbuf, buf_output.c_str(), buf_output.size());
                    std::memcpy(wbuf + buf_output.size(), res_message.iv, res_message.iv_size());

                    chat_message_header res_header(final_size, 
                        chat_message_header::MT_CONFIRMUINFO);

                    auto res_hbuf = res_header.serialization();

                    do_write_header(res_hbuf);
                    do_write(wbuf, res_header.body_size);

                    delete[] res_buf;
                    delete[] wbuf;
                    delete[] res_hbuf;
                }
                break;
            }
            case chat_message_header::MT_CHANGEPASSWORD:
            {
                //Invalid change password try
                if(!is_change_passwd_granted)
                {
                    is_reading_ = false;
                    delete[] r_buf;
                    remove_from_room();
                    do_close();
                    return;
                }

                unsigned char iv[CryptoPP::AES::DEFAULT_BLOCKSIZE];

                std::memcpy(iv, r_buf + (header_.body_size
                    - CryptoPP::AES::DEFAULT_BLOCKSIZE),
                        CryptoPP::AES::DEFAULT_BLOCKSIZE);

                 char pure_body[header_.body_size - CryptoPP::AES::DEFAULT_BLOCKSIZE];
                std::memcpy(pure_body, r_buf, header_.body_size - CryptoPP::AES::DEFAULT_BLOCKSIZE);

                std::string enc_body_data(pure_body,
                    header_.body_size - CryptoPP::AES::DEFAULT_BLOCKSIZE);
                std::string body_data;

                cproc_.DecryptWithAES(enc_body_data, body_data, iv);

                cm_change_password_message message(body_data.c_str());

                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<int> dis(0, 255);
                unsigned char res_iv[CryptoPP::AES::DEFAULT_BLOCKSIZE];

                for (int i = 0; i < sizeof(iv); i++)
                {
                    res_iv[i] = dis(gen);
                }

                std::string requested_id(message.id);
                std::string requested_password(message.password);
                std::string hashed_req_password;

                cproc_.ToHash(requested_password, hashed_req_password);

                //Processed request successfully
                if(!account_proc::instance().change_password(requested_id, hashed_req_password))
                {
                    cm_change_password_message res_message(message.id, message.password);

                    res_message.set_iv(res_iv);
                    auto res_buf = res_message.serialization_except_iv();

                    std::string str_buf(res_buf, res_message.size() - res_message.iv_size());
                    std::string buf_output;

                    cproc_.EncryptWithAES(str_buf, buf_output, res_message.iv);

                    std::size_t final_size = buf_output.size() + res_message.iv_size();
                    char* wbuf = new char[final_size];
                    std::memcpy(wbuf, buf_output.c_str(), buf_output.size());
	                std::memcpy(wbuf + buf_output.size(), res_message.iv, res_message.iv_size());

                    chat_message_header res_header(final_size, 
                        chat_message_header::MT_CHANGEPASSWORD);

                    auto res_hbuf = res_header.serialization();

                    do_write_header(res_hbuf);
                    do_write(wbuf, res_header.body_size);

                    delete[] res_buf;
                    delete[] wbuf;
                    delete[] res_hbuf;
                }
                else
                {
                    cm_change_password_message res_message;

                    res_message.set_iv(res_iv);
                    auto res_buf = res_message.serialization_except_iv();

                    std::string str_buf(res_buf, res_message.size() - res_message.iv_size());
                    std::string buf_output;

                    cproc_.EncryptWithAES(str_buf, buf_output, res_message.iv);

                    std::size_t final_size = buf_output.size() + res_message.iv_size();
                    char *wbuf = new char[final_size];
                    std::memcpy(wbuf, buf_output.c_str(), buf_output.size());
                    std::memcpy(wbuf + buf_output.size(), res_message.iv, res_message.iv_size());

                    chat_message_header res_header(final_size, 
                        chat_message_header::MT_CHANGEPASSWORD);

                    auto res_hbuf = res_header.serialization();

                    do_write_header(res_hbuf);
                    do_write(wbuf, res_header.body_size);

                    delete[] res_buf;
                    delete[] wbuf;
                    delete[] res_hbuf;
                }
                
                is_change_passwd_granted = false;
                break;
            }

            default:
            {
                std::cout << "invalid read\n";
                std::ofstream oss;
                oss.open("invalid_read.txt", std::ios::app);
                oss.write(id_.c_str(), id_.size());
                oss.write("\n", 1);
                oss.close();
                delete[] r_buf;
                is_reading_ = false;
                remove_from_room();
                do_close();
                return;
            }
        }

        delete[] r_buf;
        do_read_header();

        return;
    }
    else
    {
        is_reading_ = false;
        remove_from_room();
        do_close();

        std::cerr << "read_packet_proc error: " << std::endl << ec.message() << std::endl;
        return;
    }
}

void chat_session::write_header_packet_proc(boost::system::error_code ec, std::size_t length)
{
    if(!ec)
    {
        if(wtask_queue_.empty())
        {
            clear_wtask_queue();
            remove_from_room();
            do_close();
            return;
        }
        
        chat_message_header front_header(wtask_queue_.front().task_data);
        if(!front_header.is_valid())
        {
            std::cout << "write header is invalid\n";
            std::cout << "body_size=" << front_header.body_size << ", message_type="
                << front_header.message_type << ", left write=" << wtask_queue_.size() <<"\n";
            clear_wtask_queue();
            remove_from_room();
            do_close();
            return;
        }
        wtask_queue_.front().clear_data();
        wtask_queue_.pop();
        if(!wtask_queue_.empty())
        {
            auto next_ttype = wtask_queue_.front().task_type;

            if(next_ttype == wtask::TTYPE_BODY)
            {
                boost::asio::async_write(socket_,
                    boost::asio::buffer(wtask_queue_.front().task_data, front_header.body_size),
                    boost::bind(&chat_session::write_packet_proc, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
            }
            else
            {
                std::cout << "writing is incorrect\n";
                clear_wtask_queue();
                remove_from_room();
                do_close();
                return;
            }
        }
        else
        {
            if(!is_available())
            {
                clear_wtask_queue();
                remove_from_room();
                do_close();
                return;
            }
        }
        
        return;
    }
    else
    {
        clear_wtask_queue();
        remove_from_room();
        do_close();
        std::cout << "write_header_packet_proc error:" << ec.message() << std::endl;
        return;
    }
}

void chat_session::write_packet_proc(boost::system::error_code ec, std::size_t length)
{
    if(!ec)
    {
        if(wtask_queue_.empty())
        {
            clear_wtask_queue();
            remove_from_room();
            do_close();
            return;
        }

        wtask_queue_.front().clear_data();
        wtask_queue_.pop();
        if(!wtask_queue_.empty())
        {
            auto next_ttype = wtask_queue_.front().task_type;

            if(next_ttype == wtask::TTYPE_HEADER)
            {
                boost::asio::async_write(socket_,
                    boost::asio::buffer(wtask_queue_.front().task_data, header_size),
                    boost::bind(&chat_session::write_header_packet_proc, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
            }
            else
            {
                std::cout << "writing is incorrect\n";
                clear_wtask_queue();
                remove_from_room();
                do_close();
                return;
            }
        }
        else
        {
            if(!is_available())
            {
                clear_wtask_queue();
                remove_from_room();
                do_close();
                return;
            }
        }

        return;
    }
    else
    {
        clear_wtask_queue();
        remove_from_room();
        do_close();
        std::cout << "write_packet_proc error:" << ec.message() << std::endl;
        return;
    }
}

void chat_session::do_read_header()
{
    if(!is_available())
    {
        is_reading_ = false;
        remove_from_room();
        do_close();
        return;
    }

    std::memset(this->read_header_buffer_, 0, header_size);

    boost::asio::async_read(socket_,
        boost::asio::buffer(this->read_header_buffer_, header_size),
        boost::bind(&chat_session::read_header_packet_proc, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));

    is_reading_ = true;

    return;
}

void chat_session::do_read(std::size_t body_length)
{
    if(!is_available())
    {
        is_reading_ = false;
        remove_from_room();
        do_close();
        return;
    }

    std::memset(this->read_buffer_, 0, max_buf_size);

    std::size_t body_len = body_length;
    if(body_length == 0)
    {
        body_len = max_buf_size;
        socket_.is_open();
    }

    boost::asio::async_read(socket_,
        boost::asio::buffer(this->read_buffer_, body_len),
        boost::bind(&chat_session::read_packet_proc, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));

    is_reading_ = true;
    
    return;
}

void chat_session::do_write_header(const char* str, bool im)
{
    if(!is_available())
    {
        clear_wtask_queue();
        do_close();
        return;
    }

    bool is_writing = !wtask_queue_.empty();
    char *wbuf = new char[header_size];
    std::memset(wbuf, 0, header_size);
    std::memcpy(wbuf, str, header_size);
    wtask_queue_.emplace(wtask(wtask::TTYPE_HEADER, wbuf));

    if(!is_writing)
    {
        boost::asio::async_write(socket_,
            boost::asio::buffer(wtask_queue_.front().task_data, header_size),
            boost::bind(&chat_session::write_header_packet_proc, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }

    return;
}

void chat_session::do_write(const char *str, std::size_t length)
{
    if(!is_available())
    {
        clear_wtask_queue();
        do_close();
        return;
    }
 
    bool is_writing = !wtask_queue_.empty();
    char *wbuf = new char[length];
    std::memset(wbuf, 0, length);
    std::memcpy(wbuf, str, length);
    wtask_queue_.emplace(wtask(wtask::TTYPE_BODY, wbuf));

    if(!is_writing)
    {
        boost::asio::async_write(socket_,
            boost::asio::buffer(wtask_queue_.front().task_data, length),
            boost::bind(&chat_session::write_packet_proc, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }

    return;
}

void chat_session::do_close()
{
    if(socket_.is_open() && is_available()) {
        boost::system::error_code cls_ec;
        socket_.close(cls_ec);
    }

    if(wtask_queue_.empty() && !is_reading_)
    {
        is_run_ = false;
    }
    
    return;
}

void chat_session::store_ip(const std::string &ip)
{
    stored_ip_ = ip;
}

void chat_session::store_port(const unsigned short port)
{
    stored_port_ = port;
}

void chat_session::clear_wtask_queue()
{
    while(!wtask_queue_.empty())
    {
        auto task = wtask_queue_.front();
        task.clear_data();
        wtask_queue_.pop();
    }
}

long long chat_session::alloc_room_number()
{
    long long ret = 0;

    room_set_mutex_.lock();
    auto it = room_set_.rbegin();
    if(it == room_set_.rend()) ret = 1;
    else if((*it)->room_number() >= max_room_size) ret = -1;
    else ret = (*it)->room_number() + 1;
    room_set_mutex_.unlock();
    
    return ret;
}

bool session_compare::operator()(const session_ptr& left, const session_ptr& right) const
{
    bool ip_comp = (*left).ip() == (*right).ip();
    if(!ip_comp)
    {
        return (*left).ip() < (*right).ip();
    }
    else
    {
        return (*left).port() < (*right).port();
    }
}