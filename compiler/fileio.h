/*  fileio.h

    Provides most simplist form of
    file input/output methods.
 */
#pragma once

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <exception>

#ifndef __EMSCRIPTEN__
class fileio {
public:
    static std::wstring read_string(const std::wstring &filepath) {
        /*
        FILE *fp = _wfopen(filepath.c_str(), L"rtS, ccs=UTF-8");

        if (!fp) {
            throw base_exception(
                (char*)(L"no such file: " + std::wstring(filepath)).c_str());
        }

        fseek(fp, 0, SEEK_END);
        auto len = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        wchar_t *buf = new wchar_t[len + 1];
        len = fread(buf, sizeof(wchar_t), len, fp);
        buf[len] = 0;

        fclose(fp);

        return buf;
        */

        std::wifstream t(wstr2str(filepath.c_str()));
        std::wstringstream buffer;
        buffer << t.rdbuf();
        return buffer.str();
    }
};
#endif