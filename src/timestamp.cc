#include "timestamp.h"

#include<string>
#include<time.h>


namespace muduo_study
{

Timestamp::Timestamp() : microSecondsSinceEpoch_(0) {};

Timestamp::Timestamp(int64_t microSecondsSinceEpochArg) : microSecondsSinceEpoch_(microSecondsSinceEpochArg) {};


std::string Timestamp::toString() const
{
    char buf[128]={0};
    snprintf(buf, 128, "%s", ctime(&microSecondsSinceEpoch_));
    return buf;
}

Timestamp Timestamp::now()
{
    return Timestamp(time(NULL));
} 
   
} // namespace muduo_study





