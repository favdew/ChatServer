#include "connector.h"

connector *connector::instance_ = nullptr;

connector& connector::instance()
{
    if(instance_ == nullptr)
    {
        instance_ = new connector();
    }

    return *instance_;
}

bool connector::execute(const sql::SQLString &query)
{
    bool ret = statement_->execute(query);
    connection_->commit();
    return ret;
}

sql::ResultSet *connector::executeQuery(const sql::SQLString &query)
{
    return statement_->executeQuery(query);
}

connector::connector()
    : connection_config_path_("config/connection.conf")
{
    std::ifstream ifs;
    ifs.open(connection_config_path_);
    if(ifs.is_open())
    {
        std::string url, user, password, scheme;
        std::getline(ifs, url);

        if(url.size() <= 0)
        {
            std::cerr << "invalid database url\n";
            exit(1);
        }
        else if(ifs.eof())
        {
            std::cerr << "Invalid connection.conf file\n";
            std::cerr << "[URL]\\n[Username]\\n[Password]\n";
            exit(1);
        }
        
        std::getline(ifs, user);
        if(user.size() <= 0)
        {
            std::cerr << "invalid database user\n";
            exit(1);
        }
        else if(ifs.eof())
        {
            std::cerr << "Invalid connection.conf file\n";
            std::cerr << "[URL]\\n[Username]\\n[Password]\n";
            exit(1);
        }

        std::getline(ifs, password);
        if(password.size() <= 0)
        {
            std::cerr << "invalid database password\n";
            exit(1);
        }
         else if(ifs.eof())
        {
            std::cerr << "Invalid connection.conf file\n";
            std::cerr << "[URL]\\n[Username]\\n[Password]\n";
            exit(1);
        }

        std::getline(ifs, scheme);
        if(scheme.size() <= 0)
        {
            std::cerr << "invalid database scheme\n";
            exit(1);
        }

        driver_ = get_driver_instance();

        connection_ = driver_->connect(url, user, password);

        if(!connection_->isValid())
        {
            std::cerr << "connection has been failed or is invalid\n";
            exit(1);
        }

        connection_->setSchema(scheme);
        connection_->setTransactionIsolation(sql::TRANSACTION_READ_COMMITTED);
        connection_->setAutoCommit(false);

        statement_ = connection_->createStatement();
    }
    else
    {
        std::cerr << "Can not open connection.conf";
        exit(1);
    }
    
}