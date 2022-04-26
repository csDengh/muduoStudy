#pragma once

#include "nocopyable.h"
#include "thread.h"

#include <string>
#include <mutex>
#include <condition_variable>


namespace muduo_study
{

class EventLoop;

/**
 * @brief eventloop和thread的管理者
 */
class EventLoopThread: nocopyable
{

public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(), 
                    const std::string& name = std::string());
    
    ~EventLoopThread();

    /**
     * @brief 主线程调用，返回子线程的eventloop指针，子线程开始了poll。调用者线程想要获得eventloop指针，所以需要用了condition来同步获得
     */
    EventLoop* startLoop();


private:

    /**
     * @brief 子线程的回调
     * @details 创建EventLoop, 初始化EventLoop, 将eventloop指针回给调用者，然后执行loop处理
     */
    void threadFunc();

    /// 是否在执行析构，退出
    bool exiting_;

    /// one loop per thread
    EventLoop *loop_;
    Thread thread_;
    /// 主线程等待子线程创建好了eventloop才可继续执行
    std::mutex mutex_;
    std::condition_variable cond_;

    ThreadInitCallback callback_;
};

} // namespace muduo_study




