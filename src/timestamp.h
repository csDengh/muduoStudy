#pragma once

#include<iostream>
#include <string>


namespace muduo_study
{

/**
 * @brief 时间戳
 */
class Timestamp
{
public:
    Timestamp();

    explicit Timestamp(int64_t microSecondsSinceEpochArg);

    /**
     * @brief 时间戳转为字符串
     * @param[in] microSecondsSinceEpochArg 字符串 
     * @return 时间，形如Wed Jun 30 21:49:08 1993
     */
    std::string toString() const;

    /**
     * @brief 获得一个时间戳
     * @return 返回当前的时间戳
     */
    static Timestamp now();
private:
    int64_t microSecondsSinceEpoch_;

};

} // namespace muduo_study



