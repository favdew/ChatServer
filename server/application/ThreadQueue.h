#ifndef THREADQUEUE
#define THREADQUEUE

#include <queue>
#include <mutex>
#include <thread>
#include <memory>
#include <functional>

template<typename T>
class ThreadQueue
{
public:
    ThreadQueue()
        : worker_thread_(nullptr)
    {

    }

    bool is_empty()
    {
        std::lock_guard<std::mutex> lock(data_mutex_);
        return data_queue_.empty();
    }

    void push(T data)
    {
        work([this, data]()
        {
            data_queue_.push(data);
        });
    }

    void pop()
    {
        work([this]()
        {
            data_queue_.pop();
        });
    }

    T front()
    {
        T ret;
        work([this, &ret]()
        {
            ret = data_queue_.front();
        });

        std::lock_guard<std::mutex> lock(thread_mutex_);
        if(worker_thread_->joinable()) worker_thread_->join();
        return ret;
    }

    T back()
    {
        T ret;
        work([this, &ret]
        {
            ret = data_queue_.back();
        });

        std::lock_guard<std::mutex> lock(thread_mutex_);
        if(worker_thread_->joinable()) worker_thread_->join();
        return ret;
    }

    std::size_t size()
    {
        std::size_t ret;
        work([this, &ret]
        {
            ret = data_queue_.size();
        });

        std::lock_guard<std::mutex> lock(thread_mutex_);
        if(worker_thread_->joinable()) worker_thread_->join();
        return ret;
    }

private:
    void work(std::function<void()> works)
    {
        std::lock_guard<std::mutex> lock(thread_mutex_);
        if(worker_thread_ == nullptr)
        {
            worker_thread_ = new std::thread([this, works]() { 
                std::lock_guard<std::mutex> lock(data_mutex_);
                works();
            });
        }
        else
        {
            if(worker_thread_->joinable())
                worker_thread_->join();

            delete worker_thread_;

            worker_thread_ = new std::thread([this, works]()
            {
                std::lock_guard<std::mutex> lock(data_mutex_);
                works();
            });
        }
        
    }

private:
    
    std::queue<T> data_queue_;
    std::mutex data_mutex_;

    std::thread *worker_thread_;
    std::mutex thread_mutex_;
};

#endif