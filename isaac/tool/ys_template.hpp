#ifndef __YS_TEMPLATE_H__
#define __YS_TEMPLATE_H__

namespace isaac {

// 获取函数参数类型
template <typename T>
struct YS_ArgType;

template <typename Ret, typename Arg1, typename Arg2, typename... Args>
struct YS_ArgType<Ret (*)(Arg1, Arg2, Args...)>
{
    using type1   = Arg1;
    using type2   = Arg2;
    using typeRet = Ret;
};

template <typename T, typename Ret, typename Arg1, typename Arg2, typename... Args>
struct YS_ArgType<Ret (T::*)(Arg1, Arg2, Args...)>
{
    using type1   = Arg1;
    using type2   = Arg2;
    using typeRet = Ret;
};

} // namespace isaac

#endif