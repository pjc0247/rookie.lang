#include "stdafx.h"

#include "code_gen.h"
#include "compiler.h"
#include "backends/p2wast.h"
#include "program_io.h"
#include "runner.h"
#include "fileio.h"

#include "c_interface.h"

void rk_exec(const char *code) {
    auto b = binding::default_binding();
    auto rc = compiler::default_compiler(b);

    compile_option opts;

    auto out = rc.compile(str2wstr(code), opts);
    if (out.errors.empty()) {
        runner(*out.program, b).execute();
    }
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