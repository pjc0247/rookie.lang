#pragma once

#include <string>
#include <vector>

#include "token.h"
#include "program.h"

struct compile_error {
    std::string message;
    int line, cols;

    compile_error(const token &token, const std::string &message) :
        message(message) {

        // TODO
        line = token.line, cols = token.cols;
    }
};

class compile_context {
public:

    void push_error(const compile_error &err) {
        errors.push_back(err);
    }

public:
    std::vector<compile_error> errors;
};