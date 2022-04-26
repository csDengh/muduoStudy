#include "socket.h"
#include "logger.h"

#include<stdint.h>
#include <unistd.h>
#include<netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>


namespace muduo_study
{

Socket::~Socket()
{
    ::close(sockfd_);
}

bool Socket::getTcpInfo(tcp_info* tcpi) const
{
    socklen_t len = sizeof(*tcpi);
    memset(tcpi, 0, len);
    return getsockopt(sockfd_, SOL_TCP, TCP_INFO, tcpi, &len) == 0;
}

bool Socket::getTcpInfoString(char *buf, int len) const
{
    tcp_info tcpi;
    bool ok = getTcpInfo(&tcpi);
  if (ok)
  {
    snprintf(buf, len, "unrecovered=%u "
             "rto=%u ato=%u snd_mss=%u rcv_mss=%u "
             "lost=%u retrans=%u rtt=%u rttvar=%u "
             "sshthresh=%u cwnd=%u total_retrans=%u",
             tcpi.tcpi_retransmits,  // Number of unrecovered [RTO] timeouts
             tcpi.tcpi_rto,          // Retransmit timeout in usec
             tcpi.tcpi_ato,          // Predicted tick of soft clock in usec
             tcpi.tcpi_snd_mss,
             tcpi.tcpi_rcv_mss,
             tcpi.tcpi_lost,         // Lost packets
             tcpi.tcpi_retrans,      // Retransmitted packets out
             tcpi.tcpi_rtt,          // Smoothed round trip time in usec
             tcpi.tcpi_rttvar,       // Medium deviation
             tcpi.tcpi_snd_ssthresh,
             tcpi.tcpi_snd_cwnd,
             tcpi.tcpi_total_retrans);  // Total retransmits for entire connection
  }
  return ok;
}

void Socket::bindAddress(const InetAddress& localaddr)
{
    int ret = ::bind(sockfd_, localaddr.getSockAddr(), static_cast<socklen_t>(sizeof(struct sockaddr_in)));
    if(ret <0)
    {
        LOG_FATAL("%s", "listen socket bind error");
    }
}

void Socket::listen()
{
    int ret = ::listen(sockfd_, SOMAXCONN);
    if(ret < 0)
    {
        LOG_FATAL("%s", "listen socket listen error");
    }
}

int Socket::accept(InetAddress* peeraddr)
{
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    socklen_t addrlen = static_cast<socklen_t>(sizeof addr);

    int connfd = ::accept4(sockfd_, (sockaddr*)&addr, &addrlen, SOCK_NONBLOCK| SOCK_CLOEXEC);
    if(connfd >=0)
    {
        peeraddr->setSockAddr(addr);
    }
    return connfd;
}

void Socket::shutdownWrite()
{
    ::shutdown(sockfd_, SHUT_WR);
}

void Socket::setTcpNoDelay(bool on)
{
    int optval = on?1:0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, static_cast<socklen_t>(sizeof(optval)));
}

void Socket::setReuseAddr(bool on)
{
    int optval = on?1:0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof(optval)));
}

void Socket::setReusePort(bool on)
{
    int optval = on?1:0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, static_cast<socklen_t>(sizeof(optval)));
}

void Socket::setKeepAlive(bool on)
{
    int optval = on?1:0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<socklen_t>(sizeof(optval)));
}

} // namespace muduo_study




