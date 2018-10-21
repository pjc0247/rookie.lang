#include "stdafx.h"

#include "code_gen.h"
#include "compiler.h"
#include "backends/p2wast.h"
#include "program_io.h"
#include "runner.h"
#include "fileio.h"

#include "c_interface.h"

struct rkcc {
    binding binding;

    std::vector<type_builder> types;
};

void rk_exec(const char *code) {
    auto b = binding::default_binding();
    auto rc = compiler::default_compiler(b);

    compile_option opts;

    auto out = rc.compile(str2wstr(code), opts);
    if (out.errors.empty()) {
        runner(*out.program, b).execute();
    }
}

rkcc *rk_init() {
    auto cc = new rkcc();

    return cc;
}
int rk_add_type(rkcc *cc, const char *name) {
    auto tb = type_builder(str2wstr(name));
    cc->types.push_back(tb);

    return cc->types.size() - 1;
}
void rk_compile(rkcc *cc, const char *src) {

}

void rk_free_cc(rkcc *cc) {
    delete cc;
}
void rk_free_program(program *p) {
    if (p == nullptr) return;

    delete p;
}
void rk_free_pdb(pdb *p) {
    if (p == nullptr) return;

    delete[] p->code;
    delete[] p->inst_data;
    delete[] p->sigtable;

    delete p;
}