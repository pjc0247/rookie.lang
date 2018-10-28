#include "stdafx.h"

#include "sig2hash.h"

uint32_t sig2hash(const std::wstring &str, int offset) {
    uint32_t hash = 5381;
    for (size_t i = offset; i < str.size(); ++i)
        hash = 33 * hash + (wchar_t)str[i];
    return hash;
}
uint32_t sig2hash(const wchar_t *str, int offset) {
    uint32_t hash = 5381;
    for (size_t i = offset; i < wcslen(str); ++i)
        hash = 33 * hash + (wchar_t)str[i];
    return hash;
}

