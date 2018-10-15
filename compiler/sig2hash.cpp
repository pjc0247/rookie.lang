#include "stdafx.h"

#include "sig2hash.h"

unsigned int sig2hash(const std::wstring &str) {
    unsigned int hash = 5381;
    for (size_t i = 0; i < str.size(); ++i)
        hash = 33 * hash + (wchar_t)str[i];
    return hash;
}