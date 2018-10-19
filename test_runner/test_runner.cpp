#include "stdafx.h"

#include <iostream>
#include <filesystem>

namespace fs = std::experimental::filesystem::v1;

inline bool ends_with(const std::string &value, const std::string &ending) {
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

std::string read_string(const std::string &filepath) {
    FILE *fp = fopen(filepath.c_str(), "r");

    if (!fp) {
        throw std::exception("no such file");
    }

    fseek(fp, 0, SEEK_END);
    auto len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *buf = new char[len + 1];
    fread(buf, sizeof(char), len, fp);
    buf[len] = 0;

    if (buf[len - 2] == '\n') {
        buf[len - 2] = 0;
        if (buf[len - 3] == '\r') buf[len - 3] = 0;
    }
    if (buf[len - 1] == '\n') {
        buf[len - 1] = 0;
        if (buf[len - 2] == '\r') buf[len - 2] = 0;
    }

    fclose(fp);

    auto str = std::string(buf);
    delete[] buf;

    return str;
}

bool is_same_file(const std::string &a, const std::string &b) {
    return read_string(a) == read_string(b);
}

int main() {
    std::string path = "../tests/";

    int total = 0;
    int pass = 0, fail = 0;

    for (auto &p : fs::directory_iterator(path)) {
        auto path = p.path().string();
        auto exp_path = path + ".exp";

        if (ends_with(path, ".rk") == false)
            continue;

        if (fs::exists(exp_path) == false) {
            printf("%s does not have .exp file.\n", path.c_str());
            return -1;
        }

        printf("   * %s\n", path.c_str());

        auto out_name = std::tmpnam(nullptr);
        system(("..\\bin\\rookie.exe " + path + " >> " + out_name).c_str());

        auto result = read_string(out_name);
        auto expected = read_string(exp_path);

        if (result == expected) {
            printf("   [PASS]\n");
            pass++;
        }
        else {
            printf("   [RESULT] %s\n", result.c_str());
            printf("   [EXPECTED] %s\n", expected.c_str());
            printf("   [FAIL]\n");
            fail++;
        }
        total++;

        fs::remove(out_name);
    }

    printf("\n\n");
    printf("[TEST RESULT]\n");
    printf("   - pass   : %d\n", pass);
    printf("   - failure: %d\n", fail);
    printf("   - total  : %d\n", total);

    return 0;
}

