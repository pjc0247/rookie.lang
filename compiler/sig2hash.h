#pragma once

#include <string>

unsigned int sig2hash(const std::string &str) {
	unsigned int hash = 5381;
	for (size_t i = 0; i < str.size(); ++i)
		hash = 33 * hash + (unsigned char)str[i];
	return hash;
}