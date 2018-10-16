#pragma once

#include <stdio.h>
#include <string>
#include <exception>

#ifndef __EMSCRIPTEN__
class fileio {
public:
    static wchar_t *read_string(const std::wstring &filepath) {
        FILE *fp = _wfopen(filepath.c_str(), L"rtS, ccs=UTF-8");

        if (!fp) {
            throw base_exception(
                (char*)(L"no such file: " + std::wstring(filepath)).c_str());
        }

        fseek(fp, 0, SEEK_END);
        auto len = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        wchar_t *buf = new wchar_t[len + 1];
        fread(buf, sizeof(wchar_t), len, fp);
        buf[len] = 0;

        fclose(fp);

        return buf;
    }
};
#endif