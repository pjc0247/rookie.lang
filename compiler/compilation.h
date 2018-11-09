#pragma once

#include <string>
#include <vector>

#include "string_pool.h"
#include "token.h"
#include "syntax/syntax_node.h"
#include "binding.h"
#include "program.h"
#include "errors.h"
#include "type_attr.h"
#include "c_interface.h"

struct compile_option {
    bool generate_pdb = false;
};

struct compile_output {
    program *program;
    pdb     *pdb;

    std::vector<compile_error> errors;

    compile_output() :
        program(nullptr),
        pdb(nullptr){
    }
    compile_output(compile_output &&o) {
        pdb = o.pdb;
        program = o.program;
        errors = o.errors;

        o.pdb = nullptr;
        o.program = nullptr;
        o.errors.clear();
    }
    ~compile_output() {
        // it's safe to pass nullptr
        rk_free_program(program);
        rk_free_pdb(pdb);
    }
};

struct compiletime_methoddata {
    std::wstring name;
    uint32_t entry;
    method_attr attr;

    compiletime_methoddata() :
        entry(),
        attr(method_attr::method_none) {
    }
};
struct compiletime_typedata {
    std::wstring name;
    class_attr attr;

    std::vector<std::wstring> parents;
    std::vector<compiletime_methoddata> methods;

    compiletime_typedata() :
        attr(class_attr::class_none) {
    }
};

// Shared variables during compilation pipeline.
class compile_context {
public:
    compile_context(const compile_option &opts, binding &bindings) :
        opts(opts),
        bindings(bindings) {
    }

    std::vector<compiletime_typedata> get_programtypes() {
        std::vector<compiletime_typedata> r;
        for (auto &p : types) {
            if (!(p.second.attr & class_attr::class_systype))
                r.push_back(p.second);
        }
        return r;
    }
    std::vector<compiletime_typedata> get_systypes() {
        std::vector<compiletime_typedata> r;
        for (auto &p : types) {
            if (p.second.attr & class_attr::class_systype)
                r.push_back(p.second);
        }
        return r;
    }

    void push_error(const compile_error &err) {
        errors.push_back(err);
    }

public:
    // Input & Output
    const compile_option &opts;
    std::vector<compile_error> errors;

    // Compile data
    binding &bindings;
    syntax_node *root_node;
    std::map<std::wstring, compiletime_typedata> types;
    
    string_pool code;
};