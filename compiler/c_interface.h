/*  c_interface.h

    A set of C style methods to export.
 */
#pragma once

struct program;
struct pdb;
struct value;

struct rkcc;

extern "C" {
    void rk_exec(const char *code);

    rkcc *rk_init();

    int rk_add_type(rkcc *cc, const char *name);

    void rk_compile(rkcc *cc, const char *src);

    void rk_free_cc(rkcc *cc);;
    void rk_free_program(program *p);
    void rk_free_pdb(pdb *p);

    value rk_integer(int n);
    value rk_string(const char *str);
}