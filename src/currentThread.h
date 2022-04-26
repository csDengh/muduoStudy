#pragma once

#include<sys/syscall.h>
#include<unistd.h>


namespace muduo_study
{
namespace CurrentThread
{
    extern __thread int t_cachedTid;


    /**
     * @brief 获得调用该函数的线程id
     */
    void cacheTid();

    inline int tid()
    {
        if(__builtin_expect(t_cachedTid == 0, 0)) 
        {
            cacheTid();
        }
        return t_cachedTid;
    }
 
}

} // namespace muduo_study



