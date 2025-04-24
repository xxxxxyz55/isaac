#ifndef __YS_VALIDATOR_H__
#define __YS_VALIDATOR_H__
#include <stdint.h>
#include <regex>
#include <string>

namespace isaac {

// 一、校验数字的表达式
// 数字：^[0-9]*$
// n位的数字：^\d{n}$
// 至少n位的数字：^\d{n,}$
// m-n位的数字：^\d{m,n}$
// 零和非零开头的数字：^(0|[1-9][0-9]*)$
// 非零开头的最多带两位小数的数字：^([1-9][0-9]*)+(\.[0-9]{1,2})?$
// 带1-2位小数的正数或负数：^(\-)?\d+(\.\d{1,2})$
// 正数、负数、和小数：^(\-|\+)?\d+(\.\d+)?$
// 有两位小数的正实数：^[0-9]+(\.[0-9]{2})?$
// 有1~3位小数的正实数：^[0-9]+(\.[0-9]{1,3})?$
// 非零的正整数：^[1-9]\d*$ 或 ^([1-9][0-9]*){1,3}$ 或 ^\+?[1-9][0-9]*$
// 非零的负整数：^\-[1-9][]0-9"*$ 或 ^-[1-9]\d*$
// 非负整数：^\d+$ 或 ^[1-9]\d*|0$
// 非正整数：^-[1-9]\d*|0$ 或 ^((-\d+)|(0+))$
// 非负浮点数：^\d+(\.\d+)?$ 或 ^[1-9]\d*\.\d*|0\.\d*[1-9]\d*|0?\.0+|0$
// 非正浮点数：^((-\d+(\.\d+)?)|(0+(\.0+)?))$ 或 ^(-([1-9]\d*\.\d*|0\.\d*[1-9]\d*))|0?\.0+|0$
// 正浮点数：^[1-9]\d*\.\d*|0\.\d*[1-9]\d*$ 或 ^(([0-9]+\.[0-9]*[1-9][0-9]*)|([0-9]*[1-9][0-9]*\.[0-9]+)|([0-9]*[1-9][0-9]*))$
// 负浮点数：^-([1-9]\d*\.\d*|0\.\d*[1-9]\d*)$ 或 ^(-(([0-9]+\.[0-9]*[1-9][0-9]*)|([0-9]*[1-9][0-9]*\.[0-9]+)|([0-9]*[1-9][0-9]*)))$
// 浮点数：^(-?\d+)(\.\d+)?$ 或 ^-?([1-9]\d*\.\d*|0\.\d*[1-9]\d*|0?\.0+|0)$
// 校验字符的表达式
// 汉字：^[\u4e00-\u9fa5]{0,}$
// 英文和数字：^[A-Za-z0-9]+$ 或 ^[A-Za-z0-9]{4,40}$
// 长度为3-20的所有字符：^.{3,20}$
// 由26个英文字母组成的字符串：^[A-Za-z]+$
// 由26个大写英文字母组成的字符串：^[A-Z]+$
// 由26个小写英文字母组成的字符串：^[a-z]+$
// 由数字和26个英文字母组成的字符串：^[A-Za-z0-9]+$
// 由数字、26个英文字母或者下划线组成的字符串：^\w+$ 或 ^\w{3,20}$
// 中文、英文、数字包括下划线：^[\u4E00-\u9FA5A-Za-z0-9_]+$
// 中文、英文、数字但不包括下划线等符号：^[\u4E00-\u9FA5A-Za-z0-9]+$ 或 ^[\u4E00-\u9FA5A-Za-z0-9]{2,20}$
// 可以输入含有^%&',;=?$\"等字符：[^%&',;=?$\x22]+
// 禁止输入含有~的字符：[^~]+
// 三、特殊需求表达式
// Email地址：^\w+([-+.]\w+)*@\w+([-.]\w+)*\.\w+([-.]\w+)*$
// 域名：[a-zA-Z0-9][-a-zA-Z0-9]{0,62}(\.[a-zA-Z0-9][-a-zA-Z0-9]{0,62})+\.?
// InternetURL：[a-zA-z]+://[^\s]* 或 ^http://([\w-]+\.)+[\w-]+(/[\w-./?%&=]*)?$
// 手机号码：^(13[0-9]|14[01456879]|15[0-35-9]|16[2567]|17[0-8]|18[0-9]|19[0-35-9])\d{8}$
// 电话号码("XXX-XXXXXXX"、"XXXX-XXXXXXXX"、"XXX-XXXXXXX"、"XXX-XXXXXXXX"、"XXXXXXX"和"XXXXXXXX)：^(\(\d{3,4}-)|\d{3.4}-)?\d{7,8}$
// 国内电话号码(0511-4405222、021-87888822)：\d{3}-\d{8}|\d{4}-\d{7}
// 电话号码正则表达式（支持手机号码，3-4位区号，7-8位直播号码，1－4位分机号）: ((\d{11})|^((\d{7,8})|(\d{4}|\d{3})-(\d{7,8})|(\d{4}|\d{3})-(\d{7,8})-(\d{4}|\d{3}|\d{2}|\d{1})|(\d{7,8})-(\d{4}|\d{3}|\d{2}|\d{1}))$)
// 身份证号(15位、18位数字)，最后一位是校验位，可能为数字或字符X：(^\d{15}$)|(^\d{18}$)|(^\d{17}(\d|X|x)$)
// 帐号是否合法(字母开头，允许5-16字节，允许字母数字下划线)：^[a-zA-Z][a-zA-Z0-9_]{4,15}$
// 密码(以字母开头，长度在6~18之间，只能包含字母、数字和下划线)：^[a-zA-Z]\w{5,17}$
// 强密码(必须包含大小写字母和数字的组合，不能使用特殊字符，长度在 8-10 之间)：^(?=.*\d)(?=.*[a-z])(?=.*[A-Z])[a-zA-Z0-9]{8,10}$
// 强密码(必须包含大小写字母和数字的组合，可以使用特殊字符，长度在8-10之间)：^(?=.*\d)(?=.*[a-z])(?=.*[A-Z]).{8,10}$
// 日期格式：^\d{4}-\d{1,2}-\d{1,2}
// 一年的12个月(01～09和1～12)：^(0?[1-9]|1[0-2])$
// 一个月的31天(01～09和1～31)：^((0?[1-9])|((1|2)[0-9])|30|31)$
// 钱的输入格式：
// 有四种钱的表示形式我们可以接受:"10000.00" 和 "10,000.00", 和没有 "分" 的 "10000" 和 "10,000"：^[1-9][0-9]*$
// 这表示任意一个不以0开头的数字,但是,这也意味着一个字符"0"不通过,所以我们采用下面的形式：^(0|[1-9][0-9]*)$
// 一个0或者一个不以0开头的数字.我们还可以允许开头有一个负号：^(0|-?[1-9][0-9]*)$
// 这表示一个0或者一个可能为负的开头不为0的数字.让用户以0开头好了.把负号的也去掉,因为钱总不能是负的吧。下面我们要加的是说明可能的小数部分：^[0-9]+(.[0-9]+)?$
// 必须说明的是,小数点后面至少应该有1位数,所以"10."是不通过的,但是 "10" 和 "10.2" 是通过的：^[0-9]+(.[0-9]{2})?$
// 这样我们规定小数点后面必须有两位,如果你认为太苛刻了,可以这样：^[0-9]+(.[0-9]{1,2})?$
// 这样就允许用户只写一位小数.下面我们该考虑数字中的逗号了,我们可以这样：^[0-9]{1,3}(,[0-9]{3})*(.[0-9]{1,2})?$
// 1到3个数字,后面跟着任意个 逗号+3个数字,逗号成为可选,而不是必须：^([0-9]+|[0-9]{1,3}(,[0-9]{3})*)(.[0-9]{1,2})?$
// 备注：这就是最终结果了,别忘了"+"可以用"*"替代如果你觉得空字符串也可以接受的话(奇怪,为什么?)最后,别忘了在用函数时去掉去掉那个反斜杠,一般的错误都在这里
// xml文件：^([a-zA-Z]+-?)+[a-zA-Z0-9]+\\.[x|X][m|M][l|L]$
// 中文字符的正则表达式：[\u4e00-\u9fa5]
// 双字节字符：[^\x00-\xff] (包括汉字在内，可以用来计算字符串的长度(一个双字节字符长度计2，ASCII字符计1))
// 空白行的正则表达式：\n\s*\r (可以用来删除空白行)
// HTML标记的正则表达式：<(\S*?)[^>]*>.*?|<.*? /> ( 首尾空白字符的正则表达式：^\s*|\s*$或(^\s*)|(\s*$) (可以用来删除行首行尾的空白字符(包括空格、制表符、换页符等等)，非常有用的表达式)
// 腾讯QQ号：[1-9][0-9]{4,} (腾讯QQ号从10000开始)
// 中国邮政编码：[1-9]\d{5}(?!\d) (中国邮政编码为6位数字)
// IPv4地址：((2(5[0-5]|[0-4]\d))|[0-1]?\d{1,2})(\.((2(5[0-5]|[0-4]\d))|[0-1]?\d{1,2})){3}

// 注意，接口允许空字符串
class YS_Regex
{
public:
    // 长度 在 [min,max]之间
    static bool length(const std::string &str, uint32_t min, uint32_t max)
    {
        return (str.length() >= min && str.length() <= max);
    }

