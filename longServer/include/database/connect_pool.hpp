#pragma once
// 连接池

#include "utils/noncopyable.hpp"
#include <chrono>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <string_view>

namespace uranus::database
{
template<typename T>
class ConnectPool: public utils::Noncopyable
{
public:
    static ConnectPool<T> *get()
    {
        static ConnectPool<T> connPool;
        return &connPool;
    }

    void initalize(std::string_view conn, const int maxConn = 10, const int maxIdle = 5)
    {
        connect_ = conn;
        maxConn_ = maxConn;
        maxIdle_ = maxIdle;
    }

    auto get() -> std::shared_ptr<T>
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (pool_.empty()) {
            if (condition_.wait_for(lock, std::chrono::seconds(3)) == std::cv_status::timeout) {
                return nullptr;
            }
        }

        int free = pool_.size();

        auto conn = pool_.front();
        pool_.pop_front();
    }

    void Put(std::shared_ptr<T> conn)
    {
        if (!conn) {
            return;
        }

        std::unique_lock<std::mutex> lock(mutex_);
        pool_.push_back(conn);
    }

private:
    ConnectPool()  = default;
    ~ConnectPool() = default;

    ConnectPool(const ConnectPool &) = delete;
    ConnectPool &operator=(const ConnectPool &) = delete;

    auto creatConn()
    {
        auto conn = std::make_shared<T>();
        return conn->connect(connect_);
    }

    std::mutex mutex_;
    std::string connect_;  // 连接
    int maxIdle_{0};       // 最大空闲数
    int maxConn_{10};      // 最大连接数
    int numOpen_{0};       //打开的连接数
    std::condition_variable condition_;
    std::once_flag flag_;
    std::deque<std::shared_ptr<T>> pool_;
};
}  // namespace uranus::database