#pragma once

#include "callback.h"
#include "buffer.h"
#include "nocopyable.h"
#include "inetAddress.h"

#include <memory>
#include <atomic>

struct tcp_info;
namespace muduo_study
{
class EventLoop;
class Socket;
class Channel;
class InetAddress;
class Buff;

/**
 * @brief 建立连接的socket封装类
 */
class TcpConnection : nocopyable, 
                      public std::enable_shared_from_this<TcpConnection>
{
public:
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

    /**
     * @brief 获取一个新连接，对应的创建一个TCPConnection类来管理该连接
     * @details 为该连接绑定读写事件的处理函数
     * @param[in] loop 连接对应的loop
     */
    TcpConnection(EventLoop* loop, const std::string& name, int sockfd,
                  const InetAddress& localAddr, const InetAddress& peerAddr);

    ~TcpConnection();

    /**
     * @brief 获取连接的一些状态
     */
    EventLoop* getLoop() const { return loop_; }
    const std::string& name() const { return name_; }
    const InetAddress& localAddress() const { return localAddr_; }
    const InetAddress& peerAddress() const { return peerAddr_; }
    bool connected() const { return state_ == kConnected; }
    bool disconnected() const { return state_ == kDisconnected; }
    bool getTcpInfo(tcp_info* ) const ;
    std::string getTcpInfoString() const ;

    /**
     * @brief 向客户端发送数据
     * @details 若数据没有发送完毕，则将剩余的数据拷贝到输出缓冲区中并注册写事件。
     * @note 为了防止输出缓冲区无限增大，设置了警戒线，超出阈值则先执行 高水位回调。
     */
    void send(const void* message, int len);
    void send(Buff* message);

    /**
     * @brief socket关闭写端
     */
    void shutdown();

    /**
     * @brief 不在关心事件，调用closecallback
     */
    void forceClose();

    void setTcpNoDelay(bool on);

    /**
     * @brief 向epoll注册读事件
     */
    void startRead();

    /**
     * @brief 向epoll取消注册读事件
     */ 
    void stopRead();

    bool isReading() const { return reading_; }

    /**
     * @brief 设置对应事件的回调
     * @param[in] cb 用户可以定义，通过TcpServer类方法设定
     */ 
    void setConnectionCallback(const ConnectionCallback& cb) { conectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = cb; }
    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb) { highWaterCallback_ = cb; }
    void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }

    /**
     * @brief 缓冲区的首地址
     */     
    Buff* inputBuffer() { return &inputBuffer_; }
    Buff* outputBuffer() { return &outputBuffer_; }

    /**
     * @brief 连接建立后的处理，注册读事件，调用connectioncallback
     */   
    void connectEstablished();

     /**
     * @brief 销毁连接的处理，取消注册事件..
     */      
    void connectDestroyed();

private:
    enum StateE {
        kDisconnected,
        kConnecting,
        kConnected,
        kDisconnecting
    };

     /**
     * @brief channel绑定事件的回调函数的实现, 在构造函数中绑定
     * @note 这些方法会在某个情况下调用ConnectionCallback， MessageCallback，WriteCompleteCallback等这些系列回调函数
     * @note 这些函数可以用户传入
     */  
    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    /**
     * @brief 交由对应的loop执行
     */     
    void sendInLoop(const void* message, size_t len);
    void shutdownInLoop();
    void startReadInLoop();
    void stopReadInLoop();
    void forceCloseInLoop();

    void setState(StateE s) { state_ = s;}
    const char* stateToString() const;

    /// 连接所属的loop
    EventLoop* loop_;
    const std::string name_;
    std::atomic_int state_;
    bool reading_;
   /// 连接所对应的socket，channel
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    /// local和对端的地址
    const InetAddress localAddr_;
    const InetAddress peerAddr_;
    /// 用户可定义的回调，在handleRead，handleWrite，handleClose等函数中执行
    ConnectionCallback conectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterCallback_;
    CloseCallback closeCallback_;
    size_t highWaterMark_;
    /// 输入输出缓冲区
    Buff inputBuffer_;
    Buff outputBuffer_;

};

} // namespace muduo_study


