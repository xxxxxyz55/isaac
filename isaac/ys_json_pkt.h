#ifndef _YS_JSON_PKT_H_
#define _YS_JSON_PKT_H_

#include <vector>
#include <memory>
#include "xpack/rapidjson/document.h"
#include "xpack/rapidjson/writer.h"

namespace isaac {

// json结构构造  依赖rapidjson
class YS_JsonPkt
{
private:
    std::vector<std::shared_ptr<rapidjson::Document>> _vtDoc;

public:
    YS_JsonPkt()
    {
        _vtDoc.push_back(std::make_shared<rapidjson::Document>(rapidjson::kObjectType));
    }

    template <typename T>
    void add_field(const std::string key, T val)
    {
        return add_field(key.c_str(), val, true);
    }

    // 如果key 不在静态区 需要copy key
    template <typename T>
    void add_field(const char *key, T val, bool copyKey = false)
    {
        auto pDoc = _vtDoc[0];
        if (copyKey)
        {
            rapidjson::Value name(key, pDoc->GetAllocator());
            pDoc->AddMember(name, val, pDoc->GetAllocator());
        }
        else
        {
            pDoc->AddMember((rapidjson::GenericStringRef<char>)key, val, pDoc->GetAllocator());
        }
    }

    void add_field(const char *key, const char *val, bool copyKey = false)
    {
        auto pDoc = _vtDoc[0];
        if (copyKey)
        {
            rapidjson::Value name(key, pDoc->GetAllocator());
            pDoc->AddMember(name, rapidjson::Value().SetString(val, strlen(val), pDoc->GetAllocator()), pDoc->GetAllocator());
        }
        else
        {
            pDoc->AddMember((rapidjson::GenericStringRef<char>)key, rapidjson::Value().SetString(val, strlen(val), pDoc->GetAllocator()), pDoc->GetAllocator());
        }
    }

    void add_field(const char *key, char *val, bool copyKey = false)
    {
        auto pDoc = _vtDoc[0];
        if (copyKey)
        {
            rapidjson::Value name(key, pDoc->GetAllocator());
            pDoc->AddMember(name, rapidjson::Value().SetString(val, strlen(val), pDoc->GetAllocator()), pDoc->GetAllocator());
        }
        else
        {
            pDoc->AddMember((rapidjson::GenericStringRef<char>)key, rapidjson::Value().SetString(val, strlen(val), pDoc->GetAllocator()), pDoc->GetAllocator());
        }
    }

    void add_field(const char *key, YS_JsonPkt &obj, bool copyKey = false)
    {
        for (auto &&iter : obj._vtDoc)
        {
            _vtDoc.push_back(iter);
        }

        auto pDoc = _vtDoc[0];
        if (copyKey == false)
        {
            pDoc->AddMember((rapidjson::GenericStringRef<char>)key, obj._vtDoc[0]->GetObj(), pDoc->GetAllocator());
        }
        else
        {
            rapidjson::Value name(key, pDoc->GetAllocator());
            pDoc->AddMember(name, obj._vtDoc[0]->GetObj(), pDoc->GetAllocator());
        }
    }

    void add_field(const char *key, std::vector<YS_JsonPkt> &objs, bool copyKey = false)
    {
        auto pDoc = _vtDoc[0];

        for (auto &&obj : objs)
        {
            for (auto &&iter : obj._vtDoc)
            {
                _vtDoc.push_back(iter);
            }
        }

        rapidjson::Value tVal(rapidjson::kArrayType);
        for (auto &&iter : objs)
        {
            tVal.PushBack(iter._vtDoc[0]->GetObj(), pDoc->GetAllocator());
        }
        
        if (copyKey == false)
        {
            pDoc->AddMember((rapidjson::GenericStringRef<char>)key, tVal, pDoc->GetAllocator());
        }
        else
        {
            rapidjson::Value name(key, pDoc->GetAllocator());
            pDoc->AddMember(name, tVal, pDoc->GetAllocator());
        }
    }

    void add_field(const char *key, const std::string val, bool copyKey = false)
    {
        return add_field(key, val.c_str(), copyKey);
    }

