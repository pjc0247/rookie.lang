#pragma once

#include <string>

unsigned int sig2hash(const std::wstring &str, int offset = 0);
unsigned int sig2hash(const wchar_t *str, int offset = 0);
CONSTEXPR uint32_t sig2hash_c(const wchar_t str[]) {
    uint32_t hash = 5381;
    for (size_t i = 0;; ++i) {
        if (str[i] == 0) break;
        hash = 33 * hash + (wchar_t)str[i];
    }
    return hash;
}

#define _predefined_hash(name) \
    const unsigned int sighash_##name = sig2hash_c(L ## #name)

#pragma warning(push)  
#pragma warning(disable : 4307) // overflows 
_predefined_hash(new);
_predefined_hash(equal);

_predefined_hash(object);
_predefined_hash(array);
_predefined_hash(dictionary);
_predefined_hash(string);
_predefined_hash(integer);
#pragma warning(pop)  