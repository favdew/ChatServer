#ifndef CONNECTOR
#define CONNECTOR

#include <iostream>
#include <string>
#include <fstream>

#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

//Singleton class
class connector
{
public:
    static connector& instance();

    bool execute(const sql::SQLString &query);
    sql::ResultSet *executeQuery(const sql::SQLString &query);

private:
    connector();

private:
    static connector* instance_;

    sql::Driver *driver_;
    sql::Connection *connection_;
    sql::Statement *statement_;

    const std::string connection_config_path_;
};

#endif