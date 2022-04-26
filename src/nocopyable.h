#pragma once

namespace muduo_study
{

/**
 * @brief 继承nocopyable类的派生类对象不可被拷贝构造和拷贝赋值
 */
class nocopyable
{
protected:
    nocopyable() = default;
    ~nocopyable() = default;
public:
    nocopyable(const nocopyable& nocopy_obj) = delete;
    nocopyable operator=(const nocopyable& nocopy_obj) = delete;
};

} // namespace muduo_study



