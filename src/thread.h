#pragma once

#include "nocopyable.h"

#include<functional>
#include <string>
#include <memory>
#include <thread>
#include <atomic>


namespace muduo_study
{

/**
 * @brief thread类
 */
class Thread: nocopyable
{

public:
    using ThreadFunc = std::function<void()>;

    explicit Thread(ThreadFunc, const std::string &name = std::string());
    ~Thread();

    /**
     * @brief 创建子线程，执行回调函数
     */
    void start();

    /**
     * @brief thread.join()
     */
    void join();

    /**
     * @brief 获取线程是否运行
     */
    bool started() const { return started_; } 
    
    /**
     * @brief 获取线程的id
     */
    pid_t tid() const {return tid_;}

    /**
     * @brief 获取线程的名字
     */  
    const std::string& name() const {return name_; }

    /**
     * @brief 获取创建线程的数量
     */
    static int numCreated() { return numCreated_; }
private:

    /**
     * @brief 设置线程名字
     */
    void setDefaultName();

    /// 线程是否在运行
    bool started_;
    /// 线程是否join
    bool joined_;
    std::shared_ptr<std::thread> thread_;  
    /// 线程id
    pid_t tid_;    
    /// 线程回调函数的执行的函数
    ThreadFunc func_;
    /// 线程名字
    std::string name_;   
    /// 统计线程数量
    static std::atomic_int numCreated_;  
};

} // namespace muduo_study



