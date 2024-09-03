#include "Logger.h"

Logger *Logger::instance_ = nullptr;

time_info::time_info(unsigned short year, unsigned char month, unsigned char day,
    unsigned char hour, unsigned char minute, unsigned char second)
    : year_(year), month_(month), day_(day), hour_(hour), minute_(minute), second_(second)
{
}

time_info::time_info(std::chrono::_V2::system_clock::time_point time_point)
{
    auto tt = std::chrono::system_clock::to_time_t(time_point);
        auto tm = std::localtime(&tt);
        char buffer[TIME_INFO_BUF_SIZE];

        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm);

        char *y = std::strtok(buffer, "-");
        year_ = (unsigned short)(std::atoi(y));

        char *m = std::strtok(nullptr, "-");
        month_ = (unsigned char)(std::atoi(m));

        char *d = std::strtok(nullptr, " ");
        day_ = (unsigned char)(std::atoi(d));

        char *h = std::strtok(nullptr, ":");
        hour_ = (unsigned char)(std::atoi(h));

        char *mi = std::strtok(nullptr, ":");
        minute_ = (unsigned char)(std::atoi(mi));

        char *s = std::strtok(nullptr, " ");
        second_ = (unsigned char)(std::atoi(s));
}

std::string time_info::strymd()
{
    std::string result;

    result += std::to_string((unsigned long long)year_);
    result += "-";
    result += std::to_string((unsigned long long)month_);
    result += "-";
    result += std::to_string((unsigned long long)day_);

    return result;
}

std::string time_info::strtime()
{
    std::string result;

    result += std::to_string((unsigned long long)year_);
    result += "-";
    result += std::to_string((unsigned long long)month_);
    result += "-";
    result += std::to_string((unsigned long long)day_);
    result += " ";
    result += std::to_string((unsigned long long)hour_);
    result += ":";
    result += std::to_string((unsigned long long)minute_);
    result += ":";
    result += std::to_string((unsigned long long)second_);

    return result;
}

Logger &Logger::instance()
{
    if(instance_ == nullptr)
    {
        instance_ = new Logger();
    }

    return *instance_;
}

Logger::~Logger()
{
    work_thread_mutex_.lock();
    if(work_thread_ != nullptr)
    {
        if(work_thread_->joinable())
            work_thread_->join();
        delete work_thread_;
    }
    work_thread_mutex_.unlock();
}

void Logger::record_connection_info(const std::string &ip, const unsigned short port, const std::string &date)
{
    time_info ti(std::chrono::system_clock::now());
    std::string write_path = CONNECTION_LOG_PATH + ti.strymd() + CONNECTION_FILE_NAME;
    work_data wd;
    wd.first = write_path;
    wd.second = ip + "," + std::to_string((unsigned long long)port) + "," + date +
        "," + ti.strtime() + "\n";

    work_data_queue_mutex_.lock();
    if(!work_data_queue_.empty())
    {
        work_data_queue_.emplace(wd);
        work_data_queue_mutex_.unlock();
    }
    else
    {
        work_data_queue_.emplace(wd);
        work_data_queue_mutex_.unlock();

        work_thread_mutex_.lock();
        if(work_thread_ == nullptr)
        {
            work_thread_ = new std::thread([this, &write_path](){
                work_data_queue_mutex_.lock();
                while(!work_data_queue_.empty())
                    log();
                work_data_queue_mutex_.unlock();
            });
        }
        else
        {
            if(work_thread_->joinable())
                work_thread_->join();
            delete work_thread_;

            work_thread_ = new std::thread([this](){
                work_data_queue_mutex_.lock();
                while(!work_data_queue_.empty())
                    log();
                work_data_queue_mutex_.unlock();
            });
        }
        work_thread_mutex_.unlock();
    }
}

void Logger::record_connection_error_info(const std::string &ip, const unsigned short port, const std::string &date)
{
    time_info ti(std::chrono::system_clock::now());
    std::string write_path = CONNECTION_ERROR_LOG_PATH + ti.strymd() +
        CONNECTION_ERROR_FILE_NAME;
    work_data wd;
    wd.first = write_path;
    wd.second = ip + "," + std::to_string((unsigned long long)port) + "," + date +
        "," + ti.strtime() + "\n";

    work_data_queue_mutex_.lock();
    if(!work_data_queue_.empty())
    {
        work_data_queue_.emplace(wd);
        work_data_queue_mutex_.unlock();
    }
    else
    {
        work_data_queue_.emplace(wd);
        work_data_queue_mutex_.unlock();

        work_thread_mutex_.lock();
        if(work_thread_ == nullptr)
        {
            work_thread_ = new std::thread([this, &write_path](){
                work_data_queue_mutex_.lock();
                while(!work_data_queue_.empty())
                    log();
                work_data_queue_mutex_.unlock();
            });
        }
        else
        {
            if(work_thread_->joinable())
                work_thread_->join();
            delete work_thread_;

            work_thread_ = new std::thread([this](){
                work_data_queue_mutex_.lock();
                while(!work_data_queue_.empty())
                {
                    log();
                }
                work_data_queue_mutex_.unlock();
            });
        }
        work_thread_mutex_.unlock();
    }
}

void Logger::record_disconnection_info(const std::string &ip, const unsigned short port, const std::string &date)
{
    time_info ti(std::chrono::system_clock::now());
    std::string write_path = DISCONNECTION_LOG_PATH + ti.strymd() + 
        DISCONNECTION_FILE_NAME;
    work_data wd;
    wd.first = write_path;
    wd.second = ip + "," + std::to_string((unsigned long long)port) + "," + date +
        "," + ti.strtime() + "\n";

    work_data_queue_mutex_.lock();
    if(!work_data_queue_.empty())
    {
        work_data_queue_.emplace(wd);
        work_data_queue_mutex_.unlock();
    }
    else
    {
        work_data_queue_.emplace(wd);
        work_data_queue_mutex_.unlock();

        work_thread_mutex_.lock();
        if(work_thread_ == nullptr)
        {
            work_thread_ = new std::thread([this, &write_path](){
                work_data_queue_mutex_.lock();
                while(!work_data_queue_.empty())
                    log();
                work_data_queue_mutex_.unlock();
            });
        }
        else
        {
            if(work_thread_->joinable())
                work_thread_->join();
            delete work_thread_;

            work_thread_ = new std::thread([this](){
                work_data_queue_mutex_.lock();
                while(!work_data_queue_.empty())
                    log();
                work_data_queue_mutex_.unlock();
            });
        }
        work_thread_mutex_.unlock();
    }
}

Logger::Logger()
    : work_thread_(nullptr)
{

}

int Logger::log()
{
    int result = 0;

    auto data = work_data_queue_.front();
    std::ofstream log_stream;
    log_stream.open(data.first, std::ios::app);
    if(log_stream.is_open())
    {
        log_stream.write(data.second.c_str(), data.second.size());
        log_stream.close();
    }
    else
    {
        result = -1;
    }
    
    work_data_queue_.pop();

    return result;
}