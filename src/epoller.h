#pragma once

#include "poller.h"

#include<vector>

struct epoll_event;

namespace muduo_study
{

/**
 * @brief epoll的封装
 */
class Eepoller: public Poller
{
public:

    /**
     * @brief 创建epoll句柄（epoll_create）
     */
    Eepoller(EventLoop* loop);
    ~Eepoller() override;

    /**
     * @brief 调用epoll_wait，获得就绪事件
     * @details 将就绪事件的channel添加到activeChannels
     * @note 当监听到事件数量等于epoll_event vector的大小时，说明当前活跃事件多，对该vector两倍扩容
     */
    Timestamp poll(int timeoutMs, ChannelList* activeChannels) override;
    
    /**
     * @brief 根据channel的状态更改在epoll的状态
     */
    void updateChannel(Channel* channel) override;

    /**
     * @brief 从poll中移除并在channelmap中移除
     */
    void removeChannel(Channel* channel) override;

private:

    /// epoll_wait参数中epoll_event vector的大小
    static const int kInitEventListSize = 16;

    /**
     * @brief 将活跃事件对应的channel写入到activeChannels
     */
    void fillActiveChannels(int numEvents, ChannelList* activeChannels) const; 

    /**
     * @brief epoll_ctl
     */
    void update(int operation, Channel* channel);

    using EventList  = std::vector<epoll_event>;
    /// epoll 句柄
    int epollfd_;
    /// epoll_wait负责将活跃的事件拷贝到events_; 相较于数组，可动态增大接受活跃事件的数量
    EventList events_;

};

} // namespace muduo_study
