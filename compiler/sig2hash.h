#pragma once

#include <string>

unsigned int sig2hash(const std::wstring &str);
CONSTEXPR unsigned int sig2hash_c(const wchar_t str[]);

#define _predefined_hash(name) \
    const unsigned int sighash_##name = sig2hash_c(L ## #name)

_predefined_hash(new);

_predefined_hash(object);
_predefined_hash(array);
_predefined_hash(dictionary);
_predefined_hash(string);