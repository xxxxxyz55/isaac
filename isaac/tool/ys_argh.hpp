#pragma once
#ifndef _YS_ARGH_H_
#define _YS_ARGH_H_

#include <algorithm>
#include <sstream>
#include <limits>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <cassert>

namespace isaac {
// Terminology:
// A command line is composed of 2 types of args:
// 1. Positional args, i.e. free standing values
// 2. Options: args beginning with '-'. We identify two kinds:
//    2.1: Flags: boolean options =>  (exist ? true : false)
//    2.2: Parameters: a name followed by a non-option value

class multimap_iteration_wrapper
{
public:
    using container_t  = std::multimap<std::string, std::string>;
    using iterator_t   = container_t::const_iterator;
    using difference_t = container_t::difference_type;
    explicit multimap_iteration_wrapper(const iterator_t &lb, const iterator_t &ub)
        : lb_(lb), ub_(ub)
    {
    }

    iterator_t   begin() const { return lb_; }
    iterator_t   end() const { return ub_; }
    difference_t size() const { return std::distance(lb_, ub_); }

private:
    iterator_t lb_;
    iterator_t ub_;
};

/**
 * param eg: -p port 使用()取值
 * flag eg: -h
 * opt  eg:  daemon
 */
class YS_ArgParser
{
public:
    enum Mode
    {
        PREFER_FLAG_FOR_UNREG_OPTION  = 1 << 0,
        PREFER_PARAM_FOR_UNREG_OPTION = 1 << 1,
        NO_SPLIT_ON_EQUALSIGN         = 1 << 2,
        SINGLE_DASH_IS_MULTIFLAG      = 1 << 3,
    };

    YS_ArgParser() = default;

    YS_ArgParser(std::initializer_list<char const *const> pre_reg_names)
    {
        add_params(pre_reg_names);
    }

    YS_ArgParser(const char *const argv[], int mode = PREFER_PARAM_FOR_UNREG_OPTION)
    {
        parse(argv, mode);
    }

    YS_ArgParser(int argc, const char *const argv[], int mode = PREFER_PARAM_FOR_UNREG_OPTION)
    {
        parse(argc, argv, mode);
    }

    void add_param(std::string const &name);
    void add_params(std::string const &name);

    void add_param(std::initializer_list<char const *const> init_list);
    void add_params(std::initializer_list<char const *const> init_list);

    void parse(const char *const argv[], int mode = PREFER_PARAM_FOR_UNREG_OPTION);
    void parse(int argc, const char *const argv[], int mode = PREFER_PARAM_FOR_UNREG_OPTION);

    std::multiset<std::string> const              &flags() const { return flags_; }
    std::multimap<std::string, std::string> const &params() const { return params_; }
    multimap_iteration_wrapper                     params(std::string const &name) const;
    std::vector<std::string> const                &pos_args() const { return pos_args_; }

    // begin() and end() for using range-for over positional args.
    std::vector<std::string>::const_iterator begin() const { return pos_args_.cbegin(); }
    std::vector<std::string>::const_iterator end() const { return pos_args_.cend(); }
    size_t                                   size() const { return pos_args_.size(); }

    //////////////////////////////////////////////////////////////////////////
    // Accessors

    // flag (boolean) accessors: return true if the flag appeared, otherwise false.
    bool operator[](std::string const &name) const;

    // multiple flag (boolean) accessors: return true if at least one of the flag appeared, otherwise false.
    bool operator[](std::initializer_list<char const *const> init_list) const;

    // returns positional arg string by order. Like argv[] but without the options
    std::string const &operator[](size_t ind) const;

    // returns a std::istream that can be used to convert a positional arg to a typed value.
    std::string operator()(size_t ind) const;

    // same as above, but with a default value in case the arg is missing (index out of range).
    template <typename T>
    std::string operator()(size_t ind, T &&def_val) const;

    // parameter accessors, give a name get an std::istream that can be used to convert to a typed value.
    // call .str() on result to get as string
    std::string operator()(std::string const &name) const;

    // accessor for a parameter with multiple names, give a list of names, get an std::istream that can be used to convert to a typed value.
    // call .str() on result to get as string
    // returns the first value in the list to be found.
    std::string operator()(std::initializer_list<char const *const> init_list) const;

    // same as above, but with a default value in case the param was missing.
    // Non-string def_val types must have an operator<<() (output stream operator)
    // If T only has an input stream operator, pass the string version of the type as in "3" instead of 3.
    template <typename T>
    std::string operator()(std::string const &name, T &&def_val) const;

    // same as above but for a list of names. returns the first value to be found.
    template <typename T>
    std::string operator()(std::initializer_list<char const *const> init_list, T &&def_val) const;

    bool hasParam(std::string const &name) const { return got_opt(name); }
    bool hasParam(std::initializer_list<char const *const> init_list) const
    {
        return std::any_of(init_list.begin(), init_list.end(), [&](char const *const name) { return got_opt(name); });
    }

