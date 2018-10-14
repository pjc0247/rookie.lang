#pragma once

#include <string>
#include <exception>

#include "token.h"

struct compile_error {
    std::string message;
    int line, cols;

    compile_error(const token &token, const std::string &message) :
        message(message) {
        line = token.line, cols = token.cols;
    }
};

class unexpected_token_error : public compile_error {
public:
    unexpected_token_error(const token &token) :
        compile_error(token, "Unexpected token: `" + token.raw + "`.") {
    }
};

class name_too_long_error : public compile_error {
public:
    name_too_long_error(const token &token) :
        compile_error(token, "Name too long: `" + token.raw + "`") {

    }
};
class syntax_error : public compile_error {
public:
    syntax_error(const token &token, const std::string &message) :
        compile_error(token, message) {

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
class invalid_access_exception : public std::exception {
public:
    invalid_access_exception(const char *msg)
        : std::exception(msg) {
    }
};