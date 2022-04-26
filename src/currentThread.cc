#include "currentThread.h"



namespace muduo_study
{
    
namespace CurrentThread
{
    __thread int t_cachedTid = 0;

    void cacheTid()
    {
        t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
    }
}

} // namespace muduo_study



