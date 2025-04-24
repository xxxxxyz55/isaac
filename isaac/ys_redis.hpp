#ifndef _YS_REDIS_HPP_
#define _YS_REDIS_HPP_

#include "hiredis/hiredis.h"
#include "ys_cli_log.h"
#include <map>

namespace isaac {
// hiredis封装

// redis执行结果
class YS_RdsRpy
{
    redisReply *_rep;

public:
    // 构造
    YS_RdsRpy(void *rep) { _rep = (redisReply *)rep; }
    // 析构
    ~YS_RdsRpy();
    // 拷贝构造
    YS_RdsRpy(YS_RdsRpy &&rep)
    {
        _rep     = rep._rep;
        rep._rep = nullptr;
    }

    // null error 或nil
    static bool isError(redisReply *rpy);
    bool        isError() { return isError(_rep); }

    // get
    std::string getStr();

    // hmapget
    std::vector<std::string> getRows();

    operator redisReply *() { return _rep; }
    redisReply *operator->() { return _rep; }
};

// redis连接
class YS_RdsConn
{
    redisContext *_conn = nullptr;

public:
    YS_RdsConn() {}
    // 构造加连接加认证
    YS_RdsConn(const std::string &host, uint16_t port, const std::string &pwd = "")
    {
        if (connect(host, port, pwd)) close();
    }

    ~YS_RdsConn() { close(); }
    // 连接成功
    bool isOk() { return _conn != nullptr; }
    // 连接
    int32_t connect(const std::string &host, uint16_t port, const std::string &pwd = "");
    // 断开
    void close();
    // 认证
    int32_t auth(std::string pwd);

    // get命令
    std::string get(const std::string &key);

    // hmget命令
    std::vector<std::string> hmget(const std::string &key, const std::string &fields);

    // hgetall 命令
    std::map<std::string, std::string> hgetall(const std::string &key);

    // 执行一条完整命令，获取结果
    YS_RdsRpy query(const std::string &cmd);

    // 执行一条分开的命令，获取结果,支持二进制
    YS_RdsRpy query(const std::vector<std::string> &argv);

    // 执行一条完整命令
    int32_t exec(const std::string &cmd);

    // 执行一条分开的命令,支持二进制
    int32_t exec(const std::vector<std::string> &cmd);
};

// 连接池
class YS_RdsPool
{
public:
    // 构造,执行命令才会连接
    YS_RdsPool(const std::string &host, uint16_t port, const std::string &pwd = "")
        : _host(host), _port(port), _pwd(pwd)
    {
    }

    // 析构,关闭连接
    ~YS_RdsPool();
    // get 命令
    std::string get(const std::string &key);
    // hmget 命令
    std::vector<std::string> hmget(const std::string &key, const std::string &fields);
    // hgetall命令
    std::map<std::string, std::string> hgetall(const std::string &key);
    // 执行完整语句,获取结果
    YS_RdsRpy query(const std::string &cmd);
    // 执行分割的语句,获取结果,支持二进制
    YS_RdsRpy query(const std::vector<std::string> &cmd);
    // 执行语句
    int32_t exec(const std::string &cmd);
    // 执行分割的语句,支持二进制
    int32_t exec(const std::vector<std::string> &cmd);

private:
    YS_ThQueue<YS_RdsConn *> _que;
    std::string              _host;
    uint16_t                 _port;
    std::string              _pwd;
    // 获取
    YS_RdsConn *pop();
    // 释放
    void push(YS_RdsConn *conn) { _que.push_back(conn); }

    // 构造是获取连接，析构时放回pool
    class UniqueConn
    {
        YS_RdsConn *_conn;
        YS_RdsPool &_pool;

    public:
        UniqueConn(YS_RdsPool &pool) : _pool(pool) { _conn = pool.pop(); }
        ~UniqueConn()
        {
            if (_conn) _pool.push(_conn);
        }

