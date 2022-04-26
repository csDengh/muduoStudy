#pragma once

#include "timestamp.h"
#include "nocopyable.h"
#include "currentThread.h"

#include <functional>
#include <vector>
#include <atomic>
#include <mutex>
#include <memory>




namespace muduo_study
{

class Channel;
class Poller;


/**
 * @brief 作为epoll类和channel类的管理者
 */
class EventLoop: nocopyable
{

public:
    using Functor = std::function<void()>;

    /**
     * @note 一个线程只能创建一个eventloop
     */
    EventLoop();
    ~EventLoop();

    /**
     * @brief 开启事件循环
     * @details 调用poll，返回调用后的时间，获得活跃的channel；对活跃的channel的活跃事件进行回调处理。
     * @details 并执行由其他loop传过来的函数对象
     */
    void loop();

    /**
     * @brief 退出事件循环
     * @note  在其他线程调用quit，先唤醒该epoll，然后退出loop
     * @note  每个loop都有一个wakefd，通过绑定该fd的读到epoll，其他线程可通过调用wakeup，唤醒该loop。
     */
    void quit();

    /**
     * @brief 返回最近调用的epoll_wait 的时间，也就是在Epoller::poll
     */
    Timestamp pollReturnTime() const {return pollReturnTime_;}

    /**
     * @brief 在当前loop的线程中执行cb，如果不在，把cb放入pendingFunctors_，然后唤醒loop
     */
    void runInloop(Functor cb);

    /**
     * @brief 把cb放入队列，到对应的线程执行
     */
    void queueInloop(Functor cb);

    /**
     * @brief 唤醒当前eventloop，通过写入数据，触发可读事件，然后执行doPendingFunctors
     */
    void wakeup();

    /**
     * @brief 通过 EventLoop来管理 channel 和 epoll
     */
    void updateChannel(Channel* channel);
    void removeChannel(Channel* Channel);
    bool hasChannel(Channel* Channel);

    /**
     * @brief 当前执行的线程是否是创建该EventLoop的线程
     */
    bool isInLoopThread() const {return threadId_ == CurrentThread::tid();}


private:

    /**
     * @brief wakefd的read事件对应的事件处理函数
     * @note  wakefd read读取的数据不重要，重要的是可以wakeup loop去执行PendingFunctors
     */
    void handleRead();

    /**
     * @brief 执行由其他loop传过来的函数对象
     */
    void doPendingFunctors();

    using ChannelList = std::vector<Channel*>;

    /// 当前是否在运行
    std::atomic_bool looping_; 
    std::atomic_bool quit_;

    /// 记录创建该eventLoop的线程，保证 one loop per thread
    const pid_t threadId_;  
    /// 调用poll后的时间点，也就是 Epoll::poll(epoll_wait)的返回
    Timestamp pollReturnTime_;   
    /// 所属的poller
    std::unique_ptr<Poller> poller_;
    /// 该fd作用是通过向该fd写入数据，使得epoll_wait可以立刻返回，因为epoll_wait 有10秒的超时时间。
    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;  

    /// 调用poller_::poll所传入的参数，可得到当前活跃的channel
    ChannelList activeChannels_;    
    Channel* curtentActiveChannel_;

    /// 当前是否正在执行PendingFunctors
    std::atomic_bool callingPendingFunctors_;  
    /// 存储的PendingFunctors
    std::vector<Functor> pendingFunctors_;
    //保证PendingFunctors线程安全。
    std::mutex mutex_;    
};

} // namespace muduo_study



