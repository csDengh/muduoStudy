#pragma once

#include "timestamp.h"
#include "nocopyable.h"

#include <functional>
#include <memory>


namespace muduo_study
{

class EventLoop;

/**
 * @brief fd的封装
 * @details fd的关心的事件，以及对关心事件的回调。
 */
class Channel: nocopyable
{
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    /**
     * @brief channel的传参构造
     * @param[in] loop fd所属的EventLoop 
     * @param[in] fd  fd的类型：listenfd, accepted fd, eventfd
     */
    Channel(EventLoop* loop, int fd);
    ~Channel();

    /**
     * @brief 根据epoll_wait返回的活跃事件revent调用相应的设置好的函数
     * @param[in] receiveTime  epoll_wait返回后的时间戳
     */
    void handleEvent(Timestamp receiveTime);

    /**
     * @brief 设置相应的回调函数
     */
    void setReadCallback(ReadEventCallback cb) {readCallback_ = std::move(cb);}
    void setWriteCallback(EventCallback cb) {writeCallback_ = std::move(cb);}
    void setCloseCallback(EventCallback cb) {closeCallback_ = std::move(cb);}
    void setErrorCallback(EventCallback cb) {errorCallback_ = std::move(cb);}
    

    /**
     * @brief  防止当前channel对应的tcpconnection对象销毁，还在调用
     */
    void tie(const std::shared_ptr<void>&);

    /**
     * @brief  设置fd
     */
    int fd() const {return fd_;}

    /**
     * @brief 获取当前fd想要监听的事件
     */
    int event() const {return events_;} 

    /**
     * @brief 设置当前fd活跃的事件
     */
    void set_revent(int revt) {revents_ = revt;}

    /**
     * @brief 获取fd是否有事件在监听
     */ 
    bool isNoneEvent() const {return events_ == kNoneEvent;}

    /**
     * @brief 添加或者取消关心的事件
     */ 
    void enableReading() {events_ |= kReadEvent; update();}
    void disableReading() {events_ &= ~kReadEvent; update();}
    void enableWriting() { events_ |= kWriteEvent; update(); }
    void disableWriting() { events_ &= ~kWriteEvent; update(); }
    void disableAll() { events_ = kNoneEvent; update(); }

    /**
     * @brief 当前是否注册读写事件
     */ 
    bool isWriting() const { return events_ & kWriteEvent; }
    bool isReading() const { return events_ & kReadEvent; }

    /**
     * @brief channel的状态
     */ 
    int index() { return index_;}

    /**
     * @brief 设置channel的状态
     */ 
    void set_index(int idx) {index_ = idx;}

    /**
     * @brief channel所属的EventLoop
     */ 
    EventLoop* ownerLoop() {return loop_;}

    /**
     * @brief pool 有一个channel map 管理fd->channel, 对应的就是在map中delete并且将epoll_ctl delete fd
     */     
    void remove();

private:

    /**
     * @brief 添加感兴趣事件后的更新到epoll
     */
    void update();

    /**
     * @brief 根据epoll_wait返回的活跃事件revent调用相应的设置好的函数
     * @param[in] receiveTime  epoll_wait返回后的时间戳
     */
    void handleEventWithGuard(Timestamp receiveTime);

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    /// fd 所属的EventLoop
    EventLoop* loop_; 
    /// channel 对应的fd
    const int fd_; 
    /// epoll注册的事件
    int events_;
    /// epoll wait返回的活跃的事件
    int revents_;
    /// 标记当前channel是否加入到epoll，是否是delete的，还是没有加入过的
    int index_;    
    /// 通过weak_ptr判断对象是否释放
    std::weak_ptr<void> tie_;
    /// 对象是否释放
    bool tied_;
    /// 是否加入到loop
    bool addedToLoop_;
    /// 是否正在执行活跃事件对应的处理函数
    bool eventHandling_;

    /// 事件就绪时的回调处理函数，根据不同的类型调用对应的函数
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};

} // namespace muduo_study
