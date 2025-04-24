#include "ys_cli_log.h"
#include "tool/ys_argh.hpp"

using namespace isaac;
using namespace std;

int32_t usage()
{
    CLI_PRINT(R"(
        tohex    convet file to hex string.
        option:
            -h         help
            -in [file] in file
)");
    return 0;
}

int32_t tohex(string path)
{
    if(!YS_File::isFile(path))
    {
        CLI_ERROR("file {} is not exist.", path);
        return -1;
    }
    string data = YS_File::read(path);
    string hex  = YS_String::toHex(data);
    CLI_DEBUG("hex = [{}]", hex);
    string bin;
    for (size_t i = 0; i < hex.length(); i += 2)
    {
        bin.append(fmt::format("0x{}{},", hex[i], hex[i + 1]));
    }
    if (!bin.empty())
    {
        bin.pop_back();
    }

    CLI_DEBUG("bin array = [{}]", bin);
    return 0;
}

int32_t main(int32_t argc, char const *argv[])
{
    YS_ArgParser parser(argv);
    if (parser.hasParam("-h"))
    {
        return usage();
    }

    if (parser.hasParam("-in"))
    {
        string path = parser("-in");
        return tohex(path);
    }

    return usage();
}