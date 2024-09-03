#ifndef THREADEDSET
#define THREADEDSET

#include <set>
#include <memory>
#include <mutex>
#include <thread>
#include <functional>

template<typename T, typename comp = std::less<T>>
class ThreadedSet
{
typedef typename std::set<T>::iterator iterator;
typedef typename std::set<T>::const_iterator const_iterator;
typedef typename std::set<T>::reverse_iterator reverse_iterator;
typedef typename std::pair<iterator, bool> insertion_result;
typedef typename std::set<T>::size_type size_type;

public:
    ThreadedSet()
        : worker_thread_(nullptr)
    {

    }

    ~ThreadedSet()
    {
        if(!data_set_.empty())
        {
            for(auto it = data_set_.begin(); it != data_set_.end();)
                it = data_set_.erase(it);
        }

        if(worker_thread_ != nullptr)
        {
            if(worker_thread_->joinable())
                worker_thread_->join();
            
            delete worker_thread_;
        }
    }

    iterator begin()
    {
        iterator ret;
        work([this, &ret]
        {
            ret = data_set_.begin();
        });

        std::lock_guard<std::mutex> lock(thread_mutex_);
        if(worker_thread_->joinable()) worker_thread_->join();
        return ret;
    }

    iterator begin() const
    {
        iterator ret;
        work([this, &ret]
        {
            ret = data_set_.begin();
        });

        std::lock_guard<std::mutex> lock(thread_mutex_);
        if(worker_thread_->joinable()) worker_thread_->join();
        return ret;
    }

    iterator end()
    {
        iterator ret;
        work([this, &ret]
        {
            ret = data_set_.end();
        });

        std::lock_guard<std::mutex> lock(thread_mutex_);
        if(worker_thread_->joinable()) worker_thread_->join();
        return ret;
    }

    iterator end() const
    {
        iterator ret;
        work([this, &ret]
        {
            ret = data_set_.end();
        });

        std::lock_guard<std::mutex> lock(thread_mutex_);
        if(worker_thread_->joinable()) worker_thread_->join();
        return ret;
    }

    reverse_iterator rbegin()
    {
        reverse_iterator ret;
        work([this, &ret]
        {
            ret = data_set_.rbegin();
        });

        std::lock_guard<std::mutex> lock(thread_mutex_);
        if(worker_thread_->joinable()) worker_thread_->join();
        return ret;
    }

    reverse_iterator rbegin() const
    {
        reverse_iterator ret;
        work([this, &ret]
        {
            ret = data_set_.rbegin();
        });

        std::lock_guard<std::mutex> lock(thread_mutex_);
        if(worker_thread_->joinable()) worker_thread_->join();
        return ret;
    }

    reverse_iterator rend()
    {
        reverse_iterator ret;
        work([this, &ret]
        {
            ret = data_set_.rend();
        });

        std::lock_guard<std::mutex> lock(thread_mutex_);
        if(worker_thread_->joinable()) worker_thread_->join();
        return ret;
    }

    reverse_iterator rend() const
    {
        reverse_iterator ret;
        work([this, &ret]
        {
            ret = data_set_.rend();
        });

        std::lock_guard<std::mutex> lock(thread_mutex_);
        if(worker_thread_->joinable()) worker_thread_->join();
        return ret;
    }

    insertion_result emplace(const T& data)
    {
        insertion_result ret = std::pair<iterator, bool>(data_set_.begin(), true);
        work([this, &ret, data]()
        {
            ret = data_set_.emplace(data);
            
        });

        std::lock_guard<std::mutex> lock(thread_mutex_);
        if(worker_thread_->joinable()) worker_thread_->join();
        return ret;
    }

    bool empty()
    {
        bool ret;
        work([this, &ret]()
        {
            ret = data_set_.empty();
        });

        std::lock_guard<std::mutex> lock(thread_mutex_);
        if(worker_thread_->joinable()) worker_thread_->join();
        return ret;
    }

    bool empty() const
    {
        bool ret;
        work([this, &ret]()
        {
            ret = data_set_.empty();
        });

        std::lock_guard<std::mutex> lock(thread_mutex_);
        if(worker_thread_->joinable()) worker_thread_->join();
        return ret;
    }

    iterator erase(iterator position)
    {
        iterator ret;
        work([this, &ret, position]()
        {
            ret = data_set_.erase(position);
        });

        std::lock_guard<std::mutex> lock(thread_mutex_);
        if(worker_thread_->joinable()) worker_thread_->join();
        return ret;
    }

    iterator erase(iterator first, iterator last)
    {
        iterator ret;
        work([this, &ret, first, last]()
        {
            ret = data_set_.erase(first, last);
        });

        std::lock_guard<std::mutex> lock(thread_mutex_);
        if(worker_thread_->joinable()) worker_thread_->join();
        return ret;
    }

    size_type size()
    {
        size_type ret;
        work([this, &ret]()
        {
            ret = data_set_.size();
        });

        std::lock_guard<std::mutex> lock(thread_mutex_);
        if(worker_thread_->joinable()) worker_thread_->join();
        return ret;
    }

    iterator find(const T& data)
    {
        iterator ret;
        work([this, &ret, data]
        {
            ret = data_set_.find(data);
        });

        std::lock_guard<std::mutex> lock(thread_mutex_);
        if(worker_thread_->joinable()) worker_thread_->join();
        return ret;
    }

    iterator find(const T& data) const
    {
        iterator ret;
        work([this, &ret, data]
        {
            ret = data_set_.find(data);
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
            worker_thread_ = new std::thread([this, works]
            {
                std::lock_guard<std::mutex> lock(data_mutex_);
                works();
            });
        }
        else
        {
            if(worker_thread_->joinable())
                worker_thread_->join();

            delete worker_thread_;

            worker_thread_ = new std::thread([this, works]
            {
                std::lock_guard<std::mutex> lock(data_mutex_);
                works(); 
            });
        }
    }
private:
    std::set<T, comp> data_set_;
    std::mutex data_mutex_;

    std::thread* worker_thread_;
    std::mutex thread_mutex_;
};

#endif