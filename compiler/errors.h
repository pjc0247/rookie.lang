#pragma once

#include <string>
#include <exception>

#include "token.h"

struct compile_error {
    std::wstring message;
    int line, cols;

    compile_error(const token &token, const std::wstring &message) :
        message(message) {
        line = token.line, cols = token.cols;
    }
};

class unexpected_token_error : public compile_error {
public:
    unexpected_token_error(const token &token) :
        compile_error(token, L"Unexpected token: `" + token.raw + L"`.") {
    }
};

class codegen_error : public compile_error {
public:
    codegen_error(const std::wstring &message) :
        compile_error(::token(), message) {
    }
};
class name_too_long_error : public compile_error {
public:
    name_too_long_error(const token &token) :
        compile_error(token, L"Name too long: `" + token.raw + L"`") {

    }
};
class syntax_error : public compile_error {
public:
    syntax_error(const token &token, const std::wstring &message) :
        compile_error(token, message) {

    }
};
class undeclared_method_error : public compile_error {
public:
    undeclared_method_error(const token &token, const std::wstring &name) :
        compile_error(token, L"Undeclared method: `" + name + L"`.") {

    }
};
class undefined_variable_error : public compile_error {
public:
    undefined_variable_error(const token &token) :
        compile_error(token, L"Undefined variable: `" + token.raw + L"`."){

    }
};

class rkexception : public std::exception {
public:
    rkexception(const char *msg) {
        this->msg = std::string(msg);
    }
    virtual char const *what() const noexcept { return msg.c_str(); }
private:
    std::string msg;
};
class codegen_exception : public rkexception {
public:
    codegen_exception(const char *msg)
        : rkexception(msg) {
    }
};
class invalid_program_exception : public rkexception {
public:
    invalid_program_exception(const char *msg)
        : rkexception(msg) {
    }
};
class invalid_access_exception : public rkexception {
public:
    invalid_access_exception(const char *msg)
        : rkexception(msg) {
    }
};