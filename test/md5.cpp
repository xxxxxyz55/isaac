#include "alg/ys_md5.hpp"
#include "tool/ys_argh.hpp"
#include "tool/ys_file.hpp"
#include "ys_cli_log.h"

using namespace isaac;
using namespace std;

int32_t usage()
{
    printf(R"(
    md5 [file]          sum md5 of file
    md5 -h              help
    md5 -v,--version    version
)");
    return 0;
}
int32_t version()
{
    printf("1.0.0\n");
    return 0;
}

int32_t md5sum(string file)
{
    uint8_t  md[16];
    uint32_t off = 0;
    uint32_t sz  = YS_File::size(file);
    YS_Md5   ctx;

    for (; off < sz;)
    {
        string data = YS_File::read(file, 8 * 8192, off);
        if (data.empty())
        {
            CLI_ERROR("read file error, has read len {}.", off);
            break;
        }
        ctx.update((uint8_t *)data.c_str(), data.length());
        off += data.length();
    }
    ctx.final(md);
    CLI_PRINT("{}\t{}\n", YS_String::toHex(md, sizeof(md)), file);
    return 0;
}

int32_t main(int32_t argc, char const *argv[])
{
    YS_ArgParser parser(argv);

    if (parser.hasFlag({"-h", "-help"})) return usage();

    if (parser.hasFlag({"-v", "--version"})) return version();

    auto  &vtarg = parser.pos_args();
    string file;
    if (vtarg.size())
    {
        file = vtarg[1];
    }

    if (!YS_File::isFile(file))
    {
        CLI_ERROR("{} is not a regular file.", file);
    }
    else
    {
        return md5sum(file);
    }
    return 0;
}