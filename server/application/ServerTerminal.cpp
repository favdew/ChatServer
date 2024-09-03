#include "ServerTerminal.h"

ServerTerminal::ServerTerminal(chat_server& server)
    : server_(server)
{

}

ServerTerminal::~ServerTerminal()
{

}

void ServerTerminal::order(std::string o)
{
    if(o.size() <= 0) return;

    std::size_t tok_pos = o.find(' ');
    if(tok_pos <= 0) return;

    std::string main_order = o.substr(0, tok_pos);

    //print number of session
    if(main_order == "scount")
    {
        std::cout << server_.session_count() << "\n";
    }
    //print IP and port of each session
    else if(main_order == "slist")
    {
        auto sessions = server_.get_sessions();

        for(auto it = sessions.begin(); it != sessions.end(); it++)
        {
            auto info = (*it)->get_kal_info();
            std::cout << "IP=" << (*it)->ip() << ", port=" << (*it)->port() <<
                ", is_run=" << (*it)->is_run() <<
                ", left write_num=" << (*it)->get_wq_size() <<
                ", received num=" << (*it)->received_packet_num_ <<
                ", is open=" << (*it)->is_socket_open() << 
                ", is_reading=" << (*it)->is_reading_ << 
                ", is_kal=" << info.is_kal <<
                ", idle=" << info.idle <<
                ", interval=" << info.interval << 
                ", maxpkt=" << info.maxpkt << 
                ", syncnt=" << (*it)->get_syncnt() << std::endl;
        }
    }
    //write to session
    else if(main_order == "write")
    {
        auto sessions = server_.get_sessions();
        (*sessions.begin())->write("a", 1);
    }
    else if(main_order == "close")
    {
        if(tok_pos == std::string::npos)
        {
            std::cout << "Invalid order: order must be ""close [IP] [port]""\n";
            return;
        }

        int argc;
        std::string argv[2];
        std::string next = o.substr(tok_pos + 1);
        for(argc = 0; argc < 2; argc++)
        {
            tok_pos = next.find(' ');
            argv[argc] = next.substr(0, tok_pos);
            next = next.substr(tok_pos + 1);
        }

        if(argc != 2)
        {
            std::cout << "Invalid order: order must be ""close [IP] [port]""\n";
            return;
        }

        std::cout << "IP[" << argv[0] << "]\n";
        std::cout << "PORT[" << argv[1] << "]\n";

        unsigned short port = (unsigned short)std::stoul(argv[1]);

        auto session = server_.get_session(argv[0], port);
        if(session == nullptr)
        {
            std::cout << "requested session doesn't exist\n";
            return;
        }
        else
        {
            std::cout << "requested session exists\n";
            session->close();
        }
    }
    else if(main_order == "rcount")
    {
        std::cout << server_.room_count() << "\n";
    }
    else if(main_order == "pkalinfo")
    {
        if(tok_pos == std::string::npos)
        {
            std::cout << "Invalid order: order must be ""close [IP] [port]""\n";
            return;
        }

        int argc;
        std::string argv[2];
        std::string next = o.substr(tok_pos + 1);
        for(argc = 0; argc < 2; argc++)
        {
            tok_pos = next.find(' ');
            argv[argc] = next.substr(0, tok_pos);
            next = next.substr(tok_pos + 1);
        }

        if(argc != 2)
        {
            std::cout << "Invalid order: order must be ""close [IP] [port]""\n";
            return;
        }

        unsigned short port = (unsigned short)std::stoul(argv[1]);

        auto session = server_.get_session(argv[0], port);
        if(session == nullptr)
        {
            std::cout << "requested session doesn't exist\n";
            return;
        }
        else
        {
            std::cout << "requested session exists\n";
            auto info = session->get_kal_info();

            std::cout << "ip=" << session->ip() << ", port=" << session->port()
                << ", is_kal=" << info.is_kal << ", idle=" << info.idle
                << ", interval=" << info.interval << ", maxpkt=" << info.maxpkt << std::endl;
        }
    }
    //Invliad order
    else
    {
        std::cout << "Invalid order\n";
    }
    
}