    void add_field(const char *key, std::vector<const char *> val, bool copyKey = false)
    {
        auto             pDoc = _vtDoc[0];
        rapidjson::Value tVal(rapidjson::kArrayType);
        for (auto &&iter : val)
        {
            tVal.PushBack((rapidjson::GenericStringRef<char>)iter, pDoc->GetAllocator());
        }

        if (copyKey)
        {
            rapidjson::Value name(key, pDoc->GetAllocator());
            pDoc->AddMember(name, tVal, pDoc->GetAllocator());
        }
        else
        {
            pDoc->AddMember((rapidjson::GenericStringRef<char>)key, tVal, pDoc->GetAllocator());
        }
    }

    void add_field(const char *key, std::vector<std::string> val, bool copyKey = false)
    {
        auto             pDoc = _vtDoc[0];
        rapidjson::Value tVal(rapidjson::kArrayType);
        for (auto &&iter : val)
        {
            tVal.PushBack(rapidjson::Value().SetString(iter.c_str(), iter.length(), pDoc->GetAllocator()), pDoc->GetAllocator());
        }

        if (copyKey)
        {
            rapidjson::Value name(key, pDoc->GetAllocator());
            pDoc->AddMember(name, tVal, pDoc->GetAllocator());
        }
        else
        {
            pDoc->AddMember((rapidjson::GenericStringRef<char>)key, tVal, pDoc->GetAllocator());
        }
    }

    void add_field(const char *key, std::vector<int32_t> &val, bool copyKey = false)
    {
        auto             pDoc = _vtDoc[0];
        rapidjson::Value tVal(rapidjson::kArrayType);
        for (auto &&iter : val)
        {
            tVal.PushBack(iter, pDoc->GetAllocator());
        }

        if (copyKey)
        {
            rapidjson::Value name(key, pDoc->GetAllocator());
            pDoc->AddMember(name, tVal, pDoc->GetAllocator());
        }
        else
        {
            pDoc->AddMember((rapidjson::GenericStringRef<char>)key, tVal, pDoc->GetAllocator());
        }
    }

    void add_field(const char *key, std::vector<uint32_t> &val, bool copyKey = false)
    {
        auto             pDoc = _vtDoc[0];
        rapidjson::Value tVal(rapidjson::kArrayType);
        for (auto &&iter : val)
        {
            tVal.PushBack(iter, pDoc->GetAllocator());
        }
        if (copyKey)
        {
            rapidjson::Value name(key, pDoc->GetAllocator());
            pDoc->AddMember(name, tVal, pDoc->GetAllocator());
        }
        else
        {
            pDoc->AddMember((rapidjson::GenericStringRef<char>)key, tVal, pDoc->GetAllocator());
        }
    }

    void add_obj(const char *key, std::string &val)
    {
        auto                pDoc = _vtDoc[0];
        rapidjson::Document doc{rapidjson::kObjectType, &pDoc->GetAllocator()};
        rapidjson::Value    name(key, pDoc->GetAllocator());

        doc.Parse(val.c_str(), val.length());
        pDoc->AddMember(name, doc, pDoc->GetAllocator());
    }

    void getJsonStr(char *buf, uint32_t max)
    {
        rapidjson::StringBuffer                    buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        auto                                       pDoc = _vtDoc[0];
        pDoc->Accept(writer);
        snprintf(buf, max, "%s", buffer.GetString());
    }

    std::string toJsonStr()
    {
        rapidjson::StringBuffer                    buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

        auto pDoc = _vtDoc[0];
        pDoc->Accept(writer);
        return std::string(buffer.GetString());
    }

    ~YS_JsonPkt()
    {
        _vtDoc.clear();
    }

    static std::string arrToJStr(std::vector<YS_JsonPkt> &vtPkt)
    {
        rapidjson::Document doc(rapidjson::kArrayType);
        for (auto &&iter : vtPkt)
        {
            doc.PushBack(iter._vtDoc[0]->GetObj(), doc.GetAllocator());
        }

        rapidjson::StringBuffer                    buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);
        return std::string(buffer.GetString());
    }
};

}


#endif