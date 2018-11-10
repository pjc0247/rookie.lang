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
        runner(*out.program, b)
            .execute();
    }
    else {
        for (auto er : out.errors)
            printf("%S\r\n", er.message.c_str());
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

    for (uint32_t i = 0; i < p->header.types_len;i++)
        free(p->types[i].methods);

    free(p->types);
    free((void*)p->rdata);
    free(p->entries);
    free(p->code);
    free((void*)p->lookups);

    free(p);
}
void rk_free_pdb(pdb *p) {
    if (p == nullptr) return;

    free(p->code);
    free(p->inst_data);
    free(p->sigtable);

    free(p);
}

value rk_empty() {
    value v;
    v.type = value_type::empty;
    v.objref = nullptr;
    return v;
}
value rk_null() {
    value v;
    v.type = value_type::null;
    v.objref = nullptr;
    return v;
}
value rk_boolean(bool b) {
    value v;
    v.type = value_type::boolean;
    v.integer = b ? 1 : 0;
    return v;
}
value rk_integer(int n) {
    value v;
    v.type = value_type::integer;
    v.integer = n;
    return v;
}
value rk_decimal(float f) {
    value v;
    v.type = value_type::decimal;
    v.decimal = f;
    return v;
}
value rk_string(const char *str) {
    return str2rk(str2wstr(str));
}
