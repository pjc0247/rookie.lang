#include "stdafx.h"

std::wstring str2wstr(const char* text) {
    const size_t size = std::strlen(text);
    std::wstring wstr;
    if (size > 0) {
        wstr.resize(size);
        std::mbstowcs(&wstr[0], text, size);
    }
    return wstr;
}
std::string wstr2str(const wchar_t* text) {
    const size_t size = std::wcslen(text);
    std::string str;
    if (size > 0) {
        str.resize(size);
        std::wcstombs(&str[0], text, size);
    }
    return str;
}