    // 长度小于等于
    static bool length(const std::string &str, uint32_t max)
    {
        return str.length() <= max;
    }

    // 只包含数字
    static bool num(const std::string &str)
    {
        static std::regex reg("^[0-9]*$");
        return std::regex_match(str, reg);
    }

    // 字母和数字
    static bool schar(const std::string &str)
    {
        static std::regex reg(R"(^[A-Za-z0-9]*$)");
        return std::regex_match(str, reg);
    }

    // 可见字符
    static bool vchar(const std::string &str)
    {
        for (auto &&c : str)
        {
            if (c < ' ' || c > '~') return false;
        }
        return true;
    }

    // 不包含chs中字符
    static bool except(const std::string &str, const char *chs)
    {
        for (size_t i = 0; i < strlen(chs); i++)
        {
            if (str.find(chs[i]) != std::string::npos)
            {
                return false;
            }
        }
        return true;
    }

    // 不包含vt中字符串
    static bool except(const std::string &str, const std::vector<std::string> &vtChs)
    {
        for (auto &&iter : vtChs)
        {
            if (str.find(iter) != std::string::npos)
            {
                return false;
            }
        }
        return true;
    }
    // url
    static bool url(const std::string &str)
    {
        if (str.empty()) return true;
        static std::regex reg(R"(\b((?:https?|ftp):\/\/[-a-zA-Z0-9+&@#\/%?=~_|!:,.;]*[-a-zA-Z0-9+&@#\/%=~_|])\b)");
        return std::regex_match(str, reg);
    }
    // 5-16位字母数字下划线
    static bool user(const std::string &str)
    {
        if (str.empty()) return true;
        static std::regex reg(R"(^[a-zA-Z][a-zA-Z0-9_]{4,15}$)");
        return std::regex_match(str, reg);
    }
    // 5-16位可见字符
    static bool pwdEasy(const std::string &str)
    {
        if (str.empty()) return true;
        return vchar(str) && length(str, 5, 16);
    }

