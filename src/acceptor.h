#pragma once

#include "nocopyable.h"
#include "socket.h"
#include "channel.h"

#include <functional>


namespace muduo_study
{

class EventLoop;
class InetAddress;

/**
 * @brief listenfd封装类
 */
class Acceptor: nocopyable
{

public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress& )>;

    /**
     * @param[in] loop 该loop为mainloop，有用户创建，并传到TcpServer, 然后实例化给acceptor
     */ 
    Acceptor(EventLoop* loop, const InetAddress& listentAddr, bool reuseport);
    ~Acceptor();

    /**
     * @brief 当连接建立后的回调
     * @param[in] cb 用户可以定义，以函数对象形式传递。
     */ 
    void setNewConnectionCallback(const NewConnectionCallback& cb){newConnectionCallback_ = cb;}

    /**
     * @brief 开启监听，并将读事件注册到main reactor。
     */ 
    void listen();
    bool listening() const { return listening_;}

private:

    /**
     * @brief 新连接到来的read回调，先调用accept获取connfd和address，然后调用newconnectcallback
     */ 
    void handleRead();   

    /// baseloop也就是mainloop也就是用户定义的loop
    EventLoop* loop_;   
    /// listen的fd，
    Socket acceptSocket_; 
    /// listen的fd的channel封装， 
    Channel accpetChannel_;  
    /// accept后的连接建立后的sockfd进行分发给subreactor
    NewConnectionCallback newConnectionCallback_;
    ///是否 sock::listen
    bool listening_;  
    int idleFd_;

};

} // namespace muduo_study


