#include <string>
#include <fstream>
#include <streambuf>
#include <filesystem>

struct IUnknown; // Workaround for "combaseapi.h(229): error C2187: syntax error: 'identifier' was unexpected here" when using /permissive-
#include "../compiler/conout.h"

namespace fs = std::filesystem;

inline bool ends_with(const std::string &value, const std::string &ending) {
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

std::string read_string(const std::string &filepath) {
    std::ifstream t(filepath);
    std::string str((std::istreambuf_iterator<char>(t)),
        std::istreambuf_iterator<char>());

    if (!str.empty() && str[str.length() - 1] == '\n') {
        str.erase(str.length() - 1);
    }

    return str;
}

bool is_same_file(const std::string &a, const std::string &b) {
    return read_string(a) == read_string(b);
}

int main() {
    std::string path = "../tests/";

    int total = 0;
    int pass = 0, fail = 0;

    con::set_color(CON_WHITE);
    printf("[Rookie::test] - %s \n", path.c_str());

    for (auto &p : fs::recursive_directory_iterator(path)) {
        auto path = p.path().string();
        auto exp_path = path + ".exp";

        if (path.find("invalid\\") != std::string::npos)
            continue;

        if (ends_with(path, ".rk") == false)
            continue;

        if (fs::exists(exp_path) == false) {
            printf("%s does not have .exp file.\n", path.c_str());
            return -1;
        }

        con::set_color(CON_WHITE);
        printf("   * %-45s   ", path.c_str());
        con::set_color(CON_LIGHTGRAY);

        auto out_name = std::tmpnam(nullptr);
        int r = system(("..\\bin\\rookie.exe " + path + " >> " + out_name).c_str());

        if (r == 0) {
            auto result = read_string(out_name);
            auto expected = read_string(exp_path);

            if (result == expected) {
                con::set_color(CON_LIGHTGREEN);
                printf("   [PASS]\n");
                pass++;
            }
            else {
                con::set_color(CON_LIGHTRED);
                printf("   [FAIL]\n");

                con::set_color(CON_YELLOW);
                printf("   [RESULT]\n");
                con::set_color(CON_LIGHTGRAY);
                printf("%s\n", result.c_str());
                con::set_color(CON_YELLOW);
                printf("   [EXPECTED]\n");
                con::set_color(CON_LIGHTGRAY);
                printf("%s\n", expected.c_str());
                
                fail++;
            }
        }
        else {
            con::set_color(CON_LIGHTRED);
            printf("   [FAIL]\n");
            con::set_color(CON_YELLOW);
            printf("   [OUTPUT]\n");
            con::set_color(CON_LIGHTGRAY);
            auto result = read_string(out_name);
            printf("%s\n", result.c_str());
            fail++;
        }

        total++;

        fs::remove(out_name);
    }

    con::set_color(CON_WHITE);
    printf("\n\n");
    printf("[TEST RESULT]\n");
    printf("   - pass   : %d\n", pass);
    printf("   - failure: %d\n", fail);
    printf("   - total  : %d\n", total);

    return 0;
}

