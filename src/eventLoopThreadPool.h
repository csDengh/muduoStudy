#pragma once

#include "nocopyable.h"

#include <functional>
#include <string>
#include <vector>
#include <memory>


namespace muduo_study
{

class EventLoop;
class EventLoopThread;

/**
 * @details EventLoopThreadPool就是根据配置的numThread数量，创建对应的EventLoopThread
 * @details EventLoopThread对象通过创建子线程，然后创建EventLoop, 执行事件循环
 * @details 主线程通过调用EventLoopThread对象的start方法获得EventLoop指针, 从而MainReactor可以方便管理这些EventLoop
 * @details 当mainReactor监听到可读事件时，accept获得connfd，使用循环分配或者hash分配到某个subreactor，后续该connfd的读写事件处理就在该subreactor处理
 */
class EventLoopThreadPool: nocopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    /**
     * @note baseLoop为mainreactor的eventloop
     */
    EventLoopThreadPool(EventLoop* baseLoop, const std::string& nameArg);
    ~EventLoopThreadPool();

    /**
     * @brief 设置线程数量
     */
    void setThreadNum(int numThreads) { numThreads_ = numThreads; }

    /**
     * @brief 根据配置的线程数量，创建与之对等的EventLoopThread对象，EventLoopThread对象调用startLoop(),开始loop，同时返回EventLoop指针。
     */
    void start(const ThreadInitCallback& cb = ThreadInitCallback());

    /**
     * @brief round-robin 轮询分配subreactor，返回对应的eventLoop
     * @note 如果是单线程，那就意味返回main reactor 的eventloop
     */
    EventLoop* getNextLoop();

    /**
     * @brief hash 分配subreactor，
     * @note 如果是单线程，那就意味返回main reactor 的eventloop
     */
    EventLoop* getLoopForHash(size_t hashCode);

    /**
     * @brief 获取所以的subreactor的eventloop
     * @note 没有subreactor，就返回mainreactor eventloop
     */
    std::vector<EventLoop*> getAllLoops();

    /**
     * @brief 是否调用start函数来创建subreactor
     */
    bool started() const {return started_;}

    /**
     * @brief server名字
     */
    const std::string& name() const { return name_;}

private:
    /// mainReactor的loop
    EventLoop* baseLoop_;
    /// server名字
    std::string name_;
    /// 是否调用start函数来创建subreactor
    bool started_;
    /// subreactor 的数量
    int numThreads_;
    /// 轮询的下一个subreactor的index
    int next_;
    /// EventLoopThread的vector
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    /// EventLoopThread对应的EventLoop的Vector
    std::vector<EventLoop*> loops_; 
};
    
} // namespace muduo_study





