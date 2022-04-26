#include "eventLoopthread.h"
#include "eventLoop.h"


namespace muduo_study
{

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb, const std::string& name)
    : loop_(nullptr),
    exiting_(false),
    thread_(std::bind(&EventLoopThread::threadFunc, this), name),
    callback_(cb),
    mutex_(),
    cond_()
{
}
    
EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if(loop_ != nullptr)
    {
        loop_->quit();
        thread_.join();
    }
}



EventLoop* EventLoopThread::startLoop()
{
    thread_.start(); 

    /**
     * @details 主线程获得子线程tid即可退出start(),此时子线程在执行回调threadFunc（）
     * @details 当创建好了EventLoop，主线程就可以退出startLoop(),同时返回Loop指针
     */
    EventLoop* loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);

        while (loop_ == nullptr)
        {
            cond_.wait(lock);
        }
        loop = loop_;
    }

    return loop;
}



void EventLoopThread::threadFunc()
{
    EventLoop loop;
    if(callback_)
    {
        callback_(&loop);
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }

    loop.loop();

    loop_ = nullptr;
}

} // namespace muduo_study



