#ifndef LOGGER
#define LOGGER

#include <cstring>
#include <ctime>
#include <fstream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <utility>

#define TIME_INFO_BUF_SIZE 20

#define CONNECTION_LOG_PATH "log/conn/"
#define CONNECTION_FILE_NAME "_conn_log.csv"
#define CONNECTION_ERROR_LOG_PATH "log/conn/"
#define CONNECTION_ERROR_FILE_NAME "_conn_error_log.csv"
#define DISCONNECTION_LOG_PATH "log/disconn/"
#define DISCONNECTION_FILE_NAME "_disconn_log.csv"

class time_info
{
public:
    time_info(unsigned short year, unsigned char month, unsigned char day, unsigned char hour, unsigned char minute, unsigned char second);
    time_info(std::chrono::_V2::system_clock::time_point time_point);

    std::string strymd();
    std::string strtime();

public:
    unsigned short year_;
    unsigned char month_;
    unsigned char day_;
    unsigned char hour_;
    unsigned char minute_;
    unsigned char second_;
};

//Singleton class
class Logger
{
//first member is path written
//second memeber is data written
typedef std::pair<std::string, std::string> work_data;

public:
    static Logger &instance();
    ~Logger();

    void record_connection_info(const std::string &ip, const unsigned short port, const std::string &date);
    void record_connection_error_info(const std::string &ip, const unsigned short port, const std::string &date);
    void record_disconnection_info(const std::string &ip, const unsigned short port, const std::string &date);

private:
    Logger();
    int log();

private:
    static Logger *instance_;
    
    std::thread *work_thread_;
    std::mutex work_thread_mutex_;
    std::queue<work_data> work_data_queue_;
    std::mutex work_data_queue_mutex_;
};

#endif