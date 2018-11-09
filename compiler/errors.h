#pragma once

#include <string>
#include <exception>

#include "token.h"
#include "syntax.h"

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
    syntax_error(syntax_node *node, const std::wstring &message) :
        syntax_error(node->token(), message) {
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

class base_exception : public std::exception {
public:
    base_exception() {
        this->msg = std::string();
    }
    base_exception(const base_exception &other) {
        this->msg = other.msg;
    }
    base_exception(const char *msg) {
        this->msg = std::string(msg);
    }
    base_exception(const std::string &str) {
        this->msg = str;
    }

    virtual char const *what() const noexcept { return msg.c_str(); }
private:
    std::string msg;
};
class codegen_exception : public base_exception {
public:
    codegen_exception(const char *msg)
        : base_exception(msg) {
    }
};
class invalid_program_exception : public base_exception {
public:
    invalid_program_exception(const char *msg)
        : base_exception(msg) {
    }
};
class invalid_access_exception : public base_exception {
public:
    invalid_access_exception(const char *msg)
        : base_exception(msg) {
    }
};