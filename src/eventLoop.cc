#include "eventLoop.h"
#include "logger.h"
#include "poller.h"
#include "channel.h"

#include <sys/eventfd.h>
#include <signal.h>
#include <memory>

namespace
{
    using namespace muduo_study;

    /// 每个线程创建出来的变量地址都不一样, 使得每个线程只有一个eventloop
    __thread EventLoop* t_loopInThisThread = 0; 

    /// epoll_wait 的时间
    const int kPollTimeMs = 10000;  

    int createEventfd()
    {
        // 非堵塞， fork 执行exec函数时，关闭该fd
        int evfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC); 
        if(evfd < 0)
        {
            LOG_FATAL("%s", "Failed in eventfd");
        }
        return evfd;
    }

    #pragma GCC diagnostic ignored "-Wold-style-cast"
    class IgnoreSigPipe
    {
        public:
        IgnoreSigPipe()
        {
        ::signal(SIGPIPE, SIG_IGN);
        }
    };
};


namespace muduo_study
{

EventLoop::EventLoop()
    : looping_(false),
    quit_(false),
    callingPendingFunctors_(false),
    threadId_(CurrentThread::tid()),
    poller_(Poller::newDefaultPoller(this)),
    wakeupFd_(createEventfd()),
    wakeupChannel_(new Channel(this, wakeupFd_)),
    curtentActiveChannel_(nullptr)
{
    if(t_loopInThisThread)
    {
        LOG_FATAL("%s", "another eventloop exit");
    }
    else
    {
        t_loopInThisThread = this;
    }


    /**
     * @details 将eventfd创建的用于线程间通信的fd也封装为channel，因为该fd可以被epoll监听，
     * @details 所以就将该wakeupChannel注册到subreactor。因为注册了read事件，所以可以唤醒subreactor。
     */
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = NULL;
}


void EventLoop::loop()
{
    looping_ = true;
    quit_ = false;

    while(!quit_)
    {

        activeChannels_.clear();

        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);

        for(Channel* channel: activeChannels_)
        {
            curtentActiveChannel_ = channel;
            curtentActiveChannel_->handleEvent(pollReturnTime_);
        }

        curtentActiveChannel_ = nullptr;
        doPendingFunctors();
    }

    looping_ = false;
}


void EventLoop::quit()
{
    quit_ = true;

    if (!isInLoopThread())
    {
        wakeup();
    }
}


void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_, &one, sizeof(one));
    if(n != sizeof(n))
    {
        LOG_ERROR("eventloop::wakeup() error %lu", n);
    }
}


void EventLoop::runInloop(Functor cb)
{
    if(isInLoopThread())
    {
        cb();
    }
    else
    {
        queueInloop(cb);
    }
}

void EventLoop::queueInloop(Functor cb)
{
    std::unique_lock<std::mutex> lock(mutex_);
    pendingFunctors_.emplace_back(cb);

    /**
     * @details 因为添加了新的回调，然后因为当前正在执行，因为唤醒，所以epoll返回，所以需要唤醒执行回调函数
     * @details 有10秒的阻塞时间，加快退出epoll_wait，从而更快执行functors
     */
    if(!isInLoopThread() || callingPendingFunctors_)
    {
        wakeup();
    }
}



void EventLoop::updateChannel(Channel* channel)
{
    
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel)
{
    return poller_->hasChannel(channel);
}


void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof (one));
    if( n != sizeof(one))
    {
        LOG_ERROR("%s", "wakeread_handleread_error");
    }
}


void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    
    /**
     * @note 定义了一个局部的functors 然后交换的好处在于：因为functors 是存在多个pool写入，
     * @note 为了不阻塞线程，也就是加锁的时间尽量短，就进行swap（），而且functors的回调做完后就不用了。
     */
    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for(const Functor& functor: functors)
    {
        functor();
    }

    callingPendingFunctors_ = false;
}

} // namespace muduo_study
