#include "tcpConnection.h"
#include "buffer.h"
#include "socket.h"
#include "channel.h"
#include "eventLoop.h"
#include "timestamp.h"
#include "logger.h"

#include <string.h>
#include <unistd.h>
#include <errno.h>



namespace muduo_study
{

/**
 * @brief 连接建立后的回调，如果用户不指定，默认行为就是不作处理
 */ 
void defaultConnectionCallback(const TcpConnectionPtr& conn)
{

}

/**
 * @brief 连接建立后触发读事件的回调，不指定，默认行为就是buf读写idx复位
 */ 
void defaultMessageCallback(const TcpConnectionPtr& conn, Buff* bufer, Timestamp receiveTime)
{
    bufer->retrieveAll(); 
}

TcpConnection::TcpConnection(EventLoop* loop, const std::string& name, int sockfd,
                const InetAddress& localAddr, const InetAddress& peerAddr)
    : loop_(loop),
    name_(name),
    socket_(new Socket(sockfd)),
    localAddr_(localAddr),
    peerAddr_(peerAddr),
    channel_(new Channel(loop, sockfd)),
    state_(kDisconnected),
    reading_(false),
    highWaterMark_(64*1024*1024)
{
    channel_->setReadCallback(
        std::bind(&TcpConnection::handleRead, this, std::placeholders::_1)
    );
    channel_->setWriteCallback(
        std::bind(&TcpConnection::handleWrite, this)
    );
    channel_->setCloseCallback(
        std::bind(&TcpConnection::handleClose, this)
    );
    channel_->setErrorCallback(
        std::bind(&TcpConnection::handleError, this)
    );
    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{

}

bool TcpConnection::getTcpInfo(tcp_info* tcpInfo) const 
{
    return socket_->getTcpInfo(tcpInfo);
}

std::string TcpConnection::getTcpInfoString() const 
{
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    socket_->getTcpInfoString(buf, sizeof(buf));
    return buf;
}

void TcpConnection::send(const void* message, int len)
{
    if(state_ == kConnected)
    {
        if(loop_->isInLoopThread())
        {
            sendInLoop(message, len);
        }
        else
        {
            loop_->runInloop(
                std::bind(&TcpConnection::sendInLoop, this, message, len)
            );
        }
    }
}

void TcpConnection::send(Buff* message)
{
    if(state_ == kConnected)
    {
        if( loop_->isInLoopThread())
        {
            sendInLoop(message->peek(), message->readableBytes());
            message->retrieveAll();
        }
        else
        {
            loop_->runInloop(
                std::bind(&TcpConnection::sendInLoop, this,  message->peek(), message->readableBytes())
            );
        }
    }
}



void TcpConnection::sendInLoop(const void* message, size_t len)
{
    ssize_t nwrote = 0;
    size_t  remaining = len;
    bool faultError = false;

    if(state_ == kDisconnected)
    {
        LOG_ERROR("%s", "disconnected");
        return ;
    }
    
    if(!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
    {
        nwrote = write(channel_->fd(), message, len);
        if(nwrote >= 0)
        {
            remaining = len-nwrote;
            if(remaining == 0 && writeCompleteCallback_)
            {
                loop_->queueInloop(
                    std::bind(writeCompleteCallback_, shared_from_this())
                );
            }
        }
        else
        {
            nwrote = 0;
            if(errno != EWOULDBLOCK)
            {
                if(errno == EPIPE || errno == ECONNRESET)
                {
                    faultError = true;
                }
            }
        }
    }

    if(!faultError && remaining >0)
    {
        size_t oldlen = outputBuffer_.readableBytes();

        if(oldlen + remaining >= highWaterMark_ && oldlen < highWaterMark_ && highWaterCallback_)
        {
            loop_->queueInloop(
                std::bind(highWaterCallback_, shared_from_this(), oldlen+remaining)
            );
        }
        outputBuffer_.append(static_cast<const char*>(message) + nwrote, remaining);
        if(!channel_->isWriting())
        {
            channel_->enableWriting();
        }
    }

}

void TcpConnection::shutdown()
{
    if(state_ == kConnected)
    {
        setState(kDisconnecting);
        loop_->runInloop(
            std::bind(&TcpConnection::shutdownInLoop, this)
        );
    }
}

void TcpConnection::shutdownInLoop()
{
    if( !channel_->isWriting())
    {
        socket_->shutdownWrite();
    }
}


void TcpConnection::forceClose()
{
    if(state_ == kConnected || state_ == kDisconnecting)
    {
        loop_->queueInloop(
            std::bind(&TcpConnection::forceCloseInLoop, shared_from_this())
        );
    }
}

void TcpConnection::forceCloseInLoop()
{
    if(state_ == kConnected || state_ == kDisconnecting)
    {
        handleClose();
    }
}


void TcpConnection::setTcpNoDelay(bool on)
{
    socket_->setTcpNoDelay(on);
}

const char* TcpConnection::stateToString() const
{
  switch (state_)
  {
    case kDisconnected:
      return "kDisconnected";
    case kConnecting:
      return "kConnecting";
    case kConnected:
      return "kConnected";
    case kDisconnecting:
      return "kDisconnecting";
    default:
      return "unknown state";
  }
}


void TcpConnection::startRead()
{
    loop_->runInloop(
        std::bind(&TcpConnection::startReadInLoop, this)
    );
}


void TcpConnection::startReadInLoop()
{
    if(!reading_ || !channel_->isReading())
    {
        channel_->enableReading();
        reading_ = true;
    }
}

void TcpConnection::stopRead()
{
    loop_->runInloop(
        std::bind(&TcpConnection::stopReadInLoop, this)
    );
}

void TcpConnection::stopReadInLoop()
{
    if( reading_ || channel_->isReading())
    {
        channel_->disableReading();
        reading_ = false;
    }
}



void TcpConnection::connectEstablished()
{
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading();
    conectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
    if(state_ == kConnected)
    {
        setState(kDisconnected);
        channel_->disableAll();
        conectionCallback_(shared_from_this());
    }
    channel_->remove();
}


void TcpConnection::handleRead(Timestamp receiveTime)
{
    int saveErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &saveErrno);
    if(n > 0)
    {
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    }
    else if( n== 0)
    {
        handleClose();
    }
    else
    {
        errno = saveErrno;
        handleError();
    }
}

void TcpConnection::handleWrite()
{
    if(channel_->isWriting())
    {
        ssize_t n = ::write(channel_->fd(), outputBuffer_.peek(), outputBuffer_.readableBytes());
        if(n > 0)
        {
            outputBuffer_.retrieve(n);
            if(outputBuffer_.readableBytes() == 0)
            {
                channel_->disableWriting();
                if(writeCompleteCallback_)
                {
                    loop_->queueInloop(
                        std::bind(writeCompleteCallback_, shared_from_this())
                    );
                }
                if(state_ == kDisconnecting)
                {
                    shutdownInLoop();
                }
            }
        }
        else
        {

        }
    }
}

void TcpConnection::handleClose()
{
    channel_->disableAll();

    TcpConnectionPtr guardThis(shared_from_this());
    conectionCallback_(guardThis);
    closeCallback_(guardThis);
}

void TcpConnection::handleError()
{
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof optval);
    int err;
    if (::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        err = errno;
    }
    else
    {
        err = optval;
    }
}
    
} // namespace muduo_study



