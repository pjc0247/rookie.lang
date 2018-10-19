#pragma once

extern "C" {
    void rk_exec(const char *code);

    void rk_free(program *p);
    void rk_free(pdb *p);
}