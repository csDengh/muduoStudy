#pragma once

#include <vector>
#include <algorithm>
#include <string.h>
#include <string>


namespace muduo_study
{

/**
 * @brief 用户输入/输出缓冲区封装类
 */
class Buff
{
public:
    /// 预留8字节
    static const size_t kCheapPrepend = 8;
    /// 初始缓冲区大小1024
    static const size_t kInitialSize = 1024;

    explicit Buff(size_t initialSize = kInitialSize)
    : buf_(kCheapPrepend + initialSize),
    readerIndex_(kCheapPrepend),
    writerIndex_(kCheapPrepend)
    {
    }

    /**
     * @brief buffer的交换
     */
    void swap(Buff& rhs)
    {
        buf_.swap(rhs.buf_);
        std::swap(readerIndex_, rhs.readerIndex_);
        std::swap(writerIndex_, rhs.writerIndex_);
    }

    /**
     * @brief 可读的字节数
     */
    size_t readableBytes() const { return writerIndex_ - readerIndex_; }

    /**
     * @brief 可写的字节数
     */
    size_t writableBytes() const { return buf_.size() - writerIndex_; }


    /**
     * @brief 头部预留字节数
     */
    size_t prependableBytes() const { return readerIndex_; }

    /**
     * @brief 可读的首索引
     */
    const char* peek() const { return begin() + readerIndex_; }

    /**
     * @brief 查找 \r\n开始的索引
     */
    const char* findCRLF() const {
        const char *crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF+2);
        return crlf == beginWrite()? nullptr: crlf;
    }

    /**
     * @brief 从start开始查找 \r\n开始的索引
     */
    const char* findCRLF(const char* start) const
    {
        const char *crlf = std::search(start, beginWrite(), kCRLF, kCRLF+2);
        return crlf == beginWrite()? nullptr: crlf;
    }

    /**
     * @brief 查找 \n开始的索引
     */
    const char* findEOL() const
    {
        const void* eol = memchr(peek(), '\n', readableBytes());
        return static_cast<const char*>(eol);
    }

    /**
     * @brief 从start开始查找 \n开始的索引
     */
    const char* findEOL(const char* start) const
    {
        const void* eol = memchr(start, '\n', beginWrite()-start);
        return static_cast<const char*>(eol);
    }

    /**
     * @brief 读取完数据后的更新指针， 若未读取完，就更新readindex，若读取完，就重置readidx，wirteidx
     */
    void retrieve(size_t len)
    {
        if(len < readableBytes())
        {
            readerIndex_ += len; 
        }
        else
        {
            retrieveAll();
        }
    }

    void retrieveUntil(const char* end)
    {
        retrieve(end-peek());
    }

    void retrieveInt64()
    {
        retrieve(sizeof(int64_t));
    }

    void retrieveInt32()
    {
        retrieve(sizeof(int32_t));
    }

    void retrieveInt16()
    {
        retrieve(sizeof(int16_t));
    }

    void retrieveInt8()
    {
        retrieve(sizeof(int8_t));
    }

    /**
     * @brief 重置readidx，wirteidx
     */
    void retrieveAll()
    {
        readerIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend;
    }

    /**
     * @brief 读取缓冲区里的数据，以字符串输出
     */
    std::string retrieveAllAsString()
    {
        return retrieveAsString(readableBytes());
    }

    /**
     * @brief 读取缓冲区里len大小的数据，以字符串输出
     */
    std::string retrieveAsString(size_t len)
    {
        std::string result(peek(), len);
        retrieve(len);
        return result;
    }

    /**
     * @brief 向缓冲区中写入数据，若剩余空间不足，使用vector动态扩容，使得能够写入 或者 调整数据的位置。
     */
    void append(const char* data, size_t len)
    {
        ensureWritableBytes(len);
        std::copy(data, data+len, beginWrite());
        hasWritten(len);
    }

    void append(const void* data, size_t len)
    {
        append(static_cast<const char*>(data), len);
    }

    /**
     * @brief 更新写索引
     */
    void hasWritten(size_t len)
    {
        writerIndex_ += len;
    }

    /**
     * @brief 确保写的空间足够，若剩余空间不足，使用vector动态扩容，使得能够写入 或者 调整数据的位置。
     */
    void ensureWritableBytes(size_t len)
    {
        if(writableBytes() < len)
        {
            makeSpace(len);
        }
    }

    /**
     * @brief 可写的开始索引
     */
    char *beginWrite() { return begin() + writerIndex_;}
    const char * beginWrite() const {return begin() + writerIndex_; }

    void unwrite(size_t len)
    {
        writerIndex_ -= len;
    }

    /**
     * @brief 向preprend空间写入数据
     */
    void prepend(const void *data, size_t len)
    {
        readerIndex_ -=len;
        const char *d = static_cast<const char*>(data);
        std::copy(d, d+len, begin()+readerIndex_);
    }

    /**
     * @brief buf容量
     */
    size_t internalCapacity() const { return buf_.capacity();}

    /**
     * @details fd是LT，没有循环读取完sockfd的数据，由于不知道socket缓冲区的大小，所以也就不确定buf_的大小，
     * @details 通过从sock缓冲区读取数据到buf_, 减少系统调用，使用了readv来读取。
     * @details 使用了buf_ extrabuf 组成iovec来读取，如果使用了extra，必须扩容或者移动来读取extrabuf中的数据
     */
    ssize_t readFd(int fd, int* saveErrno);


private:

    char *begin() { return &*buf_.begin(); }
    const char* begin() const { return &*buf_.begin(); }


    void makeSpace(size_t len)
    {
        if( writableBytes() + prependableBytes() < len + kCheapPrepend)
        {
            buf_.resize(len + writerIndex_);
        }
        else
        {
            // 不需要扩容，将 readidx-writeidx的数据拷贝到kCheapPrepend;
            size_t readable = readableBytes();
            std::copy(begin()+readerIndex_, begin()+writerIndex_, begin()+kCheapPrepend);

            readerIndex_ = kCheapPrepend;
            writerIndex_ = kCheapPrepend + readable;
        }
    }



    std::vector<char> buf_;
    size_t readerIndex_;
    size_t writerIndex_;

    static const char kCRLF[];
};

} // namespace muduo_study






