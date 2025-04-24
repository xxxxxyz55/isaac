#ifndef _YS_HANDLE_H_
#define _YS_HANDLE_H_

#include "ys_socket.h"
#include "tool/ys_buffer.hpp"

namespace isaac {

// 接收上下文，如http解析
class YS_PktCtx
{
public:
    virtual ~YS_PktCtx() {}
};
using YS_FullPktCb = int32_t (*)(uint8_t *, uint32_t, YS_PktCtx **);

// 网络缓冲区
// 默认大小32k，超过请分包
class YS_NetBuf
{
public:
    YS_Buf                recvData;    // 待接收数据
    YS_Buf                sendData;    // 待发送数据
    YS_SocketPtr          refSock;     // 对应的socket
    uint32_t              tid;         // 业务线程ID
    YS_PktCtx            *ctx;         // 协议对应的包,根据需求设置
    std::function<void(YS_NetBuf *)> cbBusiFinal; // 处理完成回调

    static constexpr size_t capacity() { return YS_Buf::size32KB(); }

    // 构造 接收发送默认申请capacity大小的空间
    YS_NetBuf();
    // 清空
    void clear();
    // 析构,ctx 不为nullptr则delete
    ~YS_NetBuf();
};

// rpc驱动核心
class YS_Driver
{
public:
    bool supportBlock;
    // 业务线程初始化
    virtual void init() {}
    // 输入请求，输出响应
    virtual void handle(YS_NetBuf &buf) = 0;
    // 业务线程结束
    virtual void final() {}
    YS_Driver(bool isSupportBlock = false) : supportBlock(isSupportBlock) {}

    // 接收到一个包
    // 大于0继续接收，小于0包错误
    virtual int32_t isFullPkt(YS_Buf &buf, isaac::YS_PktCtx **)
    {
        if (buf.empty()) return buf.capacity();
        return 0;
    }
};
using YS_DriverPtr = std::shared_ptr<YS_Driver>;

inline YS_NetBuf::YS_NetBuf()
{
    recvData.alloc(capacity());
    sendData.alloc(capacity());
    refSock     = nullptr;
    ctx         = nullptr;
    cbBusiFinal = nullptr;
}

inline void YS_NetBuf::clear()
{
    recvData.clear();
    sendData.clear();
    refSock = nullptr;
    cbBusiFinal = nullptr;
    if (ctx)
    {
        delete ctx;
        ctx = nullptr;
    }
}

inline YS_NetBuf::~YS_NetBuf()
{
    if (ctx) delete ctx;
}

}; // namespace isaac

#endif