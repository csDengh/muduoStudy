#include "inetAddress.h"

#include<iostream>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string>
#include<string.h>


namespace muduo_study
{

InetAddress::InetAddress(uint16_t port, std::string ip)
{
    bzero(&addr_, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr = inet_addr(ip.c_str());
}

std::string InetAddress::toIp() const
{
    return inet_ntoa(addr_.sin_addr);
}

std::string InetAddress::toPort() const
{
    return std::to_string(ntohs(addr_.sin_port));
}
std::string InetAddress::toIpPort() const
{
    return toIp() + ":" + toPort();
}
    
} // namespace muduo_study
