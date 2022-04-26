#pragma once

#include "nocopyable.h"
#include "callback.h"
#include <functional>
#include <memory>

#include <atomic>
#include <map>


namespace muduo_study
{

class EventLoop;
class Acceptor;
class EventLoopThreadPool;
class InetAddress;

/**
 * @brief tcpServer封装类
 */
class TcpServer: nocopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    enum Option
    {
        kNoReusePort,
        kReusePort,
    };

    /**
     * @details 用户创建的loop，该loop为mainloop，在没有设置线程数量时，该loop既要处理新连接，也要处理建立连接的socket的读写。
     * @details 如果设置了线程数量，则该loop只需处理新连接，并分发给某个线程处理后续的读写。
     */
    TcpServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& nameArg, Option option = kNoReusePort);

    ~TcpServer();

    /**
     * @brief 获取server的某些信息
     */    
    const std::string& ipPort() const { return ipPort_;}
    const std::string& name() const { return name_;}
    EventLoop* getLoop() const { return loop_;}
    std::shared_ptr<EventLoopThreadPool> threadPool() { return threadPool_;}

     /**
     * @brief 设置线程数量，该数量也表明了subreactor的数量（one loop per thread）
     */      
    void setThreadNum(int numThreads);

     /**
     * @brief 设置初始thread对应的loop的回调
     */  
    void setThreadInitCallback(const ThreadInitCallback& cb) { threadInitCallback_ = cb;}

     /**
     * @details 创建numThread数量的线程，以及与之对应的eventLoop.然后子线程开始pool（epoll_wait）,子线程的EventLoop管理自己的channel，和监听对应的事件，以及做出回应。
     * @details 此时子线程是没有任务的，需要由Accptor分发给上面的EventLoop。然后listenfd开始监听并向mainloop_注册了读事件
     * @details 最后 mainloop_->loop()开始了server的运行
     */ 
    void start();

    /**
     * @brief 给与用户设置事件对应的回调函数
     */  
    void setConnectionCallback(const ConnectionCallback& cb){connectionCallback_ = cb;}

    void setMessageCalback(const MessageCallback& cb) { messageCallback_ = cb; }

    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = cb; }

private:

    /**
     * @brief listenfd对应的读事件的处理函数，在server调用构造函数时绑定
     * @details 通过eventloopthreadpool轮询获得一个eventloop，如果没有子线程，就是当前的用户创建的eventloop
     * @details 然后创建对应的tcpconnection，为其设置对应的回调函数
     * @details 最后为该channel注册读事件    
     */  
    void newConnection(int sockfd, const InetAddress& peerAddr);

    /**
     * @brief channel remove, disableAll
     */  
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

  
    using ConnectionMap = std::map<std::string, TcpConnectionPtr>;
    /// 用户创建的EventLoop, MainLoop;
    EventLoop* loop_;
    const std::string ipPort_;
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_;
    std::shared_ptr<EventLoopThreadPool> threadPool_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    ThreadInitCallback threadInitCallback_;

    std::atomic_int started_;

    int nextConnId_;
    /// 维护所以建立连接的TcpConnection
    ConnectionMap connections_;
};

} // namespace muduo_study


