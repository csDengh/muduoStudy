#include <iostream>

#include "logger.h"
#include "timestamp.h"

namespace muduo_study
{

Logger& Logger::get_instance()
{
    static Logger log_instance;
    return log_instance;
}

void Logger::set_level(logger_level m_log_level)
{
    log_level_ = m_log_level;
}

void Logger::output_for_terminal(std::string str)
{
    switch (log_level_)
    {
        case INFO: std::cout << "INFO ";
            break;

        case ERROR: std::cout << "ERROR ";
            break;

        case DEBUG: std::cout << "DEBUG ";
            break;

        case FATAL: std::cout << "FATAL ";
            break;
    }

    std::cout << Timestamp::now().toString() << str <<std::endl;
}

} // namespace muduo_study




