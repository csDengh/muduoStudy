#pragma once

#include<netinet/in.h>
#include<string>
#include<sys/socket.h>


namespace muduo_study
{

/**
 * @brief 网络地址封装类
 */
class InetAddress
{
public:
    explicit InetAddress(uint16_t port, std::string ip="127.0.0.1");
    explicit InetAddress(sockaddr_in& addr): addr_(addr) {}

    sa_family_t family() const {return addr_.sin_family;}
    std::string toIp() const;
    std::string toPort() const;
    std::string toIpPort() const;
    sockaddr* getSockAddr() const { return (sockaddr*)&addr_;};
    void setSockAddr(sockaddr_in& addr) {addr_ = addr;}
private:
    sockaddr_in addr_;
};

} // namespace muduo_study


