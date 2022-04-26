#pragma once

#include<string>

/**
 * @brief 将日志输出到终端
 * @details 单例获取Logger对象，设置日志级别，输出到终端。
 */
#define LOG_INFO(loggerformat, ...) \
    {\
        Logger &log_instance = Logger::get_instance();\
        log_instance.set_level(INFO); \
        char buf[1024]={0}; \
        snprintf(buf, 1024, loggerformat, ##__VA_ARGS__); \
        log_instance.output_for_terminal(buf);\
    }\

#define LOG_ERROR(loggerformat, ...) \
    {\
        Logger &log_instance = Logger::get_instance();\
        log_instance.set_level(ERROR); \
        char buf[1024]={0}; \
        snprintf(buf, 1024, loggerformat, ##__VA_ARGS__); \
        log_instance.output_for_terminal(buf);\
    }\

#define LOG_DEBUG(loggerformat, ...) \
    {\
        Logger &log_instance = Logger::get_instance();\
        log_instance.set_level(DEBUG); \
        char buf[1024]={0}; \
        snprintf(buf, 1024, loggerformat, ##__VA_ARGS__); \
        log_instance.output_for_terminal(buf);\
    }\

#define LOG_FATAL(loggerformat, ...) \
    {\
        Logger &log_instance = Logger::get_instance();\
        log_instance.set_level(FATAL); \
        char buf[1024]={0}; \
        snprintf(buf, 1024, loggerformat, ##__VA_ARGS__); \
        log_instance.output_for_terminal(buf);\
        exit(-1); \
    }\


namespace muduo_study
{

/**
 * @brief 日志级别
 */
enum logger_level
{
    INFO,
    ERROR,
    DEBUG,
    FATAL,
};

/**
 * @brief 简单的日志实现
 */
class Logger
{
private:
    Logger(){};
    ~Logger(){};

public:

    /**
     * @brief 单例，获取日志对象
     */
    static Logger&  get_instance();

    /**
     * @brief 将str输出到终端
     * @details 附带日期和日志级别
     */
    void output_for_terminal(std::string str);

    /**
     * @brief 设置日志级别
     * @param[in] m_log_level 日志级别 
     */
    void set_level(logger_level m_log_level);

private:
    /// 日志级别
    logger_level log_level_;

};
    
} // namespace mduo_study









