#include "chat_server.h"

chat_server::chat_server(boost::asio::io_context &io_context, short port)
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
{
    std::cout << "chat_server has been created\n";
    max_user_num_ = 0;
    rpacket_num_ = 0;
    cleaner_thread_ = std::thread([this](){
        while(true)
        {
            sessions_mutex_.lock();
            for(auto it = sessions_.begin(); it != sessions_.end();)
            {
                if(!(*it)->is_socket_open() && !(*it)->is_run())
                {
                    std::cout << "socket is not opened\n";

                    rpacket_num_ += (*it)->received_packet_num_;

                    time_info ti(std::chrono::system_clock::now());
                    Logger::instance().record_disconnection_info(
                        (*it)->get_stored_ip(), (*it)->get_stored_port(),
                        ti.strtime());

                    it = sessions_.erase(it);

                    std::cout << "max user=" << max_user_num_ << std::endl;
                    std::cout << "left session=" << sessions_.size() << std::endl;
                    std::cout << "rpacket num=" << rpacket_num_ << "\n";
                    continue;
                }

                if(!(*it)->is_available() && !(*it)->is_run())
                {
                    std::cout << "socket is not available\n";

                    rpacket_num_ += (*it)->received_packet_num_;

                    time_info ti(std::chrono::system_clock::now());
                    Logger::instance().record_disconnection_info(
                        (*it)->get_stored_ip(), (*it)->get_stored_port(),
                        ti.strtime());

                    it = sessions_.erase(it);

                    std::cout << "max user=" << max_user_num_ << std::endl;
                    std::cout << "left session=" << sessions_.size() << std::endl;
                    std::cout << "rpacket num=" << rpacket_num_ << "\n";
                    continue;
                }
                
                it++;
            }
            sessions_mutex_.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    });
    do_accept();
}

chat_server::~chat_server()
{
}

std::size_t chat_server::session_count()
{
    sessions_mutex_.lock();
    auto ret = sessions_.size();
    sessions_mutex_.unlock();
    return ret;
}

std::size_t chat_server::room_count()
{
    room_set_mutex_.lock();
    auto ret = room_set_.size();
    room_set_mutex_.unlock();
    return ret;
}

std::shared_ptr<chat_session> chat_server::get_session(std::string ip, unsigned short port)
{
    sessions_mutex_.lock();
    std::shared_ptr<chat_session> ret = nullptr;
    for(auto it = sessions_.begin(); it != sessions_.end(); it++)
    {
        if((*it)->ip() == ip && (*it)->port() == port)
        {
            ret = (*it);
            break;
        }
    }
    sessions_mutex_.unlock();

    return ret;
}

std::vector<std::shared_ptr<chat_session>> chat_server::get_sessions()
{
    std::vector<std::shared_ptr<chat_session>> ret;

    sessions_mutex_.lock();
    for(auto it = sessions_.begin(); it != sessions_.end(); it++)
    {
        ret.push_back((*it));
    }
    sessions_mutex_.unlock();

    return ret;
}

void chat_server::destroy_session_in_session(std::shared_ptr<chat_session> session)
{
    sessions_mutex_.lock();
    auto it = sessions_.find(session);
    if(it != sessions_.end())
    {
        std::cout << "[Session delete] ip=" << (*it)->ip() << ", port=" <<
            (*it)->port() << std::endl;
        sessions_.erase(it);
        std::cout << "left session=" << sessions_.size() << std::endl;
    }
    sessions_mutex_.unlock();
}

void chat_server::safety_exit()
{
    sessions_mutex_.lock();
    auto it = sessions_.begin();
    while(!sessions_.empty())
    {
        if((*it)->is_socket_open())
            (*it)->close();

        it = sessions_.erase(it);
    }
    sessions_mutex_.unlock();
}

bool chat_server::exist_user_for_id(const std::string& id)
{
    bool ret = false;
    sessions_mutex_.lock();

    for(auto it = sessions_.begin(); it != sessions_.end(); it++)
    {
        if((*it)->id() == id)
        {
            ret = true;
            break;
        }
    }

    sessions_mutex_.unlock();

    return ret;
}

void chat_server::do_accept()
{
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket)
        {
            if(!ec)
            {
                struct timeval tv;
                tv.tv_sec  = timeout_milli / 1000;
                tv.tv_usec = timeout_milli % 1000;
                int rc;
                int yes = 1;
                bool is_setted = true;

                rc = setsockopt(socket.native_handle(), SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(int));
                if(rc < 0) is_setted = false;

                int idle = 3;

                rc = setsockopt(socket.native_handle(), IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(int));
                if(rc < 0) is_setted = false;

                int interval = 2;
 
                rc = setsockopt(socket.native_handle(), IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(int));
                if(rc < 0) is_setted = false;
                int result_interval;
                socklen_t sz_interval;
                getsockopt(socket.native_handle(), IPPROTO_TCP, TCP_KEEPINTVL, &result_interval, &sz_interval);

                int maxpkt = 4;

                rc = setsockopt(socket.native_handle(), IPPROTO_TCP, TCP_KEEPCNT, &maxpkt, sizeof(int));
                if(rc < 0) is_setted = false;
                
                if(is_setted)
                {
                    auto session = std::make_shared<chat_session>(std::move(socket), room_set_, room_set_mutex_);
                    session->set_chat_server_interfafce((chat_server_interface *)this);
                    session->start();

                    sessions_mutex_.lock();
                    sessions_.emplace(session);
                    max_user_num_ = std::max(sessions_.size(), max_user_num_);
                    sessions_mutex_.unlock();

                    time_info ti(std::chrono::system_clock::now());
                    Logger::instance().record_connection_info(session->ip(),
                        session->port(), ti.strtime());
                }
                else
                {
                    boost::system::error_code close_ec;
                    socket.close(ec);
                }
            }
            else
            {  
                if(socket.is_open())
                {
                    boost::system::error_code cls_ec;
                    socket.close(ec);

                    if(!ec)
                    {
                        time_info ti(std::chrono::system_clock::now());
                        Logger::instance().record_connection_error_info(
                            socket.remote_endpoint().address().to_string(),
                            socket.remote_endpoint().port(),
                            ti.strtime());
                    }
                    else
                    {
                        time_info ti(std::chrono::system_clock::now());
                        Logger::instance().record_connection_error_info(
                            "", 0, ti.strtime());
                    }
                    
                }
                else
                {
                    time_info ti(std::chrono::system_clock::now());
                    Logger::instance().record_connection_error_info(
                        "", 0, ti.strtime());
                }
            }

            do_accept();
        }
    );
}

std::string chat_server::get_connect_success_log(const tcp::socket& socket)
{
    std::string ret;

    boost::system::error_code ec;
    auto endpoint = socket.remote_endpoint(ec);
    if(!ec)
    {
        ret += "[Log] Successful Connection\n";
        ret += "[Info]\n";
        ret += "[IP]=";
        ret += socket.remote_endpoint().address().to_string(); ret += ":";
        ret += std::to_string(socket.remote_endpoint().port());
        ret += "\n";
    }
    else
    {
        ret = "remote endpoint error: " + ec.message() + "\n";
    }
    
    return ret;
}