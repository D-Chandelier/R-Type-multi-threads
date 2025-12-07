#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

template <typename T>
class ThreadSafeQueue
{
public:
    void push(const T &v)
    {
        {
            std::lock_guard<std::mutex> lk(mutex_);
            q_.push(v);
        }
        cond_.notify_one();
    }

    // try_pop non bloquant
    bool try_pop(T &out)
    {
        std::lock_guard<std::mutex> lk(mutex_);
        if (q_.empty())
            return false;
        out = std::move(q_.front());
        q_.pop();
        return true;
    }

    // pop bloquant (optionnel)
    T wait_and_pop()
    {
        std::unique_lock<std::mutex> lk(mutex_);
        cond_.wait(lk, [this]
                   { return !q_.empty(); });
        T v = std::move(q_.front());
        q_.pop();
        return v;
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lk(mutex_);
        return q_.empty();
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable cond_;
    std::queue<T> q_;
};
