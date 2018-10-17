#pragma once

#include <string>
#include <vector>

#include "string_pool.h"
#include "token.h"
#include "program.h"
#include "errors.h"

struct compile_option {
    bool generate_pdb = false;
};

struct compile_output {
    program program;
    pdb pdb;

    std::vector<compile_error> errors;
};

class compile_context {
public:
    compile_context(const compile_option &opts) :
        opts(opts) {
    }
        

    void push_error(const compile_error &err) {
        errors.push_back(err);
    }

    void fin() {
    }

public:
    const compile_option &opts;

    std::vector<compile_error> errors;
    
    string_pool code;
};