#pragma once

#include <stdio.h>
#include <string>
#include <exception>

class fileio {
public:
    static char *read_string(const char *filepath) {
        FILE *fp = fopen(filepath, "rb");
        if (!fp) {
            throw std::exception(
                ("no such file: " + std::string(filepath)).c_str());
        }

        fseek(fp, 0, SEEK_END);
        auto len = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        char *buf = new char[len + 1];
        fread(buf, sizeof(char), len, fp);
        buf[len] = 0;

        fclose(fp);

        return buf;
    }
};