        operator YS_RdsConn *() { return _conn; }
        YS_RdsConn *operator->() { return _conn; }
    };
};

inline bool YS_RdsRpy::isError(redisReply *rpy)
{
    if (rpy == nullptr)
    {
        CLI_ERROR("redis command exec fail.");
        return true;
    }
    if (rpy->type == REDIS_REPLY_ERROR)
    {
        CLI_ERROR("redis reply error, {}", rpy->str);
        return true;
    }

    if (rpy->type == REDIS_REPLY_NIL)
    {
        CLI_DEBUG("redis reply nil.");
        return true;
    }
    return false;
}

inline std::string YS_RdsRpy::getStr()
{
    if (isError())
    {
        return "";
    }

    if (_rep->type != REDIS_REPLY_STRING)
    {
        CLI_ERROR("reply type is not string, type = {}", _rep->type);
        return "";
    }
    return std::string(_rep->str, _rep->len);
}

inline std::vector<std::string> YS_RdsRpy::getRows()
{
    std::vector<std::string> res;
    if (isError(_rep))
    {
        return res;
    }

    if (_rep->type != REDIS_REPLY_ARRAY)
    {
        CLI_ERROR("reply is`nt array, type = {}", _rep->type);
        return res;
    }

    for (size_t i = 0; i < _rep->elements; i++)
    {
        if (_rep->element[i]->type == REDIS_REPLY_STRING)
        {
            std::string item = _rep->element[i]->str;
            res.emplace_back(item);
        }
        else if (_rep->element[i]->type == REDIS_REPLY_NIL)
        {
            res.emplace_back("");
        }
        else
        {
            CLI_ERROR("not support element type = {}.", _rep->element[i]->type);
            return {};
        }
    }
    return res;
}

inline YS_RdsRpy::~YS_RdsRpy()
{
    if (_rep)
    {
        freeReplyObject(_rep);
        _rep = nullptr;
    }
}

inline int32_t YS_RdsConn::connect(const std::string &host, uint16_t port, const std::string &pwd)
{
    if (_conn)
    {
        close();
    }

    _conn = redisConnectWithTimeout(host.c_str(), port, {1, 0});
    if (!_conn)
    {
        CLI_ERROR("redis conn fail, {}:{}", host, port);
        return -1;
    }
    if (_conn->err)
    {
        CLI_ERROR("redis conn fail, {}", _conn->errstr);
        redisFree(_conn);
        _conn = nullptr;
        return _conn->err;
    }

    if (!pwd.empty())
    {
        return auth(pwd);
    }
    return 0;
}

inline void YS_RdsConn::close()
{
    redisFree(_conn);
    _conn = nullptr;
}

inline int32_t YS_RdsConn::auth(std::string pwd)
{
    if (!pwd.empty())
    {
        std::string cmd = "AUTH " + pwd;
        YS_RdsRpy   rds_reply(redisCommand(_conn, cmd.c_str()));
        if (rds_reply == nullptr)
        {
            CLI_ERROR("redis auth reply null, {}", _conn->errstr);
            return -1;
        }

        if (rds_reply->type == REDIS_REPLY_ERROR)
        {
            CLI_ERROR("redis auth error, err {}, {}", rds_reply->str, _conn->errstr);
            return -1;
        }
    }
    return 0;
};

inline std::string YS_RdsConn::get(const std::string &key)
{
    YS_RdsRpy rpy = query(fmt::format("GET {}", key));
    return rpy.getStr();
}

inline std::vector<std::string> YS_RdsConn::hmget(const std::string &key, const std::string &fields)
{
    YS_RdsRpy rpy = query(fmt::format("HMGET {} {}", key, fields));
    return rpy.getRows();
}

inline std::map<std::string, std::string> YS_RdsConn::hgetall(const std::string &key)
{
    YS_RdsRpy rpy  = query(fmt::format("HGETALL {}", key));
    auto      rows = rpy.getRows();

    std::map<std::string, std::string> mp;
    if (rows.size())
    {
        for (size_t i = 0; i < rows.size(); i++)
        {
            mp[rows[i]] = mp[rows[i + 1]];
            i++;
        }
    }
    return mp;
}

inline YS_RdsRpy YS_RdsConn::query(const std::string &cmd)
{
    if (_conn == nullptr)
    {
        CLI_ERROR("call connect first before exec.");
        return YS_RdsRpy(nullptr);
    }
    YS_RdsRpy rpy(redisCommand(_conn, cmd.c_str()));

    if (rpy == nullptr && (_conn->err == 1 || _conn->err == 3))
    {
        if (redisReconnect(_conn) == REDIS_OK)
        {
            return query(cmd);
        }
    }
    return rpy;
}

inline YS_RdsRpy YS_RdsConn::query(const std::vector<std::string> &argv)
{
    if (_conn == nullptr)
    {
        CLI_ERROR("call connect first before exec.");
        return YS_RdsRpy(nullptr);
    }
    if (!argv.size())
    {
        CLI_ERROR("empty argv.");
        return YS_RdsRpy(nullptr);
    }
    const char **pArg = new const char *[argv.size()];
    size_t      *pLen = new size_t[argv.size()];
    for (size_t i = 0; i < argv.size(); i++)
    {
        pArg[i] = argv[i].c_str();
        pLen[i] = argv[i].length();
    }

    YS_RdsRpy rpy(redisCommandArgv(_conn, argv.size(), pArg, pLen));

    if (rpy == nullptr && (_conn->err == 1 || _conn->err == 3))
    {
        if (redisReconnect(_conn) == REDIS_OK)
        {
            delete[] pArg;
            delete[] pLen;
            return query(argv);
        }
    }
    delete[] pArg;
    delete[] pLen;
    return rpy;
}

inline int32_t YS_RdsConn::exec(const std::string &cmd)
{
    YS_RdsRpy rpy = query(cmd);
    if (rpy == nullptr)
    {
        if (_conn)
        {
            CLI_DEBUG("redis cmd exec fail ,{}", cmd, _conn->err);
        }
        return -1;
    }

    if (rpy->type == REDIS_REPLY_ERROR)
    {
        CLI_DEBUG("redis cmd exec fail ,{}, {}", cmd, rpy->str);
        return -1;
    }
    return 0;
}

inline int32_t YS_RdsConn::exec(const std::vector<std::string> &cmd)
{
    YS_RdsRpy rpy = query(cmd);
    if (rpy == nullptr)
    {
        if (_conn)
        {
            CLI_DEBUG("redis cmd exec fail ,{}", YS_String::fmtVector(cmd), _conn->err);
        }
        return -1;
    }

    if (rpy->type == REDIS_REPLY_ERROR)
    {
        CLI_DEBUG("redis cmd exec fail ,{}, {}", YS_String::fmtVector(cmd), rpy->str);
        return -1;
    }
    return 0;
}

inline YS_RdsPool::~YS_RdsPool()
{
    YS_RdsConn *pConn = nullptr;
    while (_que.pop_front(pConn, 0, false))
    {
        delete pConn;
    }
}

inline std::string YS_RdsPool::get(const std::string &key)
{
    UniqueConn conn(*this);
    if (conn) return conn->get(key);
    return {};
}

inline std::vector<std::string> YS_RdsPool::hmget(const std::string &key, const std::string &fields)
{
    UniqueConn conn(*this);
    if (conn) return conn->hmget(key, fields);
    return {};
}

inline std::map<std::string, std::string> YS_RdsPool::hgetall(const std::string &key)
{
    UniqueConn conn(*this);
    if (conn) return conn->hgetall(key);
    return {};
}

inline YS_RdsRpy YS_RdsPool::query(const std::string &cmd)
{
    UniqueConn conn(*this);
    if (conn) return conn->query(cmd);
    return nullptr;
}

inline YS_RdsRpy YS_RdsPool::query(const std::vector<std::string> &cmd)
{
    UniqueConn conn(*this);
    if (conn) return conn->query(cmd);
    return nullptr;
}

inline int32_t YS_RdsPool::exec(const std::string &cmd)
{
    UniqueConn conn(*this);
    if (conn) return conn->exec(cmd);
    return -1;
}

inline int32_t YS_RdsPool::exec(const std::vector<std::string> &cmd)
{
    UniqueConn conn(*this);
    if (conn) return conn->exec(cmd);
    return -1;
}

inline YS_RdsConn *YS_RdsPool::pop()
{
    YS_RdsConn *conn = nullptr;
    if (_que.pop_front(conn, 0, false))
    {
        return conn;
    }

    conn = new YS_RdsConn(_host, _port, _pwd);
    if (conn->isOk())
    {
        CLI_DEBUG("new rds conn ok.");
        return conn;
    }
    CLI_DEBUG("new rds conn fail.");
    return nullptr;
}

}; // namespace isaac
#endif