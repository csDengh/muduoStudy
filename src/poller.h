#pragma once

#include "timestamp.h"
#include "eventLoop.h"
#include "nocopyable.h"

#include<map>
#include<vector>


namespace muduo_study
{

class Channel;

/**
 * @brief 抽象基类，多态使得epoll select 有相同的调用接口
 */
class Poller: nocopyable
{
public:
    using ChannelList = std::vector<Channel*>;

    Poller(EventLoop* loop);        
    virtual ~Poller();

    /**
     * @brief 让派生类重写poll的轮询
     */
    virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;
    
    /**
     * @brief 根据channel的index状态更新channel在poll的状态
     */
    virtual void updateChannel(Channel* channel) = 0;

    /**
     * @brief 从poll中移除
     */
    virtual void removeChannel(Channel* channel) = 0;

    /**
     * @brief 创建并获取当前的对象。不在基类实现，分离
     */
    static Poller* newDefaultPoller(EventLoop* loop);

    /**
     * @brief 查看channel是否在channelmap中
     */
    virtual bool hasChannel(Channel* channel) const;

protected:
    using ChannelMap = std::map<int, Channel*>; 

    /// 已经加入到poll监听的channel
    ChannelMap channels_; 

private:
    /// poll 所属的EventLoop
    EventLoop* ownerLoop_; 
};

} // namespace muduo_study
