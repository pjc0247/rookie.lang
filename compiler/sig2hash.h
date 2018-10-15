#pragma once

#include <string>

unsigned int sig2hash(const std::wstring &str);

#define _predefined_hash(name) \
    const unsigned int sighash_##name = sig2hash(L#name)

_predefined_hash(new);
_predefined_hash(array);
_predefined_hash(string);