    // 8-16位大小写加数字，可包含特殊字符
    static bool pwdHard(const std::string &str)
    {
        if (str.empty()) return true;
        static std::regex reg(R"(^(?=.*\d)(?=.*[a-z])(?=.*[A-Z]).{8,16}$)");
        return std::regex_match(str, reg);
    }

    static bool ipv4(const std::string &str)
    {
        if (str.empty()) return true;
        static std::regex reg(R"(((2(5[0-5]|[0-4]\d))|[0-1]?\d{1,2})(\.((2(5[0-5]|[0-4]\d))|[0-1]?\d{1,2})){3})");
        return std::regex_match(str, reg);
    }

    static bool ipv6(const std::string &str)
    {
        if (str.empty()) return true;
        static std::regex reg(R"(^(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:)|fe80:(:[0-9a-fA-F]{0,4}){0,4}%[0-9a-zA-Z]{1,}|::(ffff(:0{1,4}){0,1}:){0,1}((25[0-5]|(2[0-4]|1{0,1}[0-9]|[1-9])?[0-9])\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]|[1-9])?[0-9])|([0-9a-fA-F]{1,4}:){1,4}:((25[0-5]|(2[0-4]|1{0,1}[0-9]|[1-9])?[0-9])\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]|[1-9])?[0-9]))$)");
        return std::regex_match(str, reg);
    }

