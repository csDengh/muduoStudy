#include <muduo_study/tcpServer.h>
#include <muduo_study/eventLoop.h>
#include <muduo_study/tcpConnection.h>

#include <functional>
#include <iostream>


class TestServer
{
public:

    TestServer(muduo_study::EventLoop *loop, muduo_study::InetAddress &listenAddr)
    :server_(loop, listenAddr, "ChatServer")
    {

    server_.setConnectionCallback(std::bind(&TestServer::onConnection, this, std::placeholders::_1));
    server_.setMessageCalback(std::bind(&TestServer::onMessage, this,  std::placeholders::_1,  std::placeholders::_2,  std::placeholders::_3));

    server_.setThreadNum(10);
    }

    void start()
    {
    server_.start();
    }
private:
    // TcpServer绑定的回调函数，当有新连接或连接中断时调用
    void onConnection(const muduo_study::TcpConnectionPtr &con);
    // TcpServer绑定的回调函数，当有新数据时调用
    void onMessage(const muduo_study::TcpConnectionPtr &con,
    muduo_study::Buff *buf,
    muduo_study::Timestamp time);
private:
    muduo_study::TcpServer server_;
};

void TestServer::onConnection(const muduo_study::TcpConnectionPtr &con)
{
    if(con->connected())
    {
        std::cout <<"connected"<< con->name() << con->peerAddress().toIpPort() << con->localAddress().toIpPort() <<std::endl;
    }
    else 
    {
        std::cout << "close" <<std::endl;
    }
}

void TestServer::onMessage(const muduo_study::TcpConnectionPtr &con, muduo_study::Buff *buffer, muduo_study::Timestamp time)
{
    std::string buf = buffer->retrieveAllAsString();
    std::cout << "revc" << buf <<std::endl;

    muduo_study::Buff send_buf;
    send_buf.append("hello", 5);

    con->send(&send_buf);
    std::cout <<"send over" <<std::endl;
}


int main()
{
    muduo_study::EventLoop event_loop;
    muduo_study::InetAddress  intaddr(6000, "127.0.0.1");
    TestServer test_server(&event_loop, intaddr);

    test_server.start();
    event_loop.loop();

}