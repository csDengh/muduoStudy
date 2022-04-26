#include "epoller.h"
#include "logger.h"
#include "channel.h"

#include<sys/epoll.h>
#include <unistd.h>
#include <string.h>


namespace muduo_study
{


/**
 * @brief channel::index，表明channel在poll中的状态
 */
const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;


Eepoller::Eepoller(EventLoop* loop)
    : Poller(loop), 
    epollfd_(epoll_create1(EPOLL_CLOEXEC)),
    events_(kInitEventListSize)
{
    if (epollfd_ < 0)       
    {
        LOG_FATAL("%s", "Epollfd_create failed");
    }
    
}

Eepoller::~Eepoller()
{
    ::close(epollfd_);
}

Timestamp Eepoller::poll(int timeoutMs, ChannelList* activeChannels)
{
    int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), 
                                static_cast<int>(events_.size()), timeoutMs);
    int saveErrno = errno;

    Timestamp now(Timestamp::now());

    if(numEvents > 0)
    {
        fillActiveChannels(numEvents, activeChannels);

        if(static_cast<size_t>(numEvents) == events_.size())
        {
            events_.resize(events_.size()*2);
        }
    }
    else if(numEvents < 0)
    {
        if(saveErrno != EINTR)
        {
            LOG_ERROR("%s", "epoll::poll()");
        }
    }
    return now;
}

void Eepoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const
{
    for(int i=0; i<numEvents; ++i)
    {
        Channel * channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revent(events_[i].events);
        activeChannels->push_back(channel);
    }
}


void Eepoller::updateChannel(Channel* channel)
{
    const int index = channel->index();
    if(index == kNew || index == kDeleted)
    {
        int fd = channel->fd();
        if(kNew == index)
        {
            channels_[fd] = channel;
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else
    {
        if(channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else
        {
            update(EPOLL_CTL_MOD, channel);
        }

    }
}

void Eepoller::removeChannel(Channel* channel)
{
    int fd = channel->fd();
    int index = channel->index();
    channels_.erase(fd);
    if(index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}


void Eepoller::update(int operation, Channel* channel)
{
    epoll_event event;
    memset(&event, 0, sizeof(event));
    int fd = channel->fd();
    event.data.fd = fd;
    event.data.ptr = channel;
    event.events = channel->event();

    int state = epoll_ctl(epollfd_, operation, fd, &event);

    if(state <0)
    {
        if(operation == EPOLL_CTL_DEL)
        {
            LOG_ERROR("%s %d", "epoll update del error ", fd);
        }
        else
        {
            LOG_ERROR("update op:%d error %d", operation, fd);
        }
    }

}

} // namespace muduo_study