    static bool ip(const std::string &str)
    {
        return ipv4(str) || ipv6(str);
    }

    static bool range(uint64_t val, uint64_t min, uint64_t max)
    {
        return val >= min && val <= max;
    }

    // 备注，允许字母数字中文,_.@!
    static bool remarks(const std::string &str)
    {
        if (str.empty()) return true;
        static std::regex reg("^[\u4E00-\u9FA5A-Za-z0-9_,.?@!]+$");
        return std::regex_match(str, reg);
    }

    // 备注，允许字母数字中文,_.@!
    static bool fileName(const std::string &str)
    {
        static std::regex reg("^[\u4E00-\u9FA5A-Za-z0-9_.]+$");
        return std::regex_match(str, reg);
    }

    static bool inject(const std::string &str)
    {
        if (str.empty()) return true;
        return except(str, std::vector<std::string>{"--"}) &&
               except(str, "; (&|><'\"{");
    }
};

class YS_Valid
{
    YS_Valid &check(bool val)
    {
        if (!val) throw -1;
        return *this;
    }

public:
    // 长度 在 [min,max]之间
    YS_Valid &length(const std::string &str, uint32_t min, uint32_t max)
    {
        return check(YS_Regex::length(str, min, max));
    }
    // 长度小于
    YS_Valid &length(const std::string &str, uint32_t max)
    {
        return check(YS_Regex::length(str, max));
    }

    // 只包含数字
    YS_Valid &num(const std::string &str)
    {
        return check(YS_Regex::num(str));
    }

    // 字母和数字
    YS_Valid &schar(const std::string &str)
    {
        return check(YS_Regex::schar(str));
    }

    // 可见字符
    YS_Valid &vchar(const std::string &str)
    {
        return check(YS_Regex::vchar(str));
    }

    // 不包含chs中字符
    YS_Valid &except(const std::string &str, const char *chs)
    {
        return check(YS_Regex::except(str, chs));
    }

    // 不包含v中字符串
    YS_Valid &except(const std::string &str, const std::vector<std::string> &vtChs)
    {
        return check(YS_Regex::except(str, vtChs));
    }

    // url
    YS_Valid &url(const std::string &str)
    {
        return check(YS_Regex::url(str));
    }
    // 5-16位字母数字下划线
    YS_Valid &user(const std::string &str)
    {
        return check(YS_Regex::user(str));
    }
    // 5-16位可见字符
    YS_Valid &pwdEasy(const std::string &str)
    {
        return check(YS_Regex::pwdEasy(str));
    }

    // 8-16位大小写加数字，可包含特殊字符
    YS_Valid &pwdHard(const std::string &str)
    {
        return check(YS_Regex::pwdHard(str));
    }

    YS_Valid &ipv4(const std::string &str)
    {
        return check(YS_Regex::ipv4(str));
    }

    YS_Valid &ipv6(const std::string &str)
    {
        return check(YS_Regex::ipv6(str));
    }

    YS_Valid &ip(const std::string &str)
    {
        return check(YS_Regex::ip(str));
    }

    YS_Valid &range(uint64_t val, uint64_t min, uint64_t max)
    {
        return check(YS_Regex::range(val, min, max));
    }

    YS_Valid &remarks(const std::string &str)
    {
        return check(YS_Regex::remarks(str));
    }

    YS_Valid &fileName(const std::string &str)
    {
        return check(YS_Regex::fileName(str));
    }
    YS_Valid &inject(const std::string &str)
    {
        return check(YS_Regex::inject(str));
    }
};

class YS_Verify
{
    virtual void validator(YS_Valid &r) = 0;

public:
    int32_t verify()
    {
        try
        {
            YS_Valid r;
            validator(r);
        }
        catch (int32_t ret)
        {
            return ret;
        }
        return 0;
    }
};

} // namespace isaac
#endif