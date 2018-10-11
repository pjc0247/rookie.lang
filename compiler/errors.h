#pragma once

#include "compilation.h"

class unexpected_token_error : public compile_error {
public:
    unexpected_token_error(const token &token) :
        compile_error(token, "Unexpected token: " + token.raw + ".") {
    }
};


class invalid_program_exception : public std::exception {
public:
	invalid_program_exception(const char *msg)
		: std::exception(msg) {
	}
};