#include "tcpServer.h"
#include "inetAddress.h"
#include "acceptor.h"
#include "eventLoopThreadPool.h"
#include "eventLoop.h"
#include "tcpConnection.h"
#include "logger.h"

#include <unistd.h>
#include <string.h>
#include <functional>


namespace muduo_study
{
TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& nameArg, Option option)
    : loop_(loop),
    ipPort_(listenAddr.toIpPort()),
    name_(nameArg),
    acceptor_(new Acceptor(loop,listenAddr, option == kReusePort)),
    threadPool_(new EventLoopThreadPool(loop, nameArg)),
    connectionCallback_(defaultConnectionCallback),
    messageCallback_(defaultMessageCallback),
    nextConnId_(1),
    started_(0)
{
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer()
{
    for(auto& item: connections_)
    {
        TcpConnectionPtr conn(item.second);
        item.second.reset(); 
        conn->getLoop()->runInloop(
            std::bind(&TcpConnection::connectDestroyed, conn)
        );
    }
}

void TcpServer::setThreadNum(int numThreads)
{
    threadPool_->setThreadNum(numThreads);
}

void TcpServer::start()
{

    if(started_++ == 0)
    {
        threadPool_->start(threadInitCallback_);

        loop_->runInloop(
            std::bind(&Acceptor::listen, get_pointer(acceptor_))
        );
    }

}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
{
    EventLoop* ioLoop =  threadPool_->getNextLoop();
    char buf[64];
    snprintf(buf, 64, "-%s#%d", name_.c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    LOG_INFO("New connection servername::%s newconnectname::%s from %s", name_.c_str(), connName.c_str(), peerAddr.toIpPort().c_str());

    sockaddr_in localSocketAddr;
    memset(&localSocketAddr, 0, sizeof(localSocketAddr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(localSocketAddr));
    ::getsockname(sockfd, (sockaddr*)&localSocketAddr, &addrlen);
    InetAddress localAddr(localSocketAddr);

    TcpConnectionPtr conn( new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));

    connections_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(
        std::bind(&TcpServer::removeConnection, this, std::placeholders::_1)
    );

    ioLoop->runInloop(
        std::bind(&TcpConnection::connectEstablished, conn)
    );
}


void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
    loop_->runInloop(
        std::bind(&TcpServer::removeConnectionInLoop, this, conn)
    );
}
void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
    size_t n = connections_.erase(conn->name());
    EventLoop *ioLoop = conn->getLoop();
    ioLoop->queueInloop(
        std::bind(&TcpConnection::connectDestroyed, conn)
    );
}    
} // namespace muduo_study



