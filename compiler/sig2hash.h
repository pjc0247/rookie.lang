#pragma once

#include <string>

unsigned int sig2hash(const std::string &str);

#define _predefined_hash(name) \
	const unsigned int sighash_##name = sig2hash(#name)

_predefined_hash(new);