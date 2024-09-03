#include "connector.h"

//Singleton Class
class account_proc
{
public:
    static account_proc &instance();

    int login(const std::string &id, const std::string &passwd);
    bool id_exist(const std::string &id);
    void register_user(const std::string& id, const std::string& password, const std::string& name, const std::string& email);
    bool exist_uinfo(const std::string &id, const std::string &name, const std::string &email);
    bool change_password(const std::string &id, const std::string &password);
private:
    account_proc();
    static account_proc* instance_;
};