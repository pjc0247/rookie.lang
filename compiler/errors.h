#pragma once

#include "compilation.h"

class unexpected_token_error : public compile_error {
public:
    unexpected_token_error(const token &token) :
        compile_error(token, "Unexpected token: `" + token.raw + "`.") {
    }
};

class undeclared_method_error : public compile_error {
public:
	undeclared_method_error(const token &token, const std::string &name) :
		compile_error(token, "Undeclared method: `" + name + "`.") {

	}
};
class undefined_variable_error : public compile_error {
public:
	undefined_variable_error(const token &token) :
		compile_error(token, "Undefined variable: `" + token.raw + "`."){

	}
};

class invalid_program_exception : public std::exception {
public:
    invalid_program_exception(const char *msg)
        : std::exception(msg) {
    }
};