    bool hasOpt(std::string const &name) const
    {
        auto tname = trim_leading_dashes(name);
        for (auto &&iter : pos_args())
        {
            if (iter == tname) return true;
        }
        return false;
    }
    bool hasOpt(std::initializer_list<char const *const> init_list) const
    {
        return std::any_of(init_list.begin(), init_list.end(), [&](char const *const name) { return hasOpt(name); });
    }

    std::string opt(std::string const &name) const
    {
        return operator()(name);
    }

    std::string opt(std::initializer_list<char const *const> init_list) const
    {
        return operator()(init_list);
    }
    // flag eg: -h -g
    bool hasFlag(std::string const &name) const { return got_flag(name); }
    bool hasFlag(std::initializer_list<char const *const> init_list) const
    {
        return std::any_of(init_list.begin(), init_list.end(), [&](char const *const name) { return got_flag(name); });
    }

private:
    std::string trim_leading_dashes(std::string const &name) const;
    bool        is_number(std::string const &arg) const;
    bool        is_option(std::string const &arg) const;
    bool        is_param(std::string const &name) const;
    bool        got_flag(std::string const &name) const;
    bool        got_opt(std::string const &name) const;

private:
    std::vector<std::string>                args_;
    std::multimap<std::string, std::string> params_;
    std::vector<std::string>                pos_args_;
    std::multiset<std::string>              flags_;
    std::set<std::string>                   registeredParams_;
    std::string                             empty_;
};

//////////////////////////////////////////////////////////////////////////

inline void YS_ArgParser::parse(const char *const argv[], int mode)
{
    int argc = 0;
    for (auto argvp = argv; *argvp; ++argc, ++argvp)
        ;
    parse(argc, argv, mode);
}

//////////////////////////////////////////////////////////////////////////

inline void YS_ArgParser::parse(int argc, const char *const argv[], int mode /*= PREFER_FLAG_FOR_UNREG_OPTION*/)
{
    // clear out possible previous parsing remnants
    flags_.clear();
    params_.clear();
    pos_args_.clear();

    // convert to strings
    args_.resize(static_cast<decltype(args_)::size_type>(argc));
    std::transform(argv, argv + argc, args_.begin(), [](const char *const arg) { return arg; });

    // parse line
    for (auto i = 0u; i < args_.size(); ++i)
    {
        if (!is_option(args_[i]))
        {
            pos_args_.emplace_back(args_[i]);
            continue;
        }

        auto name = trim_leading_dashes(args_[i]);

        if (!(mode & NO_SPLIT_ON_EQUALSIGN))
        {
            auto equalPos = name.find('=');
            if (equalPos != std::string::npos)
            {
                params_.insert({name.substr(0, equalPos), name.substr(equalPos + 1)});
                continue;
            }
        }

        // if the option is unregistered and should be a multi-flag
        if (1 == (args_[i].size() - name.size()) &&          // single dash
            YS_ArgParser::SINGLE_DASH_IS_MULTIFLAG & mode && // multi-flag mode
            !is_param(name))                                 // unregistered
        {
            std::string keep_param;

            if (!name.empty() && is_param(std::string(1ul, name.back()))) // last char is param
            {
                keep_param += name.back();
                name.resize(name.size() - 1);
            }

            for (auto const &c : name)
            {
                flags_.emplace(std::string{c});
            }

            if (!keep_param.empty())
            {
                name = keep_param;
            }
            else
            {
                continue; // do not consider other options for this arg
            }
        }

        // any potential option will get as its value the next arg, unless that arg is an option too
        // in that case it will be determined a flag.
        if (i == args_.size() - 1 || is_option(args_[i + 1]))
        {
            flags_.emplace(name);
            continue;
        }

        // if 'name' is a pre-registered option, then the next arg cannot be a free parameter to it is skipped
        // otherwise we have 2 modes:
        // PREFER_FLAG_FOR_UNREG_OPTION: a non-registered 'name' is determined a flag.
        //                               The following value (the next arg) will be a free parameter.
        //
        // PREFER_PARAM_FOR_UNREG_OPTION: a non-registered 'name' is determined a parameter, the next arg
        //                                will be the value of that option.

        assert(!(mode & YS_ArgParser::PREFER_FLAG_FOR_UNREG_OPTION) || !(mode & YS_ArgParser::PREFER_PARAM_FOR_UNREG_OPTION));

        bool preferParam = (mode & YS_ArgParser::PREFER_PARAM_FOR_UNREG_OPTION) ? true : false;

        if (is_param(name) || preferParam)
        {
            params_.insert({name, args_[i + 1]});
            ++i; // skip next value, it is not a free parameter
            continue;
        }
        else
        {
            flags_.emplace(name);
        }
    }
}

//////////////////////////////////////////////////////////////////////////

inline bool YS_ArgParser::is_number(std::string const &arg) const
{
    // inefficient but simple way to determine if a string is a number (which can start with a '-')
    std::istringstream istr(arg);
    double             number;
    istr >> number;
    return !(istr.fail() || istr.bad());
}

//////////////////////////////////////////////////////////////////////////

inline bool YS_ArgParser::is_option(std::string const &arg) const
{
    assert(0 != arg.size());
    if (is_number(arg))
        return false;
    return '-' == arg[0];
}

//////////////////////////////////////////////////////////////////////////

inline std::string YS_ArgParser::trim_leading_dashes(std::string const &name) const
{
    auto pos = name.find_first_not_of('-');
    return std::string::npos != pos ? name.substr(pos) : name;
}

//////////////////////////////////////////////////////////////////////////

inline bool YS_ArgParser::got_flag(std::string const &name) const
{
    return flags_.end() != flags_.find(trim_leading_dashes(name));
}

inline bool YS_ArgParser::got_opt(std::string const &name) const
{
    return params_.end() != params_.find(trim_leading_dashes(name));
}

//////////////////////////////////////////////////////////////////////////

inline bool YS_ArgParser::is_param(std::string const &name) const
{
    return registeredParams_.count(name);
}

//////////////////////////////////////////////////////////////////////////

inline bool YS_ArgParser::operator[](std::string const &name) const
{
    return got_flag(name);
}

//////////////////////////////////////////////////////////////////////////

inline bool YS_ArgParser::operator[](std::initializer_list<char const *const> init_list) const
{
    return std::any_of(init_list.begin(), init_list.end(), [&](char const *const name) { return got_flag(name); });
}

//////////////////////////////////////////////////////////////////////////

inline std::string const &YS_ArgParser::operator[](size_t ind) const
{
    if (ind < pos_args_.size())
        return pos_args_[ind];
    return empty_;
}

//////////////////////////////////////////////////////////////////////////

inline std::string YS_ArgParser::operator()(std::string const &name) const
{
    auto optIt = params_.find(trim_leading_dashes(name));
    if (params_.end() != optIt)
        return optIt->second;
    return "";
}

//////////////////////////////////////////////////////////////////////////

inline std::string YS_ArgParser::operator()(std::initializer_list<char const *const> init_list) const
{
    for (auto &name : init_list)
    {
        auto optIt = params_.find(trim_leading_dashes(name));
        if (params_.end() != optIt)
            return optIt->second;
    }
    return "";
}

//////////////////////////////////////////////////////////////////////////

template <typename T>
std::string YS_ArgParser::operator()(std::string const &name, T &&def_val) const
{
    auto optIt = params_.find(trim_leading_dashes(name));
    if (params_.end() != optIt)
        return optIt->second;

    std::ostringstream ostr;
    ostr.precision(std::numeric_limits<long double>::max_digits10);
    ostr << def_val;
    return ostr.str(); // use default
}

//////////////////////////////////////////////////////////////////////////

// same as above but for a list of names. returns the first value to be found.
template <typename T>
std::string YS_ArgParser::operator()(std::initializer_list<char const *const> init_list, T &&def_val) const
{
    for (auto &name : init_list)
    {
        auto optIt = params_.find(trim_leading_dashes(name));
        if (params_.end() != optIt)
            return optIt->second;
    }
    std::ostringstream ostr;
    ostr.precision(std::numeric_limits<long double>::max_digits10);
    ostr << def_val;
    return ostr.str(); // use default
}

//////////////////////////////////////////////////////////////////////////

inline std::string YS_ArgParser::operator()(size_t ind) const
{
    if (pos_args_.size() <= ind)
        return "";

    return pos_args_[ind];
}

//////////////////////////////////////////////////////////////////////////

template <typename T>
std::string YS_ArgParser::operator()(size_t ind, T &&def_val) const
{
    if (pos_args_.size() <= ind)
    {
        std::ostringstream ostr;
        ostr.precision(std::numeric_limits<long double>::max_digits10);
        ostr << def_val;
        return ostr.str();
    }

    return pos_args_[ind];
}

//////////////////////////////////////////////////////////////////////////

inline void YS_ArgParser::add_param(std::string const &name)
{
    registeredParams_.insert(trim_leading_dashes(name));
}

//////////////////////////////////////////////////////////////////////////

inline void YS_ArgParser::add_param(std::initializer_list<const char *const> init_list)
{
    YS_ArgParser::add_params(init_list);
}

//////////////////////////////////////////////////////////////////////////

inline void YS_ArgParser::add_params(std::initializer_list<char const *const> init_list)
{
    for (auto &name : init_list)
        registeredParams_.insert(trim_leading_dashes(name));
}

//////////////////////////////////////////////////////////////////////////

inline void YS_ArgParser::add_params(const std::string &name)
{
    YS_ArgParser::add_param(name);
}

//////////////////////////////////////////////////////////////////////////

inline multimap_iteration_wrapper YS_ArgParser::params(std::string const &name) const
{
    auto trimmed_name = trim_leading_dashes(name);
    return multimap_iteration_wrapper(params_.lower_bound(trimmed_name), params_.upper_bound(trimmed_name));
}
} // namespace isaac

#endif