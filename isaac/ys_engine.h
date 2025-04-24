#ifndef _YS_ENGINE_H_
#define _YS_ENGINE_H_

#include <vector>
#include <map>

namespace isaac {

class YS_NetBuf;

// map 用于指令码找处理函数
template <typename K, typename F>
class YS_Engine
{
private:
    std::map<K, F> _engine;

public:
    void route(std::vector<std::pair<K, F>> vtRouter)
    {
        for (auto &&i : vtRouter)
        {
            _engine.insert(i);
        }
    }

    YS_Engine(std::vector<std::pair<K, F>> vtRouter) { route(vtRouter); }

    virtual void done(YS_NetBuf &buffer) {}

    F get(const K &key)
    {
        auto iter = _engine.find(key);
        if (iter != _engine.end())
        {
            return iter->second;
        }
        else
        {
            return nullptr;
        }
    }

    YS_Engine() {}
    ~YS_Engine() { _engine.clear(); }
};

} // namespace isaac

#endif