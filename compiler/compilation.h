#pragma once

#include <string>
#include <vector>

#include "string_pool.h"
#include "token.h"
#include "syntax.h"
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

struct compiletime_methoddata {
    std::wstring name;
    uint32_t entry;
};
struct compiletime_typedata {
    std::wstring name;
    std::vector<compiletime_methoddata> methods;
};

// Shared variables during compilation pipeline.
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
    // Input & Output
    const compile_option &opts;
    std::vector<compile_error> errors;

    // Compile data
    syntax_node *root_node;
    std::vector<compiletime_typedata> types;
    
    string_pool code;
};