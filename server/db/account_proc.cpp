#include "account_proc.h"

account_proc *account_proc::instance_ = nullptr;

account_proc &account_proc::instance()
{
    if(instance_ == nullptr)
    {
        instance_ = new account_proc();
    }

    return *instance_;
}

int account_proc::login(const std::string &id, const std::string &passwd)
{
    auto db_connector = connector::instance();
    auto result = db_connector.executeQuery(
        "select * from chat_user_account where user_id ='" +
        id + "' and user_passwd='" + passwd + "'"
    );
    bool is_exist = result->next();
    delete result;
    if(!is_exist) 
        return -1;
    else
        return 0;
}

bool account_proc::id_exist(const std::string &id)
{
    auto db_connector = connector::instance();
    auto result = db_connector.executeQuery(
        "select user_id from chat_user_account where user_id = '" +
        id + "'"
    );

    bool is_exist = result->next();
    delete result;
    if(is_exist)
        return true;
    else
        return false;
}

void account_proc::register_user(const std::string& id, const std::string& password, const std::string& name, const std::string& email)
{
    auto db_connector = connector::instance();
    db_connector.execute("insert into chat_user_account values('"
        + id + "', '" +  password + "', '" + name + "', '" + email + "')");
}

bool account_proc::exist_uinfo(const std::string &id, const std::string &name, const std::string &email)
{
    auto db_connector = connector::instance();
    auto result = db_connector.executeQuery("select * from chat_user_account "
        "where user_id = '" + id + "' and user_name = '" + name + "' and "
        "user_email = '" + email + "'");

    bool is_exist = result->next();
    delete result;
    if(is_exist)
        return true;
    else
        return false;
}

bool account_proc::change_password(const std::string &id, const std::string &password)
{
    auto db_connector = connector::instance();
    return db_connector.execute("update chat_user_account "
        "set user_passwd = '" + password + "' where user_id = '" + id + "'");
}

account_proc::account_proc()
{
    
}