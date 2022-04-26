#include "acceptor.h"
#include "inetAddress.h"
#include "logger.h"

#include <unistd.h>
#include <fcntl.h>


namespace muduo_study
{

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listentAddr, bool reuseport)
    :loop_(loop),
    acceptSocket_(::socket(listentAddr.family(), SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP)),
    accpetChannel_(loop, acceptSocket_.fd()),
    listening_(false),
    idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC))
{
    acceptSocket_.setReusePort(reuseport);
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.bindAddress(listentAddr);
    accpetChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor()
{
    accpetChannel_.disableAll();
    accpetChannel_.remove();
    ::close(idleFd_);
}


void Acceptor::listen()
{
    listening_ = true;
    acceptSocket_.listen();
    accpetChannel_.enableReading();
}


void Acceptor::handleRead()
{
    sockaddr_in sockaddr_in_;
    InetAddress peerAddr(sockaddr_in_);
    int connfd = acceptSocket_.accept(&peerAddr);
    if(connfd >=0 )
    {
        if(newConnectionCallback_)
        {
            newConnectionCallback_(connfd, peerAddr);
        }
        else
        {
            ::close(connfd);
        }
    }
    else
    {
        LOG_ERROR("%s", "acceptor:: accpet error");
        if(errno == EMFILE)
        {
            ::close(idleFd_);
            idleFd_ = ::accept(acceptSocket_.fd(), NULL, NULL);
            ::close(idleFd_);
            idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}

} // namespace muduo_study



