#pragma once

#include "timestamp.h"
#include "buffer.h"

#include <functional>
#include <memory>


namespace muduo_study
{

/**
 * @brief 返回原生指针。
 */
template<typename T>
inline T*get_pointer(const std::shared_ptr<T>& ptr)
{
    return ptr.get();
}

template<typename T>
inline T* get_pointer(const std::unique_ptr<T>& ptr)
{
    return ptr.get();
}


class TcpConnection;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using TimerCallback = std::function<void()>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;
using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr&, size_t)>;

/**
 * @brief 用户发送请求后的回调。
 */
using MessageCallback = std::function<void(const TcpConnectionPtr&, Buff*, Timestamp)>;

/**
 * @brief 连接建立后的回调，如果用户不指定，默认行为
 */
void defaultConnectionCallback(const TcpConnectionPtr& conn);

/**
 * @brief 用户发送请求后的回调，不指定，默认行为
 */
void defaultMessageCallback(const TcpConnectionPtr& conn, Buff* , Timestamp receiveTime);

} // namespace muduo_study





