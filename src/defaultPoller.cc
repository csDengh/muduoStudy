#include "poller.h"
#include "epoller.h"


namespace muduo_study
{

class EventLoop;

/**
 * @brief 创建epoll对象
 */
Poller* Poller::newDefaultPoller(EventLoop* evtlp)
{
   return new Eepoller(evtlp);
}

} // namespace muduo_study


