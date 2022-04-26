#pragma once

#include "nocopyable.h"
#include "inetAddress.h"
#include<netinet/tcp.h>


namespace muduo_study
{

/**
 * @brief Socket封装类
 */
class Socket: nocopyable
{

public:
    explicit Socket(int sockfd): sockfd_(sockfd) {}
    ~Socket();

    int fd() const {return sockfd_;}

    /**
     * @brief 获取tcp_info
     */
    bool getTcpInfo(tcp_info*) const;

    /**
     * @brief 获取tcp_info，已字符串形式输出
     */
    bool getTcpInfoString(char *buf, int len) const;

    /**
     * @brief listen socket 的地址端口绑定
     */
    void bindAddress(const InetAddress& localaddr);

    /**
     * @brief 开始listen，backlog默认设置为128
     */
    void listen();

    /**
     * @brief 接受一个新连接，返回新连接的sockfd，同时更新peeraddr
     */
    int accept(InetAddress* peeraddr);

    /**
     * @brief 关闭写端
     */
    void shutdownWrite();

    /**
     * @brief 是否启用NoDelay
     */
    void setTcpNoDelay(bool on);

    /**
     * @brief 是否启用地址复用
     */
    void setReuseAddr(bool on);

    /**
     * @brief 是否启用端口复用
     */
    void setReusePort(bool on);

    /**
     * @brief 是否启用TCP保活机制
     */
    void setKeepAlive(bool on);

private:
    int sockfd_;
};

} // namespace muduo_study




