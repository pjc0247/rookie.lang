#pragma once

#include "program.h"

extern "C" {
    void rk_exec(const char *code);

    void rk_free_program(program *p);
    void rk_free_pdb(pdb *p);
}